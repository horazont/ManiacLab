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

#include <CEngine/Misc/Int.hpp>
#include <CEngine/IO/Thread.hpp>

#include "Types.hpp"
#include "PhysicsConfig.hpp"
#include "Stamp.hpp"

class GameObject;

struct Cell {
    double airPressure;
    double heatEnergy;

    // flow is in relation to upper left neighbour!
    double flow[2];
    double fogDensity;
};

struct CellMetadata {
    bool blocked;
    const GameObject *obj;
};

typedef Cell CellStamp[cellStampLength];

struct CellInfo {
    CoordPair offs;
    Cell phys;
    CellMetadata meta;
};

class AutomatonThread;

/** \rst
Create a cellular automaton which simulates air flow between a given grid of
cells more or less correctly. There are some free parameters you may change
to adapt the simulation to your needs.

*flowFriction* is used to damp the initial creation of flow between two cells.
The “wanted” flow for a pressure gradient is given by:

    newFlow = (cellA->pressure - cellB->pressure) * flowFriction

Then the old flow value (i.e. momentum of the air) is taken into account using:

    flow = oldFlow * flowDamping + newFlow * (1.0 - flowDamping)

making the flow work like a moving average.

In the nature of moving averages lies the fact that the factor used for scaling
the movement speed is highly dependend on the update rate of your simulation.
This implies that you will need very different factors depending on how often
per second you update the automaton.

*initialPressure* and *initialTemperature* just do what they sound like, they're
used as initial values for the cells in the automaton.
\endrst */
class Automaton {
public:
    Automaton(CoordInt width, CoordInt height,
        const SimulationConfig &config,
        bool mp = true,
        double initialPressure = 1.0,
        double initialTemperature = 1.0);
    ~Automaton();

private:
    bool _resumed;
    const CoordInt _width, _height;
    CellMetadata *_metadata;
    Cell *_cells, *_backbuffer;
    const SimulationConfig _config;
    unsigned int _threadCount;
    AutomatonThread **_threads;
    PyEngine::Semaphore **_finishedSignals, **_forwardSignals;
    PyEngine::Mutex **_sharedZones;

    uint32_t *_rgbaBuffer; //! Used by toGLTexture() and allocated on-demand.
private:
    void initCell(Cell *buffer, CoordInt x, CoordInt y,
        double initialPressure, double initialTemperature);
    void initMetadata(CellMetadata *buffer, CoordInt x, CoordInt y);

    /**
     * Initialize all threads for the automaton. Uses
     * PyEngine::Thread::getHardwareThreadCount() internally to find a
     * reasonable number of threads.
     *
     * This will never spawn more than 64 threads, to avoid excessive
     * synchronization overhead and ensure working levels with down to 128
     * cells on the y axis.
     */
    void initThreads();
public:
    void applyTemperatureStamp(const CoordInt x, const CoordInt y,
        const Stamp &stamp, const double temperature);

    Cell inline *cellAt(CoordInt x, CoordInt y)
    {
        return &_cells[x+_width*y];
    };

    Cell inline *safeCellAt(CoordInt x, CoordInt y)
    {
        return (x >= 0 && x < _width && y >= 0 && y < _height) ? cellAt(x, y) : 0;
    };

    void clearCells(const CoordInt x, const CoordInt y,
        const Stamp *stamp);

    void getCellStampAt(const CoordInt left, const CoordInt top, CellStamp *stamp);
    CellMetadata inline *metaAt(CoordInt x, CoordInt y) { return &_metadata[x+_width*y]; };

    void moveStamp(
        const CoordInt oldx, const CoordInt oldy,
        const CoordInt newx, const CoordInt newy,
        const Stamp *stamp, const CoordPair *const vel = nullptr);

    void placeObject(const CoordInt x, const CoordInt y,
        const GameObject *obj, const double initialTemperature);

    void placeStamp(const CoordInt atx, const CoordInt aty,
        const CellInfo *cells, const uintptr_t cellsLen,
        const CoordPair *const vel = nullptr);

    /**
     * Tell the automaton to resume it's work. The effect of this function if
     * it's called while the automaton is still working is undefined. Make sure
     * it's stopped by calling waitFor() first.
     */
    void resume();
    void setBlocked(CoordInt x, CoordInt y, bool blocked);

    /**
     * Wait until the cellular automaton has settled its calculation and return.
     * The automaton will not continue calculating until resume() is called.
     *
     * If the automaton is already suspended, return immediately.
     */
    void waitFor();
public:
    void printCells(const double min, const double max,
        const char **map, const int mapLen);
    void printCellsBlock(const double min, const double max);
    void printCells256(const double min, const double max);
    void printFlow();

public:
    /**
     * Map pressure values in the range from min to max to [0..1] and write them
     * to the currently bound OpenGL texture as GL_RGBA.
     */
    void toGLTexture(const double min, const double max, bool threadRegions);

friend class AutomatonThread;
};

class AutomatonThread: public PyEngine::Thread {
public:
    AutomatonThread(Automaton *dataClass, CoordInt sliceY0,
        CoordInt sliceY1, PyEngine::Semaphore *finishedSignal,
        PyEngine::Semaphore *topSharedReady, PyEngine::Semaphore *bottomSharedForward,
        PyEngine::Mutex *topSharedZone, PyEngine::Mutex *bottomSharedZone);
private:
    PyEngine::Semaphore *_finishedSignal, *_topSharedReady, *_bottomSharedForward;
    PyEngine::Mutex *_topSharedZone, *_bottomSharedZone;
    Automaton *_dataClass;
    const CoordInt _width, _height, _sliceY0, _sliceY1;
    const SimulationConfig _sim;
    Cell *_backbuffer, *_cells;
    CellMetadata *_metadata;
protected:
    void activateCell(Cell *front, Cell *back);
    template<class CType>
    void getCellAndNeighbours(CType *buffer, CType **cell,
        CType *(*neighbours)[2], CoordInt x, CoordInt y);
    double flow(const Cell *b_cellA, Cell *f_cellA, const Cell *b_cellB,
        Cell *f_cellB, CoordInt direction);
    void temperatureFlow(const CellMetadata *m_cellA, const Cell *b_cellA, Cell *f_CellA,
                         const CellMetadata *m_cellB, const Cell *b_cellB, Cell *f_cellB,
                         CoordInt direction);

    void updateCell(CoordInt x, CoordInt y, bool activate = true);
    void update();
public:
    virtual void *execute();
};

#endif
