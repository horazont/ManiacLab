/**********************************************************************
File name: Physics.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "Physics.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>

#include <glew.h>

#include "GameObject.hpp"

using namespace PyEngine;

inline double clamp(const double value, const double min, const double max)
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

Automaton::Automaton(
        CoordInt width, CoordInt height,
        const SimulationConfig &config,
        bool mp,
        double initial_pressure,
        double initial_temperature):
    _resumed(false),
    _width(width),
    _height(height),
    _metadata(new CellMetadata[width*height]()),
    _cells(new Cell[width*height]()),
    _backbuffer(new Cell[width*height]()),
    _config(config),
    _thread_count(mp?(get_hardware_thread_count()):1),
    _finished_signal(),
    _resume_signals(_thread_count),
    _forward_signals(_thread_count),
    _shared_zones(_thread_count),
    _threads(_thread_count),
    _rgba_buffer(0)
{
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            init_metadata(_metadata, x, y);
            init_cell(_cells, x, y, initial_pressure, initial_temperature);
            init_cell(_backbuffer, x, y, initial_pressure, initial_temperature);
        }
    }
    init_threads();
}

Automaton::~Automaton()
{
    std::cout << "destroying automaton" << std::endl;
    wait_for();
    if (_rgba_buffer) {
        free(_rgba_buffer);
    }
}

void Automaton::init_metadata(CellMetadata *buffer, CoordInt x,
    CoordInt y)
{
    CellMetadata *cell = &buffer[x+_width*y];
    cell->blocked = false;
    cell->obj = 0;
}

void Automaton::init_cell(Cell *buffer, CoordInt x, CoordInt y,
    double initial_pressure, double initial_temperature)
{
    Cell *cell = &buffer[x+_width*y];
    cell->air_pressure = initial_pressure;
    cell->heat_energy = initial_temperature * (
        airtempcoeff_per_pressure * cell->air_pressure);
    cell->flow[0] = 0;
    cell->flow[1] = 0;
    cell->fog = 0;
}

void Automaton::init_threads()
{
    // We limit the thread count to 64 for now. Above that, synchronization is
    // probably more expensive than everything else. Synchronization is O(n),
    // with n being the count of threads. Threads have to prepare the bottommost
    // and topmost row of their slice first and raise some flags, as other
    // threads need the information from the prepared cells in the backbuffer.
    // Only after the neighbouring threads have prepared the neighbouring cells,
    // the actual calculation can start.
    // Additionally, everything will break if we have an amount of threads for
    // which the height divided by the thread count (integer division) gives
    // zero (which is asserted against below).
    const CoordInt slice_size =
        _height / (_thread_count <= 64 ? _thread_count : 64);

    assert(slice_size > 0);

    CoordInt slice_y0 = 0;

    for (unsigned int i = 0; i < _thread_count - 1; i++) {
        _threads[i] = std::unique_ptr<AutomatonThread>(
            new AutomatonThread(
                *this,
                // range on which this thread works
                slice_y0, slice_y0 + slice_size-1,
                _finished_signal,
                (i > 0
                 ? &_forward_signals[i-1]
                 : nullptr),
                &_forward_signals[i],
                (i > 0
                 ? &_shared_zones[i-1]
                 : nullptr),
                &_shared_zones[i],
                _resume_signals[i]
            ));
        slice_y0 += slice_size;
    }

    _threads[_thread_count-1] = std::unique_ptr<AutomatonThread>(
        new AutomatonThread(
            *this,
            slice_y0, _height-1,
            _finished_signal,
            (_thread_count > 1
             ? &_forward_signals[_thread_count-2]
             : nullptr),
            nullptr,
            (_thread_count > 1
             ? &_shared_zones[_thread_count-2]
             : nullptr),
            nullptr,
            _resume_signals[_thread_count-1]
        ));
}

void Automaton::clear_cells(
    const CoordInt dx,
    const CoordInt dy,
    const Stamp &stamp)
{
    assert(!_resumed);

    uintptr_t stamp_cells_len = 0;
    const CoordPair *const stamp_cells = stamp.get_map_coords(&stamp_cells_len);

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        const CoordPair p = stamp_cells[i];
        const CoordInt x = p.x + dx;
        const CoordInt y = p.y + dy;

        Cell *const curr_cell = safe_cell_at(x, y);
        if (!curr_cell) {
            continue;
        }
        CellMetadata *const curr_meta = meta_at(x, y);
        init_cell(_cells, x, y, 0, 0);
        init_cell(_backbuffer, x, y, 0, 0);
        curr_meta->blocked = false;
    }
}

void Automaton::apply_temperature_stamp(const CoordInt x, const CoordInt y,
    const Stamp &stamp, const double temperature)
{
    uintptr_t stamp_cells_len = 0;
    const CoordPair *cell_coord = stamp.get_map_coords(&stamp_cells_len);
    cell_coord--;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        cell_coord++;

        const CoordInt cx = x + cell_coord->x;
        const CoordInt cy = y + cell_coord->y;

        Cell *cell = safe_cell_at(cx, cy);
        if (!cell) {
            continue;
        }

        CellMetadata *meta = meta_at(cx, cy);
        if (meta->blocked) {
            cell->heat_energy = temperature * meta->obj->info.temp_coefficient;
        } else {
            cell->heat_energy = temperature * (
                airtempcoeff_per_pressure*cell->air_pressure);
        }
    }
}

void Automaton::get_cell_stamp_at(const CoordInt left, const CoordInt top,
    PhysicsCellStamp *stamp)
{
    Cell **curr_cell = (Cell**)stamp;
    for (CoordInt y = 0; y < subdivision_count; y++) {
        Cell *src_cell = &_cells[top + y];
        for (CoordInt x = 0; x < subdivision_count; x++) {
            *curr_cell = src_cell;
            curr_cell++;
            src_cell++;
        }
    }
}

void Automaton::move_stamp(
    const CoordInt oldx, const CoordInt oldy,
    const CoordInt newx, const CoordInt newy,
    const Stamp &stamp,
    const CoordPair *const vel)
{
    assert(!_resumed);

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

        Cell *cell = safe_cell_at(x, y);
        if (!cell) {
            continue;
        }
        CellMetadata *meta = meta_at(x, y);

        CellInfo *dst = &cells[write_index];
        memcpy(&dst->offs, stamp_cells, sizeof(CoordPair));
        memcpy(&dst->phys, cell, sizeof(Cell));
        memcpy(&dst->meta, meta, sizeof(CellMetadata));
        write_index++;
        init_cell(_cells, x, y, 0, 0);
        init_cell(_backbuffer, x, y, 0, 0);
        meta->blocked = false;
        meta->obj = 0;
    }

    place_stamp(newx, newy, cells, write_index, vel);
}

void Automaton::place_object(
    const CoordInt dx, const CoordInt dy,
    const GameObject *obj,
    const double initial_temperature)
{
    assert(!_resumed);

    static CellInfo cells[cell_stamp_length];

    uintptr_t stamp_cells_len = 0;
    const CoordPair *stamp_cells = obj->info.stamp.get_map_coords(
        &stamp_cells_len);

    double heat_energy = initial_temperature * obj->info.temp_coefficient;

    for (uintptr_t i = 0; i < stamp_cells_len; i++) {
        CellInfo *const dst = &cells[i];
        dst->offs = stamp_cells[i];
        memset(&(dst->phys), 0, sizeof(Cell));
        dst->phys.heat_energy = heat_energy;
        dst->phys.flow[0] = dst->offs.x - ((float)subdivision_count / 2);
        dst->phys.flow[1] = dst->offs.y - ((float)subdivision_count / 2);
        dst->meta.blocked = true;
        dst->meta.obj = obj;
    }

    place_stamp(dx, dy, cells, stamp_cells_len);
}

void Automaton::place_stamp(const CoordInt atx, const CoordInt aty,
    const CellInfo *cells, const uintptr_t cells_len,
    const CoordPair *const vel)
{
    assert(!_resumed);

    // to iterate over neighbouring cells
    static const intptr_t offs[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    // buffers to keep temporary data. we keep them in a static variable.
    // XXX: This will break for more than one Automaton instance!
    const intptr_t index_row_length = subdivision_count+2;
    const intptr_t index_length = index_row_length * index_row_length;
    static intptr_t border_indicies[index_length];
    static Cell *border_cells[index_length];
    static double border_cell_weights[index_length];

    intptr_t border_cell_write_index = 0;
    intptr_t border_cell_count = 0;
    double border_cell_weight = 0;

    memset(border_indicies, -1, index_length * sizeof(intptr_t));
    memset(border_cells, 0, index_length * sizeof(Cell*));

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

        Cell *const curr_cell = safe_cell_at(x, y);
        if (!curr_cell) {
            continue;
        }
        CellMetadata *const curr_meta = meta_at(x, y);
        assert(!curr_meta->blocked);

        if (!curr_meta->blocked) {
            air_to_distribute += curr_cell->air_pressure;
            heat_to_distribute += curr_cell->heat_energy;
            fog_to_distribute += curr_cell->fog;
        }
        memcpy(curr_cell, &cells[i].phys, sizeof(Cell));
        memcpy(curr_meta, &cells[i].meta, sizeof(CellMetadata));

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

            Cell *const neigh_cell = safe_cell_at(nx, ny);
            if (!neigh_cell) {
                border_indicies[index_cell] = -2;
                continue;
            }
            CellMetadata *const neigh_meta = meta_at(nx, ny);
            if (neigh_meta->blocked) {
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

        assert(!isnan(curr_cell->heat_energy));
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
    for (Cell **neigh_cell = &border_cells[0]; j < border_cell_count; neigh_cell++) {
        if (!(*neigh_cell)) {
            neigh_cell_weight++;
            continue;
        }
        const double cell_weight = (border_cell_weight > 0 ? *neigh_cell_weight : 1);
        (*neigh_cell)->air_pressure += air_per_cell * cell_weight;
        (*neigh_cell)->heat_energy += heat_per_cell * cell_weight;
        (*neigh_cell)->fog += fog_per_cell * cell_weight;

        assert(!isnan((*neigh_cell)->heat_energy));
        j++;
        neigh_cell_weight++;
    }
}

void Automaton::resume()
{
    for (auto &sem: _resume_signals) {
        sem.post();
    }
    _resumed = true;
}

void Automaton::set_blocked(CoordInt x, CoordInt y, bool blocked)
{
    assert(!_resumed);
    _metadata[x+_width*y].blocked = blocked;
}

void Automaton::wait_for()
{
    if (!_resumed)
        return;
    for (unsigned int i = 0; i < _thread_count; i++) {
        _finished_signal.wait();
    }
    _resumed = false;
    Cell *tmp = _backbuffer;
    _backbuffer = _cells;
    _cells = tmp;
}

void Automaton::to_gl_texture(
    const double min, const double max,
    bool thread_regions)
{
    if (!_rgba_buffer) {
        _rgba_buffer = (uint32_t*)malloc(_width*_height*4);
    }

    const CoordInt half = _width / 2;

    uint32_t *target = _rgba_buffer;
    Cell *source = _backbuffer;
    CellMetadata *meta_source = _metadata;
    for (CoordInt i = 0; i < _width*_height; i++) {
        if (meta_source->blocked) {
            *target = 0x0000FF;
        } else {
            const bool right = (i % _height) >= half;
            const unsigned char press_color = (unsigned char)(clamp((source->air_pressure - min) / (max - min), 0.0, 1.0) * 255.0);
            // const double temperature = (meta_source->blocked ? source->heat_energy / meta_source->obj->info.temp_coefficient : source->heat_energy / (source->air_pressure * airtempcoeff_per_pressure));
            const double fog = (meta_source->blocked ? 0 : source->fog);
            // const unsigned char temp_color = (unsigned char)(clamp((temperature - min) / (max - min), 0.0, 1.0) * 255.0);
            const unsigned char fog_color = (unsigned char)(clamp((fog - min) / (max - min), 0.0, 1.0) *255.0);
            const unsigned char b = (right ? fog_color : press_color);
            const unsigned char r = b;
            if (thread_regions) {
                const unsigned char g = (unsigned char)((double)(int)(((double)(i / _width)) / _height * _thread_count) / _thread_count * 255.0);
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

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)_rgba_buffer);
}

/* AutomatonThread::AutomatonThread */

