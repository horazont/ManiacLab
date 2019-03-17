#pragma STDC FENV_ACCESS

#include "physics.hpp"

#include <iostream>

#include <cassert>
#include <cfenv>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <xmmintrin.h>

#include <GL/glew.h>

#include <ffengine/io/log.hpp>

#include "logic/game_object.hpp"

// #define TIMELOG_LABSIM

#ifdef TIMELOG_LABSIM
#include <chrono>
typedef std::chrono::steady_clock timelog_clock;

#define TIMELOG_ms(x) std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1000> > >(x).count()
#endif

static io::Logger &logger = io::logging().get_logger("sim.lab.native");


/*
const SimFloat ILabSim::convection_factor = 0.05f;
const SimFloat ILabSim::flow_damping = 0.5f;
const SimFloat ILabSim::flow_factor = 0.4f;
const SimFloat ILabSim::fog_flow_factor = 0.01f;
const SimFloat ILabSim::heat_flow_factor = 0.05f;
*/

const SimFloat ILabSim::convection_factor = 1e-5f;
const SimFloat ILabSim::air_diffusion_factor = 0.5f;
const SimFloat ILabSim::air_flow_factor = 0.8f;
const SimFloat ILabSim::fog_diffusion_factor = 0.05f;
const SimFloat ILabSim::heat_diffusion_factor = 0.05f;

template <typename T>
T first(T v1, T v2)
{
    if (v1) {
        return v1;
    }
    return v2;
}


enum SimNeighbours {
    Top = 0,
    Right = 1,
    Bottom = 2,
    Left = 3,
};


template <typename T>
inline T clamp(const T value, const T min, const T max)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    else
        return value;
}

inline double max(const double a, const double b)
{
    if (a > b)
        return a;
    return b;
}

NativeLabSim::NativeLabSim(
        CoordInt width, CoordInt height,
        const SimulationConfig &config):
    m_width(width),
    m_height(height),
    m_block_count((m_height + ROWS_PER_BLOCK-1) / ROWS_PER_BLOCK),
    m_worker_count(std::thread::hardware_concurrency()),
    m_meta_cells(m_width*m_height),
    m_front_cells(m_width*m_height),
    m_back_cells(m_width*m_height),
    m_config(config),
    m_running(false),
    m_run(false),
    m_done(false),
    m_worker_to_start(0),
    m_worker_terminate(false),
    m_worker_stopped(m_worker_count),
    m_worker_block_ctr(0),
    m_terminated(false),
    m_coordinator_thread(std::bind(&NativeLabSim::coordinator_impl,
                                   this))
{
    m_null_cell.air_pressure = m_config.initial_air_pressure;
    m_null_cell.fog_density = m_config.initial_fog_density;
    m_null_cell.heat_energy = m_config.initial_temperature;

    for (CoordInt y = 0; y < m_height; y++) {
        for (CoordInt x = 0; x < m_width; x++) {
            init_metadata(m_meta_cells, x, y);
            init_cell(m_front_cells, x, y,
                      m_config.initial_air_pressure,
                      m_config.initial_temperature,
                      m_config.initial_fog_density);
            init_cell(m_back_cells, x, y,
                      m_config.initial_air_pressure,
                      m_config.initial_temperature,
                      m_config.initial_fog_density);
        }
    }

    m_worker_threads.reserve(m_worker_count);
    for (unsigned int i = 0; i < m_worker_count; i++) {
        m_worker_threads.emplace_back(std::bind(&NativeLabSim::worker_impl, this));
    }
}

NativeLabSim::~NativeLabSim()
{
    std::cout << "destroying automaton" << std::endl;
    wait_for_frame();
}

void *NativeLabSim::coordinator_impl()
{
    logger.logf(io::LOG_INFO, "labsim: %u cells in %u blocks",
                m_width*m_height,
                m_block_count);
    while (!m_terminated) {
        {
            std::unique_lock<std::mutex> control_lock(m_control_mutex);
            while (!m_run && !m_terminated) {
                m_control_wakeup.wait(control_lock);
            }
            if (m_terminated) {
                control_lock.unlock();
                std::lock_guard<std::mutex> done_lock(m_done_mutex);
                m_done = true;
                break;
            }
            m_run = false;
        }

#ifdef TIMELOG_LABSIM
        const timelog_clock::time_point t0 = timelog_clock::now();
        timelog_clock::time_point t_sync, t_sim;
#endif

#ifdef TIMELOG_LABSIM
        t_sync = timelog_clock::now();
#endif
        coordinator_run_workers();

        {
            std::lock_guard<std::mutex> done_lock(m_done_mutex);
            assert(!m_done);
            m_done = true;
        }
        m_done_wakeup.notify_all();

#ifdef TIMELOG_LABSIM
        t_sim = timelog_clock::now();
        logger.logf(io::LOG_DEBUG, "fluid: sync time: %.2f ms",
                    TIMELOG_ms(t_sync - t0));
        logger.logf(io::LOG_DEBUG, "fluid: sim time: %.2f ms",
                    TIMELOG_ms(t_sim - t_sync));
#endif
    }
    {
        std::lock_guard<std::mutex> lock(m_worker_task_mutex);
        m_worker_terminate = true;
    }
    m_worker_wakeup.notify_all();
    return nullptr;
}

