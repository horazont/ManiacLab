/**********************************************************************
File name: Physics.hpp
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
#ifndef _ML_PHYSICS_H
#define _ML_PHYSICS_H

#include <vector>

#include <CEngine/Misc/Int.hpp>
#include <CEngine/IO/Thread.hpp>

#include "Types.hpp"
#include "PhysicsConfig.hpp"
#include "Stamp.hpp"

class GameObject;

struct Cell {
    double air_pressure;
    double heat_energy;

    // flow is in relation to upper left neighbour!
    double flow[2];
    double fog;
};

struct CellMetadata {
    bool blocked;
    const GameObject *obj;
};

typedef Cell PhysicsCellStamp[cell_stamp_length];

struct CellInfo {
    CoordPair offs;
    Cell phys;
    CellMetadata meta;
};

class AutomatonThread;

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
class Automaton {
public:
    Automaton(CoordInt width, CoordInt height,
        const SimulationConfig &config,
        bool mp = true,
        double initial_pressure = 1.0,
        double initial_temperature = 1.0);
    ~Automaton();

private:
    bool _resumed;
    const CoordInt _width, _height;
    CellMetadata *_metadata;
    Cell *_cells, *_backbuffer;
    const SimulationConfig _config;
    unsigned int _thread_count;
    PyEngine::Semaphore _finished_signal;
    std::vector<PyEngine::Semaphore> _resume_signals;
    std::vector<PyEngine::Semaphore> _forward_signals;
    std::vector<std::mutex> _shared_zones;
    std::vector<std::unique_ptr<AutomatonThread>> _threads;

    uint32_t *_rgba_buffer; //! Used by to_gl_texture() and allocated on-demand.
private:
    void init_cell(
        Cell *buffer,
        CoordInt x, CoordInt y,
        double initial_pressure,
        double initial_temperature);

    void init_metadata(CellMetadata *buffer, CoordInt x, CoordInt y);

    /**
     * Initialize all threads for the automaton. Uses
     * PyEngine::Thread::get_hardware_thread_count() internally to find a
     * reasonable number of threads.
     *
     * This will never spawn more than 64 threads, to avoid excessive
     * synchronization overhead and ensure working levels with down to
     * 128 cells on the y axis.
     */
    void init_threads();
public:
    void apply_temperature_stamp(const CoordInt x, const CoordInt y,
        const Stamp &stamp, const double temperature);

    Cell inline *cell_at(CoordInt x, CoordInt y)
    {
        return &_cells[x+_width*y];
    };

    Cell inline *safe_cell_at(CoordInt x, CoordInt y)
    {
        return (x >= 0 && x < _width && y >= 0 && y < _height) ? cell_at(x, y) : 0;
    };

    void clear_cells(
        const CoordInt x,
        const CoordInt y,
        const Stamp &stamp);

    void get_cell_stamp_at(const CoordInt left, const CoordInt top, PhysicsCellStamp *stamp);
    CellMetadata inline *meta_at(CoordInt x, CoordInt y) { return &_metadata[x+_width*y]; };

    void move_stamp(
        const CoordInt oldx, const CoordInt oldy,
        const CoordInt newx, const CoordInt newy,
        const Stamp &stamp,
        const CoordPair *const vel = nullptr);

    void place_object(const CoordInt x, const CoordInt y,
        const GameObject *obj, const double initial_temperature);

    void place_stamp(const CoordInt atx, const CoordInt aty,
        const CellInfo *cells, const uintptr_t cells_len,
        const CoordPair *const vel = nullptr);

    /**
     * Tell the automaton to resume it's work. The effect of this
     * function if it's called while the automaton is still working is
     * undefined. Make sure it's stopped by calling wait_for() first.
     */
    void resume();
    void set_blocked(CoordInt x, CoordInt y, bool blocked);

    /**
     * Wait until the cellular automaton has settled its calculation
     * and return. The automaton will not continue calculating until
     * resume() is called.
     *
     * If the automaton is already suspended, return immediately.
     */
    void wait_for();
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

    friend class AutomatonThread;
};

class AutomatonThread
{
public:
    AutomatonThread(
        Automaton &data_class,
        CoordInt slice_y0,
        CoordInt slice_y1,
        PyEngine::Semaphore &finished_signal,
        PyEngine::Semaphore *top_shared_ready,
        PyEngine::Semaphore *bottom_shared_forward,
        std::mutex *top_shared_zone,
        std::mutex *bottom_shared_zone,
        PyEngine::Semaphore &resume_signal);
    ~AutomatonThread();

private:
    PyEngine::Semaphore &_finished_signal;
    PyEngine::Semaphore *_top_shared_ready;
    PyEngine::Semaphore *_bottom_shared_forward;
    std::mutex *_top_shared_zone;
    std::mutex *_bottom_shared_zone;
    PyEngine::Semaphore &_resume_signal;
    Automaton &_dataclass;
    const CoordInt _width, _height, _slice_y0, _slice_y1;
    const SimulationConfig _sim;
    Cell *_backbuffer, *_cells;
    CellMetadata *_metadata;
    std::atomic_bool _terminated;
    std::thread _thread;

protected:
    void activate_cell(Cell *front, Cell *back);

    template<class CType>
    void get_cell_and_neighbours(
        CType *buffer,
        CType **cell,
        CType *(*neighbours)[2],
        CoordInt x,
        CoordInt y);

    double flow(
        const Cell *b_cellA,
        Cell *f_cellA,
        const Cell *b_cellB,
        Cell *f_cellB,
        CoordInt direction);

    void temperature_flow(
        const CellMetadata *m_cellA,
        const Cell *b_cellA,
        Cell *f_CellA,
        const CellMetadata *m_cellB,
        const Cell *b_cellB,
        Cell *f_cellB,
        CoordInt direction);

    void fog_flow(
        const Cell *b_cellA,
        Cell *f_cellA,
        const Cell *b_cellB,
        Cell *f_cellB);

    void update_cell(
        CoordInt x,
        CoordInt y,
        bool activate = true);

    void update();

public:
    virtual void *execute();

};

#endif