AutomatonThread::AutomatonThread(
        Automaton &dataclass,
        CoordInt slice_y0,
        CoordInt slice_y1,
        Semaphore &finished_signal,
        Semaphore *top_shared_ready,
        Semaphore *bottom_shared_forward,
        std::mutex *top_shared_zone,
        std::mutex *bottom_shared_zone,
        Semaphore &resume_signal):
    _finished_signal(finished_signal),
    _top_shared_ready(top_shared_ready),
    _bottom_shared_forward(bottom_shared_forward),
    _top_shared_zone(top_shared_zone),
    _bottom_shared_zone(bottom_shared_zone),
    _resume_signal(resume_signal),
    _dataclass(dataclass),
    _width(dataclass._width),
    _height(dataclass._height),
    _slice_y0(slice_y0),
    _slice_y1(slice_y1),
    _sim(dataclass._config),
    _backbuffer(dataclass._backbuffer),
    _cells(dataclass._cells),
    _metadata(dataclass._metadata),
    _terminated(false),
    _thread(&AutomatonThread::execute, this)
{

}

AutomatonThread::~AutomatonThread()
{
    _terminated = true;
    _resume_signal.post();
    _thread.join();
}

inline void AutomatonThread::activate_cell(Cell *front, Cell *back)
{
    //~ if (back->air_pressure <= 1e-100 && back->air_pressure != 0) {
        //~ front->air_pressure = 0;
        //~ front->flow[0] = 0;
        //~ front->flow[1] = 0;
        //~ (back+_width)->flow[1] = 0;
        //~ (back+1)->flow[0] = 0;
        //~ (front+_width)->flow[1] = 0;
        //~ (front+1)->flow[0] = 0;
        //~ front->heat_energy = 0;
    //~ } else {
    front->air_pressure = back->air_pressure;
    for (int i = 0; i < 2; i++) {
        const double flow = back->flow[i];
        if (!isinf(flow) && abs(flow) < 1e10) {
            front->flow[i] = flow;
        } else {
            front->flow[i] = 0;
            back->flow[i] = 0;
        }
    }
    front->heat_energy = back->heat_energy;
    front->fog = back->fog;
    //~ }
    assert(!isnan(back->air_pressure));
    assert(!isnan(back->fog));
    assert(!isnan(back->heat_energy));
}