void NativeLabSim::coordinator_run_workers()
{
    {
        std::lock_guard<std::mutex> lock(m_worker_done_mutex);
        assert(m_worker_stopped == m_worker_count);
        m_worker_stopped = 0;
    }
    {
        std::lock_guard<std::mutex> lock(m_worker_task_mutex);
        assert(m_worker_to_start == 0);
        m_worker_to_start = m_worker_count;
        // make sure all blocks run, we don’t need memory ordering, the mutex
        // implicitly orders
        m_worker_block_ctr.store(0, std::memory_order_relaxed);
    }
    // start all workers
    m_worker_wakeup.notify_all();

    // wait for all workers to finish
    {
        std::unique_lock<std::mutex> lock(m_worker_done_mutex);
        while (m_worker_stopped < m_worker_count) {
            m_worker_done_wakeup.wait(lock);
        }
        assert(m_worker_stopped == m_worker_count);
    }
    // some assertions
    assert(m_worker_block_ctr.load(std::memory_order_relaxed) >=
           m_block_count);
    assert(m_worker_to_start == 0);
}


template <typename buffer_t, typename cell_t>
void get_neighbours(buffer_t &buffer,
                    CoordInt x0, CoordInt y0,
                    CoordInt width, CoordInt height,
                    std::array<cell_t*, 4> &neighbours)
{
    if (x0 > 0) {
        neighbours[Left] = &buffer[(x0-1)+y0*width];
    } else {
        neighbours[Left] = nullptr;
    }

    if (y0 > 0) {
        neighbours[Top] = &buffer[x0+(y0-1)*width];
    } else {
        neighbours[Top] = nullptr;
    }

    if (x0 < width-1) {
        neighbours[Right] = &buffer[(x0+1)+y0*width];
    } else {
        neighbours[Right] = nullptr;
    }

    if (y0 < height-1) {
        neighbours[Bottom] = &buffer[x0+(y0+1)*width];
    } else {
        neighbours[Bottom] = nullptr;
    }
}

template <unsigned int dir, int flow_sign>
static inline SimFloat air_flow(
        LabCell &back,
        const LabCell &front,
        const LabCellMeta &meta,
        const LabCell &neigh_front,
        const LabCellMeta &neigh_meta,
        const LabCell &flow_source)
{
    if (neigh_meta.blocked || meta.blocked) {
        return 0;
    }

    const SimFloat dpressure = front.air_pressure - neigh_front.air_pressure;
    const SimFloat dtemp = (dir == 1
                            ? (neigh_front.air_pressure > 1e-17f && front.air_pressure > 1e-17f
                               ? front.heat_energy/front.air_pressure - neigh_front.heat_energy/neigh_front.air_pressure
                               : 0)
                            : 0);
    const SimFloat temp_flow = flow_sign*(dtemp < 0 ? dtemp * ILabSim::convection_factor : 0);
    const SimFloat press_flow = dpressure * ILabSim::air_diffusion_factor;
    const SimFloat flow = (
                flow_sign*flow_source.flow[dir] * ILabSim::air_flow_factor +
                (press_flow + temp_flow) * (SimFloat(1.0) - ILabSim::air_flow_factor));

    SimFloat applicable_flow =
            clamp(flow,
                  -neigh_front.air_pressure / SimFloat(4.),
                  front.air_pressure / SimFloat(4.));

    /*if (applicable_flow > 0 && front.air_pressure - applicable_flow < 1e-4) {
        applicable_flow = front.air_pressure - 1e-4;
    }
    if (applicable_flow < 0 && neigh_front.air_pressure + applicable_flow < 1e-4) {
        applicable_flow = -(neigh_front.air_pressure - 1e-4);
    }*/

    back.air_pressure -= applicable_flow;

    if (applicable_flow == 0) {
        return applicable_flow;
    }

    const SimFloat tc_flow = applicable_flow;
    const SimFloat energy_flow = (
                applicable_flow > 0
                ? front.heat_energy / front.air_pressure * tc_flow
                : neigh_front.heat_energy / neigh_front.air_pressure * tc_flow);

    if (std::isnan(energy_flow)) {
        std::cout << applicable_flow
                  << " " << front.heat_energy << " " << front.air_pressure
                  << "; " << neigh_front.heat_energy << " " << neigh_front.air_pressure << std::endl;
    }
    assert(!std::isnan(energy_flow));

    back.heat_energy -= energy_flow;

    const SimFloat fog_flow = (
                applicable_flow > 0
                ? front.fog_density / front.air_pressure * tc_flow
                : neigh_front.fog_density / neigh_front.air_pressure * tc_flow);

    assert(!std::isnan(fog_flow));

    back.fog_density -= fog_flow;

    return applicable_flow;
}

