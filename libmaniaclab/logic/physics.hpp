#ifndef _ML_PHYSICS_H
#define _ML_PHYSICS_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#include <ffengine/math/vector.hpp>

#include "logic/types.hpp"
#include "logic/physicsconfig.hpp"
#include "logic/stamp.hpp"

class GameObject;

class ILabSim {
public:
    static const SimFloat air_diffusion_factor;
    static const SimFloat air_flow_factor;
    static const SimFloat convection_factor;
    static const SimFloat heat_diffusion_factor;
    static const SimFloat fog_diffusion_factor;

public:
    virtual ~ILabSim();

public:
    virtual void start_frame() = 0;

    virtual void wait_for_frame() = 0;
};


struct LabCellMeta {
    LabCellMeta();

    bool blocked;
    GameObject *obj;
};


struct LabCell {
    LabCell();

    SimFloat air_pressure;
    SimFloat heat_energy;

    Vector<SimFloat, 2> flow;
    SimFloat fog_density;
};


typedef LabCell PhysicsCellStamp[cell_stamp_length];


struct SimulationConfig {
    SimFloat initial_air_pressure;
    SimFloat initial_temperature;
    SimFloat initial_fog_density;
};


struct CellInfo {
    CoordPair offs;
    LabCell phys;
    LabCellMeta meta;
};

class NativeSimWorker;

/**
 * Create a cellular automaton which simulates air flow between a given grid of
 * cells more or less correctly. There are some free parameters you may change
 * to adapt the simulation to your needs.
 *
 * *flow_friction* is used to damp the initial creation of flow between two
 * cells. The “wanted” flow for a pressure gradient is given by:
 *
 *     new_flow = (cellA->pressure - cellB->pressure) * flow_friction
 *
 * Then the old flow value (i.e. momentum of the air) is taken into account
 * using:
 *
 *     flow = old_flow * flow_damping + new_flow * (1.0 - flow_damping)
 *
 * making the flow work like a moving average.
 *
 * In the nature of moving averages lies the fact that the factor used for
 * scaling the movement speed is highly dependend on the update rate of your
 * simulation. This implies that you will need very different factors depending
 * on how often per second you update the automaton.
 *
 * *initial_pressure* and *initial_temperature* just do what they sound like,
 * they're used as initial values for the cells in the automaton.
 */
class NativeLabSim: public ILabSim {
private:
    static const std::size_t ROWS_PER_BLOCK = 10;

public:
    NativeLabSim(
            CoordInt width, CoordInt height,
            const SimulationConfig &config);
    ~NativeLabSim();

private:
    const CoordInt m_width, m_height;
    const std::size_t m_block_count;
    const unsigned int m_worker_count;

    LabCell m_null_cell;
    LabCellMeta m_null_cell_meta;

    std::vector<LabCellMeta> m_meta_cells;
    std::vector<LabCell> m_front_cells;
    std::vector<LabCell> m_back_cells;

    const SimulationConfig m_config;

    bool m_running; //! true while the simulation is running

    /* guarded by m_control_mutex */
    std::mutex m_control_mutex;
    std::condition_variable m_control_wakeup;
    bool m_run; //! true while the simulation is STARTING

    /* guarded by m_done_mutex */
    std::mutex m_done_mutex;
    std::condition_variable m_done_wakeup;
    bool m_done; //! true while the simulation is STOPPING

    /* guarded by m_worker_task_mutex */
    std::mutex m_worker_task_mutex;
    std::condition_variable m_worker_wakeup;
    unsigned int m_worker_to_start;
    bool m_worker_terminate;

    /* guarded by m_worker_done_mutex */
    std::mutex m_worker_done_mutex;
    std::condition_variable m_worker_done_wakeup;
    unsigned int m_worker_stopped;

    /* atomic */
    std::atomic<unsigned int> m_worker_block_ctr;
    std::atomic_bool m_terminated;

    std::thread m_coordinator_thread;

    /* owned by m_coordinator_thread */
    std::vector<std::thread> m_worker_threads;

    /* not guarded by mutex */
    std::vector<uint32_t> m_rgba_buffer; //! Used by to_gl_texture() and allocated on-demand.
    std::vector<Vector4f> m_data_buffer;

private:
    void *coordinator_impl();
    void coordinator_run_workers();

    inline void update_cell(const CoordInt x, const CoordInt y);
    void update_active_block(const CoordInt y0, const CoordInt y1);