template<class CType>
void AutomatonThread::get_cell_and_neighbours(CType *buffer, CType **cell,
        CType *(*neighbours)[2], CoordInt x, CoordInt y)
{
    *cell = &buffer[x+_width*y];
    (*neighbours)[0] = (x > 0) ? &buffer[(x-1)+_width*y] : 0;
    (*neighbours)[1] = (y > 0) ? &buffer[x+_width*(y-1)] : 0;
}

inline double AutomatonThread::flow(const Cell *b_cellA, Cell *f_cellA,
    const Cell *b_cellB, Cell *f_cellB,
    CoordInt direction)
{
    const double dpressure = b_cellA->air_pressure - b_cellB->air_pressure;
    const double dtemp = (direction == 1 ? b_cellA->heat_energy - b_cellB->heat_energy : 0);
    const double temp_flow = (dtemp > 0 ? dtemp * _sim.convection_friction : 0);
    const double press_flow = dpressure * _sim.flow_friction;
    const double old_flow = b_cellA->flow[direction];

    const double tcA = b_cellA->air_pressure;
    const double tcB = b_cellB->air_pressure;

    // This is to take into account inertia of mass the air has. We
    // apply a moving average on the flow vector, which is then used
    // to calculate the flow we're applying this frame.
    const double flow = old_flow * _sim.flow_damping + (temp_flow + press_flow) * (1.0 - _sim.flow_damping);
    const double applicable_flow = clamp(
        flow,
        -tcB / 4.,
        tcA / 4.
    );

    f_cellA->flow[direction] = applicable_flow;

    f_cellA->air_pressure -= applicable_flow;
    f_cellB->air_pressure += applicable_flow;

    // this was once an if which lead to a return -- we're now
    // asserting that this doesn't happen as I'm pretty sure it won't.
    // (and branching is evil)
    assert(! ((applicable_flow > 0 && tcA == 0) || (applicable_flow < 0 && tcB == 0) ));

    if (applicable_flow == 0) {
        return applicable_flow;
    }

    const double tc_flow = applicable_flow;
    const double energy_flow = (applicable_flow > 0 ? b_cellA->heat_energy / tcA * tc_flow : b_cellB->heat_energy / tcB * tc_flow);
    assert(!isnan(energy_flow));

    f_cellA->heat_energy -= energy_flow;
    f_cellB->heat_energy += energy_flow;

    const double fog_flow = (applicable_flow > 0 ? b_cellA->fog / tcA * tc_flow : b_cellB->fog / tcB * tc_flow);
    assert(!isnan(fog_flow));

    f_cellA->fog -= fog_flow;
    f_cellB->fog += fog_flow;

    return applicable_flow;
}