template <unsigned int dir, int flow_sign>
static inline void temperature_flow(
        LabCell &back,
        const LabCell &front,
        const LabCellMeta &meta,
        const LabCell &neigh_front,
        const LabCellMeta &neigh_meta)
{
    const SimFloat tc = (
                meta.blocked
                ? meta.obj->info.temp_coefficient
                : front.air_pressure * airtempcoeff_per_pressure);
    const SimFloat neigh_tc = (
                neigh_meta.blocked
                ? neigh_meta.obj->info.temp_coefficient
                : neigh_front.air_pressure * airtempcoeff_per_pressure);

    if (tc < 1e-17 || neigh_tc < 1e-17) {
        return;
    }

    const SimFloat temp = front.heat_energy / tc;
    const SimFloat neigh_temp = neigh_front.heat_energy / neigh_tc;

    const SimFloat dtemp = neigh_temp - temp;

    const SimFloat energy_flow_raw = (
                dtemp > 0
                ? neigh_tc * dtemp
                : tc * dtemp);

    const SimFloat energy_flow = clamp(
                energy_flow_raw * ILabSim::heat_diffusion_factor,
                -front.heat_energy / SimFloat(4.),
                neigh_front.heat_energy / SimFloat(4.)
                );

    if (std::isnan(energy_flow)) {
        std::cout << energy_flow_raw
                  << -front.heat_energy << "; " << neigh_front.heat_energy
                  << std::endl;
    }
    assert(!std::isnan(energy_flow));

    back.heat_energy += energy_flow;
    // assert(abs(energy_flow) < 100);

    if ((energy_flow > 0 && neigh_temp < temp) || (energy_flow <= 0 && temp < neigh_temp)) {
        const double total = neigh_front.heat_energy + front.heat_energy;
        const double avg_temp = total / (tc + neigh_tc);
        if (std::isnan(avg_temp) || std::isnan(tc)) {
            std::cout << avg_temp << " " << total
                      << -front.heat_energy << "; " << neigh_front.heat_energy
                      << std::endl;
        }

        back.heat_energy = avg_temp * tc;
    }
}

template <unsigned int dir, int flow_sign>
static inline void fog_flow(
        LabCell &back,
        const LabCell &front,
        const LabCellMeta &meta,
        const LabCell &neigh_front,
        const LabCellMeta &neigh_meta)
{
    if (meta.blocked || neigh_meta.blocked) {
        return;
    }

    const SimFloat tc = front.air_pressure;
    const SimFloat neigh_tc = neigh_front.air_pressure;

    if (tc < 1e-17 || neigh_tc < 1e-17) {
        return;
    }

    const SimFloat temp = front.fog_density / tc;
    const SimFloat neigh_temp = neigh_front.fog_density / neigh_tc;

    const SimFloat dtemp = neigh_temp - temp;

    const SimFloat fog_flow_raw = (
                dtemp > 0
                ? neigh_tc * dtemp
                : tc * dtemp);

    const SimFloat fog_flow = clamp(
                fog_flow_raw * ILabSim::fog_diffusion_factor,
                -front.fog_density / SimFloat(4.),
                neigh_front.fog_density / SimFloat(4.)
                );

    if (std::isnan(fog_flow)) {
        std::cout << fog_flow_raw
                  << -front.fog_density << "; " << neigh_front.fog_density
                  << std::endl;
    }
    assert(!std::isnan(fog_flow));

    back.fog_density += fog_flow;
    // assert(abs(energy_flow) < 100);

    if ((fog_flow > 0 && neigh_temp < temp) || (fog_flow <= 0 && temp < neigh_temp)) {
        const double total = neigh_front.fog_density + front.fog_density;
        const double avg_temp = total / (tc + neigh_tc);
        if (std::isnan(avg_temp) || std::isnan(tc)) {
            std::cout << avg_temp << " " << total
                      << -front.fog_density << "; " << neigh_front.fog_density
                      << std::endl;
        }

        back.fog_density = avg_temp * tc;
    }
}

template <unsigned int dir>
static inline void full_flow(
        LabCell &back,
        const LabCell &front,
        const LabCellMeta &meta,
        const LabCell &left_front,
        const LabCellMeta &left_meta,
        const LabCell &right_front,
        const LabCellMeta &right_meta)
{
    SimFloat incoming_flow = 0;
    SimFloat incoming_weight = 0;
    {
        const SimFloat applicable_flow = air_flow<dir, -1>(back, front, meta, left_front, left_meta, left_front);
        if (applicable_flow < 0) {
            incoming_flow += left_front.flow[dir] * (-applicable_flow);
            incoming_weight -= applicable_flow;
        }
    }
    temperature_flow<dir, -1>(back, front, meta, left_front, left_meta);
    //fog_flow<dir, -1>(back, front, meta, left_front, left_meta);

    {
        const SimFloat applicable_flow = air_flow<dir, 1>(back, front, meta, right_front, right_meta, front);
        if (applicable_flow < 0) {
            // flow == momentum
            // we want to mix the momentum of the imported air (flow from source)
            // with the momentum the cell already has (== air_flow)
            incoming_flow += right_front.flow[dir] * (-applicable_flow);
            incoming_weight -= applicable_flow;
            /*const SimFloat mixing_factor = (
                        (-applicable_flow) / back.air_pressure
                        );*/
        }
        const SimFloat mixing_factor =
                (back.air_pressure > 1e-17f
                 ? incoming_weight / back.air_pressure
                 : 0);
        back.flow[dir] = (1-mixing_factor)*applicable_flow
                + (back.air_pressure > 1e-17f ? incoming_flow / back.air_pressure : 0);
    }
    temperature_flow<dir, 1>(back, front, meta, right_front, right_meta);
    //fog_flow<dir, 1>(back, front, meta, right_front, right_meta);
}