    void *worker_impl();

private:
    void init_cell(
            std::vector<LabCell> &buffer,
            CoordInt x, CoordInt y,
            SimFloat air_pressure,
            SimFloat temperature,
            SimFloat fog_density);

    void init_metadata(std::vector<LabCellMeta> &buffer, CoordInt x, CoordInt y);

    inline LabCell &writable_front_cell_at(CoordInt x, CoordInt y) {
        return m_front_cells[x+m_width*y];
    }

public:
    void apply_temperature_stamp(
        const CoordInt x, const CoordInt y,
        const Stamp &stamp, const double temperature);

    void apply_fog_effect_stamp(const CoordInt x, const CoordInt y,
        const Stamp &stamp, const SimFloat intensity);

    void apply_flow_stamp(
            const CoordInt x, const CoordInt y,
            const Stamp &stamp, const Vector2f flow,
            const SimFloat blend = 1.0);

    void apply_pressure_stamp(
            const CoordInt x, const CoordInt y,
            const Stamp &stamp,
            const float new_pressure);

    void reset_unblocked_cells(
            const SimFloat pressure = default_pressure,
            const SimFloat temperature = default_temperature,
            const SimFloat fog_density = 0.f);

    inline CoordInt width() const
    {
        return m_width;
    }

    inline CoordInt height() const
    {
        return m_height;
    }

    inline LabCell &writable_cell_at(CoordInt x, CoordInt y)
    {
        return m_back_cells[x+m_width*y];
    }

    inline const LabCell &front_cell_at(CoordInt x, CoordInt y) const
    {
        return m_front_cells[x+m_width*y];
    }

    inline LabCell *safe_writable_cell_at(CoordInt x, CoordInt y)
    {
        return (x >= 0 && x < m_width && y >= 0 && y < m_height) ? &writable_cell_at(x, y) : nullptr;
    }

    inline const LabCell *safe_front_cell_at(CoordInt x, CoordInt y) const
    {
        return (x >= 0 && x < m_width && y >= 0 && y < m_height) ? &front_cell_at(x, y) : nullptr;
    }

    void clear_cells(
        const CoordInt x,
        const CoordInt y,
        const Stamp &stamp);

    inline const SimulationConfig &config() const
    {
        return m_config;
    }

    inline LabCellMeta &meta_at(CoordInt x, CoordInt y)
    {
        return m_meta_cells[x+m_width*y];
    }

    inline const LabCellMeta &meta_at(CoordInt x, CoordInt y) const
    {
        return m_meta_cells[x+m_width*y];
    }

    void move_stamp(
        const CoordInt oldx, const CoordInt oldy,
        const CoordInt newx, const CoordInt newy,
        const Stamp &stamp,
        const CoordPair *const vel = nullptr);

    void place_object(
        const CoordInt x, const CoordInt y,
        GameObject *obj,
        const double initial_temperature);

    void place_stamp(
        const CoordInt atx, const CoordInt aty,
        const CellInfo *cells,
        const uintptr_t cells_len,
        const CoordPair *const vel = nullptr);

    SimFloat measure_stamp_avg(const CoordInt atx, const CoordInt aty,
            const CoordPair *coords,
            const uintptr_t coords_len,
            const std::function<SimFloat(const LabCell &)> &sensor
            , bool exclude_blocked) const;

    Vector<SimFloat, 2> measure_stamp_gradient(
            const CoordInt atx, const CoordInt aty,
            const CoordPair *coords,
            const uintptr_t coords_len,
            const std::function<SimFloat(const LabCell &)> &sensor,
            bool exclude_blocked) const;

    /**
     * Tell the automaton to resume it's work. The effect of this
     * function if it's called while the automaton is still working is
     * undefined. Make sure it's stopped by calling wait_for() first.
     */
    void start_frame() override;
    void set_blocked(CoordInt x, CoordInt y, bool blocked);

    /**
     * Wait until the cellular automaton has settled its calculation
     * and return. The automaton will not continue calculating until
     * resume() is called.
     *
     * If the automaton is already suspended, return immediately.
     */
    void wait_for_frame() override;
public:
    /**
     * Convert the data in the physics cells to a human-interpretable
     * image representation and store it in the currently bound
     * opengl texture.
     *
     * @param min pressure which will be mapped to 0
     * @param max pressure which will be mapped to 1
     * @param thread_regions if true, thread regions are also visualized
     */
    void to_gl_texture(const double min, const double max, bool thread_regions);

    void data_to_gl_texture();

    friend class NativeSimWorker;
};


#endif