inline void AutomatonThread::temperature_flow(
    const CellMetadata *m_cellA, const Cell *b_cellA, Cell *f_cellA,
    const CellMetadata *m_cellB, const Cell *b_cellB, Cell *f_cellB,
    CoordInt direction)
{
    const double tcA = (m_cellA->blocked
                        ? m_cellA->obj->info.temp_coefficient
                        : b_cellA->air_pressure * airtempcoeff_per_pressure);
    const double tcB = (m_cellB->blocked
                        ? m_cellB->obj->info.temp_coefficient
                        : b_cellB->air_pressure * airtempcoeff_per_pressure);

    if (tcA < 1e-17 || tcB < 1e-17) {
        return;
    }

    const double tempA = b_cellA->heat_energy / tcA;
    const double tempB = b_cellB->heat_energy / tcB;

    const double temp_gradient = tempB - tempA;

    const double energy_flow_raw = (temp_gradient > 0
                                    ? tcB * temp_gradient
                                    : tcA * temp_gradient);
    const double energy_flow = clamp(
        energy_flow_raw * _sim.heat_flow_friction,
        -b_cellA->heat_energy / 4.,
        b_cellB->heat_energy / 4.
    );

    f_cellA->heat_energy += energy_flow;
    f_cellB->heat_energy -= energy_flow;
    assert(abs(energy_flow) < 100);

    if ((energy_flow > 0 && tempB < tempA) || (energy_flow <= 0 && tempA < tempB)) {
        const double total = b_cellA->heat_energy + b_cellB->heat_energy;
        const double avg_temp = total / (tcA + tcB);

        f_cellA->heat_energy = avg_temp * tcA;
        f_cellB->heat_energy = avg_temp * tcB;
    }
}