void NativeLabSim::update_cell(const CoordInt x, const CoordInt y)
{
    LabCell &back = writable_cell_at(x, y);
    const LabCell &front = front_cell_at(x, y);
    LabCellMeta &meta = meta_at(x, y);

    std::array<const LabCell*, 4> neigh;
    std::array<const LabCellMeta*, 4> neigh_meta;

    get_neighbours(m_front_cells, x, y, m_width, m_height, neigh);
    get_neighbours(m_meta_cells, x, y, m_width, m_height, neigh_meta);

    back.air_pressure = front.air_pressure;
    back.heat_energy = front.heat_energy;
    back.fog_density = front.fog_density;
    back.flow = front.flow;

    {
        const LabCell *left = first<const LabCell*>(neigh[Left], &m_null_cell);
        const LabCell *right = first<const LabCell*>(neigh[Right], &m_null_cell);
        const LabCellMeta *left_meta = first<const LabCellMeta*>(
                    neigh_meta[Left],
                    &m_null_cell_meta);
        const LabCellMeta *right_meta = first<const LabCellMeta*>(
                    neigh_meta[Right],
                    &m_null_cell_meta);
        full_flow<0>(
                    back, front,
                    meta,
                    *left, *left_meta,
                    *right, *right_meta);
    }

    {
        const LabCell *left = first<const LabCell*>(neigh[Top], &m_null_cell);
        const LabCell *right = first<const LabCell*>(neigh[Bottom], &m_null_cell);
        const LabCellMeta *left_meta = first<const LabCellMeta*>(
                    neigh_meta[Top],
                    &m_null_cell_meta);
        const LabCellMeta *right_meta = first<const LabCellMeta*>(
                    neigh_meta[Bottom],
                    &m_null_cell_meta);
        full_flow<1>(
                    back, front,
                    meta,
                    *left, *left_meta,
                    *right, *right_meta);
    }
}

void NativeLabSim::update_active_block(const CoordInt y0, const CoordInt y1)
{
    assert(y0 < y1);
    assert(y0 >= 0);
    assert(y1 <= m_height);
    const CoordInt width = m_width;
    // we write to the backbuffer and read from the frontbuffer
    for (CoordInt y = y0; y < y1; ++y) {
        for (CoordInt x = 0; x < width; ++x) {
            update_cell(x, y);
        }
    }
}

void *NativeLabSim::worker_impl()
{
    _mm_setcsr( _mm_getcsr() | 0x0040 );
    _mm_setcsr( _mm_getcsr() | 0x8000 );
    feenableexcept(FE_ALL_EXCEPT & ~(FE_INEXACT | FE_UNDERFLOW));

    const unsigned int out_of_tasks_limit = m_block_count;

    std::unique_lock<std::mutex> wakeup_lock(m_worker_task_mutex);
    while (!m_worker_terminate)
    {
        while (m_worker_to_start == 0 &&
               !m_worker_terminate)
        {
            m_worker_wakeup.wait(wakeup_lock);
        }
        if (m_worker_terminate) {
            return nullptr;
        }
        --m_worker_to_start;
        wakeup_lock.unlock();

        while (1) {
            const unsigned int my_block = m_worker_block_ctr.fetch_add(
                        1,
                        std::memory_order_relaxed);
            if (my_block >= out_of_tasks_limit)
            {
                break;
            }

            const unsigned int y0 = my_block * ROWS_PER_BLOCK;
            update_active_block(y0, y0+ROWS_PER_BLOCK);
        }

        {
            std::lock_guard<std::mutex> lock(m_worker_done_mutex);
            m_worker_stopped += 1;
        }
        m_worker_done_wakeup.notify_all();

        wakeup_lock.lock();
    }
    return nullptr;
}

void NativeLabSim::init_metadata(
        std::vector<LabCellMeta> &buffer,
        CoordInt x,
        CoordInt y)
{
    LabCellMeta &cell = buffer[x+m_width*y];
    cell.blocked = false;
    cell.obj = nullptr;
}

void NativeLabSim::init_cell(
        std::vector<LabCell> &buffer,
        CoordInt x,
        CoordInt y,
        SimFloat air_pressure,
        SimFloat temperature,
        SimFloat fog_density)
{
    LabCell &cell = buffer[x+m_width*y];
    cell.air_pressure = air_pressure;
    cell.heat_energy = temperature * (
                airtempcoeff_per_pressure * cell.air_pressure);
    cell.flow = Vector2f(0, 0);
    cell.fog_density = fog_density;
}

void NativeLabSim::clear_cells(
        const CoordInt dx,
        const CoordInt dy,
        const Stamp &stamp)
{
    assert(!m_run);

    uintptr_t stamp_cells_len = 0;
    const CoordPair *const stamp_cells = stamp.get_map_coords(&stamp_cells_len);

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        const CoordPair p = stamp_cells[i];
        const CoordInt x = p.x + dx;
        const CoordInt y = p.y + dy;

        LabCell *const curr_cell = safe_writable_cell_at(x, y);
        if (!curr_cell) {
            continue;
        }
        LabCellMeta &curr_meta = meta_at(x, y);
        init_cell(m_front_cells, x, y, 0, 0, 0);
        init_cell(m_back_cells, x, y, 0, 0, 0);
        curr_meta.blocked = false;
    }
}

void NativeLabSim::apply_temperature_stamp(
        const CoordInt x, const CoordInt y,
        const Stamp &stamp, const double temperature)
{
    uintptr_t stamp_cells_len = 0;
    const CoordPair *cell_coord = stamp.get_map_coords(&stamp_cells_len);
    cell_coord--;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        cell_coord++;

        const CoordInt cx = x + cell_coord->x;
        const CoordInt cy = y + cell_coord->y;

        LabCell *cell = safe_writable_cell_at(cx, cy);
        if (!cell) {
            continue;
        }

        const LabCellMeta &meta = meta_at(cx, cy);
        if (meta.blocked) {
            cell->heat_energy = temperature * meta.obj->info.temp_coefficient;
        } else {
            cell->heat_energy = temperature * (
                        airtempcoeff_per_pressure*cell->air_pressure);
        }
    }
}

void NativeLabSim::apply_fog_effect_stamp(
        const CoordInt x, const CoordInt y,
        const Stamp &stamp,
        const SimFloat intensity)
{
    assert(!m_running);

    uintptr_t stamp_cells_len = 0;
    const CoordPair *cell_coord = stamp.get_map_coords(&stamp_cells_len);
    cell_coord--;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        cell_coord++;

        const CoordInt cx = x + cell_coord->x;
        const CoordInt cy = y + cell_coord->y;

        LabCell *cell = safe_writable_cell_at(cx, cy);
        if (!cell) {
            continue;
        }

        const LabCellMeta &meta = meta_at(cx, cy);
        if (meta.blocked) {
            continue;
        }

        cell->fog_density = clamp(cell->fog_density + intensity,
                                  SimFloat(0),
                                  SimFloat(1));
    }
}

void NativeLabSim::apply_pressure_stamp(
        const CoordInt x, const CoordInt y,
        const Stamp &stamp,
        const float new_pressure)
{
    assert(!m_running);

    uintptr_t stamp_cells_len = 0;
    const CoordPair *cell_coord = stamp.get_map_coords(&stamp_cells_len);
    cell_coord--;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        cell_coord++;

        const CoordInt cx = x + cell_coord->x;
        const CoordInt cy = y + cell_coord->y;

        LabCell *cell = safe_writable_cell_at(cx, cy);
        if (!cell) {
            continue;
        }

        const LabCellMeta &meta = meta_at(cx, cy);
        if (meta.blocked) {
            continue;
        }

        cell->air_pressure = new_pressure;
    }
}

void NativeLabSim::reset_unblocked_cells(const SimFloat pressure,
                                         const SimFloat temperature,
                                         const SimFloat fog_density)
{
    assert(!m_running);

    const SimFloat heat_energy = temperature * airtempcoeff_per_pressure * pressure;

    LabCellMeta *meta = &m_meta_cells[0];
    for (LabCell &cell: m_back_cells) {
        if (meta->blocked) {
            meta++;
            continue;
        }

        cell.flow = Vector2f();
        cell.fog_density = fog_density;
        cell.heat_energy = heat_energy;
        cell.air_pressure = pressure;

        meta++;
    }
}

void NativeLabSim::apply_flow_stamp(
        const CoordInt x, const CoordInt y,
        const Stamp &stamp,
        const Vector2f flow, const SimFloat blend)
{
    assert(!m_running);

    uintptr_t stamp_cells_len = 0;
    const CoordPair *cell_coord = stamp.get_map_coords(&stamp_cells_len);
    cell_coord--;

    const SimFloat inv_blend = SimFloat(1) - blend;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        cell_coord++;

        const CoordInt cx = x + cell_coord->x;
        const CoordInt cy = y + cell_coord->y;

        LabCell *cell = safe_writable_cell_at(cx, cy);
        if (!cell) {
            continue;
        }

        const LabCellMeta &meta = meta_at(cx, cy);
        if (meta.blocked) {
            continue;
        }

        cell->flow = flow * blend + cell->flow * inv_blend;
    }
}