inline void AutomatonThread::fog_flow(
    const Cell *b_cellA, Cell *f_cellA,
    const Cell *b_cellB, Cell *f_cellB)
{
    const double dfog = b_cellA->fog - b_cellB->fog;
    const double flow = dfog * _sim.fog_flow_friction;
    double applicable_flow = clamp(
        flow,
        -b_cellB->fog / 4.,
        b_cellA->fog / 4.
    );

    f_cellA->fog -= applicable_flow;
    f_cellB->fog += applicable_flow;
}

inline void AutomatonThread::update_cell(CoordInt x, CoordInt y, bool activate)
{
    Cell *b_self, *f_self;
    CellMetadata *m_self;
    Cell *b_neighbours[2], *f_neighbours[2];
    CellMetadata *m_neighbours[2];
    get_cell_and_neighbours(_metadata, &m_self, &m_neighbours, x, y);
    get_cell_and_neighbours(_backbuffer, &b_self, &b_neighbours, x, y);
    get_cell_and_neighbours(_cells, &f_self, &f_neighbours, x, y);

    if (activate) {
        activate_cell(f_self, b_self);
    }
    for (CoordInt i = 0; i < 2; i++) {
        if (b_neighbours[i]) {
            if (!m_self->blocked && !m_neighbours[i]->blocked)
            {
                flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i);
                fog_flow(b_self, f_self, b_neighbours[i], f_neighbours[i]);
            }
            temperature_flow(
                m_self, b_self, f_self,
                m_neighbours[i], b_neighbours[i], f_neighbours[i],
                i
            );
        }
    }
}

void AutomatonThread::update()
{
    Cell *_tmp = _backbuffer;
    _backbuffer = _cells;
    _cells = _tmp;

    {
        Cell *const f_start = &_cells[_slice_y1*_width];
        Cell *const b_start = &_backbuffer[_slice_y1*_width];
        Cell *const f_end = &_cells[(_slice_y1+1)*_width];
        Cell *back = b_start;
        for (Cell *front = f_start; front != f_end; front++) {
            activate_cell(front, back);
            back++;
        }
        if (_bottom_shared_forward)
            _bottom_shared_forward->post();
    }

    if (_top_shared_ready)
        _top_shared_ready->wait();

    if (_top_shared_zone)
        _top_shared_zone->lock();
    for (CoordInt x = 0; x < _width; x++) {
        update_cell(x, _slice_y0);
    }
    if (_top_shared_zone)
        _top_shared_zone->unlock();

    for (CoordInt y = _slice_y0+1; y < _slice_y1; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            update_cell(x, y);
        }
    }

    if (_bottom_shared_zone)
        _bottom_shared_zone->lock();
    for (CoordInt x = 0; x < _width; x++) {
        update_cell(x, _slice_y1, false);
    }
    if (_bottom_shared_zone)
        _bottom_shared_zone->unlock();

    _finished_signal.post();
}

void *AutomatonThread::execute()
{
    while (true) {
        _resume_signal.wait();
        if (_terminated) {
            return 0;
        }
        update();
    }
    return 0;
}