void NativeLabSim::move_stamp(
        const CoordInt oldx, const CoordInt oldy,
        const CoordInt newx, const CoordInt newy,
        const Stamp &stamp,
        const CoordPair *const vel)
{
    assert(!m_running);

    static CellInfo cells[cell_stamp_length];

    uintptr_t write_index = 0;

    uintptr_t stamp_cells_len = 0;
    const CoordPair *stamp_cells = stamp.get_map_coords(&stamp_cells_len);
    stamp_cells--;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        assert(write_index < (uintptr_t)cell_stamp_length);
        stamp_cells++;

        const CoordInt x = oldx + stamp_cells->x;
        const CoordInt y = oldy + stamp_cells->y;

        LabCell *cell = safe_writable_cell_at(x, y);
        if (!cell) {
            continue;
        }
        LabCellMeta &meta = meta_at(x, y);

        CellInfo *dst = &cells[write_index];
        memcpy(&dst->offs, stamp_cells, sizeof(CoordPair));
        memcpy(&dst->phys, cell, sizeof(LabCell));
        memcpy(&dst->meta, &meta, sizeof(LabCellMeta));
        write_index++;
        init_cell(m_front_cells, x, y, 0, 0, 0);
        init_cell(m_back_cells, x, y, 0, 0, 0);
        meta.blocked = false;
        meta.obj = nullptr;
    }

    // logger.logf(io::LOG_DEBUG, "move_stamp: placing new stamp");
    place_stamp(newx, newy, cells, write_index, vel);
    // logger.logf(io::LOG_DEBUG, "move_stamp: placed new stamp");
}

void NativeLabSim::place_object(
        const CoordInt dx, const CoordInt dy,
        GameObject *obj,
        const double initial_temperature)
{
    assert(!m_running);

    static CellInfo cells[cell_stamp_length];

    uintptr_t stamp_cells_len = 0;
    const CoordPair *stamp_cells = obj->info.stamp.get_map_coords(
                &stamp_cells_len);

    double heat_energy = initial_temperature * obj->info.temp_coefficient;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        CellInfo *const dst = &cells[i];
        dst->offs = stamp_cells[i];
        dst->phys = {};
        dst->phys.heat_energy = heat_energy;
        dst->phys.flow[0] = dst->offs.x - ((float)subdivision_count / 2);
        dst->phys.flow[1] = dst->offs.y - ((float)subdivision_count / 2);
        dst->meta.blocked = true;
        dst->meta.obj = obj;
    }

    logger.logf(io::LOG_DEBUG, "place_object: placing stamp for %p", obj);
    place_stamp(dx, dy, cells, stamp_cells_len);
    logger.logf(io::LOG_DEBUG, "place_object: placed stamp for %p", obj);
}

void NativeLabSim::place_stamp(
        const CoordInt atx, const CoordInt aty,
        const CellInfo *cells, const uintptr_t cells_len,
        const CoordPair *const vel)
{
    assert(!m_running);

    // to iterate over neighbouring cells
    static const intptr_t offs[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    // buffers to keep temporary data. we keep them in a static variable.
    // XXX: This will break for more than one Automaton instance!
    const intptr_t index_row_length = subdivision_count+2;
    const intptr_t index_length = index_row_length * index_row_length;
    static intptr_t border_indicies[index_length];
    static LabCell *border_cells[index_length];
    static double border_cell_weights[index_length];

    intptr_t border_cell_write_index = 0;
    intptr_t border_cell_count = 0;
    double border_cell_weight = 0;

    memset(border_indicies, -1, index_length * sizeof(intptr_t));
    memset(border_cells, 0, index_length * sizeof(LabCell*));

    // collect surplus matter here
    double air_to_distribute = 0.;
    double heat_to_distribute = 0.;
    double fog_to_distribute = 0.;

    // velocity information for quick and easy access. The kind we
    // initialize these variables in special cases allows us to avoid
    // several checks later.
    const double vel_norm = (vel != nullptr ? (vel->norm() != 0 ? vel->norm() : 0) : 0);
    const double vel_x = (vel_norm > 0 ? vel->x / vel_norm : 0);
    const double vel_y = (vel_norm > 0 ? vel->y / vel_norm : 0);

    for (uintptr_t i = 0; i < cells_len; i++) {
        const CoordPair p = cells[i].offs;
        const CoordInt x = p.x + atx;
        const CoordInt y = p.y + aty;

        LabCell *const curr_cell = safe_writable_cell_at(x, y);
        if (!curr_cell) {
            continue;
        }
        LabCellMeta &curr_meta = meta_at(x, y);
        if (curr_meta.blocked) {
            std::cout << curr_meta.obj << std::endl;
            assert(!curr_meta.blocked);
        }

        if (!curr_meta.blocked) {
            air_to_distribute += curr_cell->air_pressure;
            heat_to_distribute += curr_cell->heat_energy;
            fog_to_distribute += curr_cell->fog_density;
        }
        memcpy(curr_cell, &cells[i].phys, sizeof(LabCell));
        /* this is an ugly hack; we also set the frontbuffer data of the cell
         * here; this is needed so that newly placed objects can see their
         * actual values instead of what was in the frontbuffer before. */
        memcpy(&writable_front_cell_at(x, y), &cells[i].phys, sizeof(LabCell));
        memcpy(&curr_meta, &cells[i].meta, sizeof(LabCellMeta));

        for (uintptr_t j = 0; j < 4; j++) {
            const intptr_t index_cell = (p.y + offs[j][1] + 1) * index_row_length + p.x + 1 + offs[j][0];

            if (border_indicies[index_cell] != -1) {
                // inspected earlier, no need to check again
                const intptr_t cell_index = border_indicies[index_cell];
                if (cell_index >= 0 && vel_norm > 0) {
                    // in this case, we make sure we get the maximum
                    // weight for this cell
                    const double cell_weight = max((vel_norm > 0 ? offs[j][0] * vel_x + offs[j][1] * vel_y : 1), 0);
                    const double old_weight = border_cell_weights[cell_index];
                    if (old_weight < cell_weight) {
                        border_cell_weight -= old_weight;
                        border_cell_weight += cell_weight;
                        border_cell_weights[cell_index] = cell_weight;
                    }
                }
                continue;
            }

            const intptr_t nx = x + offs[j][0];
            const intptr_t ny = y + offs[j][1];

            LabCell *const neigh_cell = safe_writable_cell_at(nx, ny);
            if (!neigh_cell) {
                border_indicies[index_cell] = -2;
                continue;
            }

            LabCellMeta &neigh_meta = meta_at(nx, ny);
            if (neigh_meta.blocked) {
                border_indicies[index_cell] = -2;
                continue;
            }

            const double cell_weight = max((vel_norm > 0 ? offs[j][0] * vel_x + offs[j][1] * vel_y : 1), 0);

            border_indicies[index_cell] = border_cell_write_index;
            border_cells[border_cell_write_index] = neigh_cell;
            border_cell_weights[border_cell_write_index] = cell_weight;
            assert(border_cell_write_index < index_length);
            border_cell_write_index++;
            border_cell_count++;
            border_cell_weight += cell_weight;
        }

        const uintptr_t index_cell = (p.y + 1) * index_row_length + (p.x + 1);
        if (border_indicies[index_cell] >= 0) {
            border_cell_count--;
            border_cells[border_indicies[index_cell]] = 0;
            border_cell_weight -= border_cell_weights[border_indicies[index_cell]];
        }
        border_indicies[index_cell] = -2;

        assert(!std::isnan(curr_cell->heat_energy));
    }

    if (air_to_distribute == 0 && fog_to_distribute == 0)
        return;

    if (border_cell_count == 0) {
        fprintf(stderr, "[PHY!] [NC] no cells to move stuff to\n");
        return;
    }

    const double weight_to_use = (border_cell_weight > 0 ? border_cell_weight : border_cell_count);

    const double air_per_cell = air_to_distribute / weight_to_use;
    const double heat_per_cell = heat_to_distribute / weight_to_use;
    const double fog_per_cell = fog_to_distribute / weight_to_use;

    unsigned int j = 0;
    double *neigh_cell_weight = &border_cell_weights[0];
    for (LabCell **neigh_cell = &border_cells[0]; j < border_cell_count; neigh_cell++) {
        if (!(*neigh_cell)) {
            neigh_cell_weight++;
            continue;
        }
        const double cell_weight = (border_cell_weight > 0 ? *neigh_cell_weight : 1);
        (*neigh_cell)->air_pressure += air_per_cell * cell_weight;
        (*neigh_cell)->heat_energy += heat_per_cell * cell_weight;
        (*neigh_cell)->fog_density += fog_per_cell * cell_weight;

        assert(!std::isnan((*neigh_cell)->heat_energy));
        j++;
        neigh_cell_weight++;
    }
}

SimFloat NativeLabSim::measure_stamp_avg(
        const CoordInt atx,
        const CoordInt aty,
        const CoordPair *coords,
        const uintptr_t coords_len,
        const std::function<SimFloat(const LabCell &)> &sensor,
        bool exclude_blocked) const
{
    if (!coords_len) {
        return NAN;
    }

    float accum = 0.f;
    float hits = 0.f;
    for (uintptr_t i = 0; i < coords_len; ++i) {
        const CoordPair p = coords[i];
        const CoordInt x = p.x + atx;
        const CoordInt y = p.y + aty;
        const LabCell *const cell = safe_front_cell_at(x, y);
        if (!cell) {
            continue;
        }
        if (exclude_blocked) {
            const LabCellMeta &meta = meta_at(x, y);
            if (meta.blocked) {
                continue;
            }
        }
        hits += 1;
        accum += sensor(*cell);
    }

    if (hits == 0) {
        return NAN;
    }

    return accum / hits;
}

Vector<SimFloat, 2> NativeLabSim::measure_stamp_gradient(
        const CoordInt atx,
        const CoordInt aty,
        const CoordPair *coords,
        const uintptr_t coords_len,
        const std::function<SimFloat (const LabCell &)> &sensor,
        bool exclude_blocked) const
{
    Vector<SimFloat, 2> accum(0, 0);
    float hits = 0;

    for (uintptr_t i = 0; i < coords_len; ++i) {
        const CoordPair p = coords[i];
        if (p.x == 0 && p.y == 0) {
            continue;
        }

        const CoordInt x = p.x + atx;
        const CoordInt y = p.y + aty;
        Vector<SimFloat, 2> dx(p.x - 2.5, p.y - 2.5);
        dx.normalize();

        const LabCell *const cell = safe_front_cell_at(x, y);
        if (!cell) {
            continue;
        }
        if (exclude_blocked) {
            const LabCellMeta &meta = meta_at(x, y);
            if (meta.blocked) {
                continue;
            }
        }

        hits += 1;
        accum += sensor(*cell) * dx;
    }

    if (hits == 0) {
        return Vector<SimFloat, 2>(NAN, NAN);
    }

    return accum / hits;
}

void NativeLabSim::start_frame()
{
    std::swap(m_front_cells, m_back_cells);
    m_running = true;
    {
        std::unique_lock<std::mutex> lock(m_control_mutex);
        assert(!m_run);
        m_run = true;
    }
    m_control_wakeup.notify_all();
}

void NativeLabSim::set_blocked(CoordInt x, CoordInt y, bool blocked)
{
    assert(!m_running);
    m_meta_cells[x+m_width*y].blocked = blocked;
}

void NativeLabSim::wait_for_frame()
{
    if (!m_running) {
        return;
    }

    std::unique_lock<std::mutex> lock(m_done_mutex);
    while (!m_done) {
        m_done_wakeup.wait(lock);
    }
    m_done = false;
    m_running = false;
}

void NativeLabSim::to_gl_texture(
        const double min, const double max,
        bool thread_regions)
{
    m_rgba_buffer.resize(m_width*m_height);

    const CoordInt half = m_width / 2;

    uint32_t *target = &m_rgba_buffer[0];
    LabCell *source = &m_back_cells[0];
    LabCellMeta *meta_source = &m_meta_cells[0];
    for (CoordInt i = 0; i < m_width*m_height; i++) {
        if (meta_source->blocked) {
            *target = 0x0000FF;
        } else {
            const bool right = (i % m_height) >= half;
            const unsigned char press_color = (unsigned char)(clamp((source->air_pressure - min) / (max - min), 0.0, 1.0) * 255.0);
            // const double temperature = (meta_source->blocked ? source->heat_energy / meta_source->obj->info.temp_coefficient : source->heat_energy / (source->air_pressure * airtempcoeff_per_pressure));
            const double fog = (meta_source->blocked ? 0 : source->fog_density);
            // const unsigned char temp_color = (unsigned char)(clamp((temperature - min) / (max - min), 0.0, 1.0) * 255.0);
            const unsigned char fog_color = (unsigned char)(clamp((fog - min) / (max - min), 0.0, 1.0) *255.0);
            const unsigned char b = (right ? fog_color : press_color);
            const unsigned char r = b;
            if (thread_regions) {
                const unsigned char g = (unsigned char)((double)(int)(((double)(i / m_width)) / m_height * m_worker_count) / m_worker_count * 255.0);
                //const unsigned char b = (unsigned char)(clamp((source->flow[1] - min) / (max - min), -1.0, 1.0) * 127.0 + 127.0);
                *target = r | (g << 8) | (r << 16);
            } else {
                *target = r | (b << 8) | (b << 16);
            }
        }
        target++;
        source++;
        meta_source++;
    }

    glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0, 0,
                m_width,
                m_height,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                (const GLvoid*)m_rgba_buffer.data());
}

void NativeLabSim::data_to_gl_texture()
{
    m_data_buffer.resize(m_width*m_height);

    const LabCell *source = &m_front_cells[0];
    const LabCellMeta *meta = &m_meta_cells[0];
    Vector4f *dest = &m_data_buffer[0];
    for (CoordInt i = 0; i < m_width*m_height; ++i) {
        /*const Vector<SimFloat, 2> flow = source->flow * (meta->blocked ? 0 : 1);
        *dest++ = Vector4f(
                    flow[eX],
                    flow[eY],
                    source->air_pressure,
                    meta->blocked ? 1 : 0
                                    );*/
        *dest++ = Vector4f(
                    meta->blocked ? meta->obj->info.temp_coefficient : source->air_pressure,
                    source->fog_density,
                    source->heat_energy,
                    meta->blocked ? 1 : 0
                                    );
        ++source;
        ++meta;
    }

    glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                0, 0,
                m_width, m_height,
                GL_RGBA,
                GL_FLOAT,
                (const GLvoid*)m_data_buffer.data());
}


LabCellMeta::LabCellMeta():
    blocked(false),
    obj(nullptr)
{

}

LabCell::LabCell():
    air_pressure(0),
    heat_energy(0),
    flow{0, 0},
    fog_density(0)
{

}

ILabSim::~ILabSim()
{

}
