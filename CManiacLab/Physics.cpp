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

#include <CEngine/IO/Thread.hpp>

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

Automaton::Automaton(CoordInt width, CoordInt height,
        const SimulationConfig &config,
        bool mp,
        double initialPressure, double initialTemperature):
    _resumed(false),
    _width(width),
    _height(height),
    _metadata(new CellMetadata[width*height]()),
    _cells(new Cell[width*height]()),
    _backbuffer(new Cell[width*height]()),
    _config(config),
    _threadCount(mp?(Thread::getHardwareThreadCount()):1),
    _threads(new AutomatonThread*[_threadCount]()),
    _finishedSignals(new Semaphore*[_threadCount]()),
    _forwardSignals(new Semaphore*[_threadCount-1]()),
    _sharedZones(new Mutex*[_threadCount-1]()),
    _rgbaBuffer(0)
{
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            initMetadata(_metadata, x, y);
            initCell(_cells, x, y, initialPressure, initialTemperature);
            initCell(_backbuffer, x, y, initialPressure, initialTemperature);
        }
    }
    initThreads();
}

Automaton::~Automaton()
{
    waitFor();
    for (unsigned int i = 0; i < _threadCount - 1; i++) {
        delete _forwardSignals[i];
        delete _finishedSignals[i];
        delete _sharedZones[i];
    }
    delete _finishedSignals[_threadCount-1];
    delete[] _sharedZones;
    delete[] _forwardSignals;
    delete[] _finishedSignals;
    delete[] _threads;
    delete[] _cells;
    delete[] _backbuffer;
    delete[] _metadata;
    if (_rgbaBuffer) {
        free(_rgbaBuffer);
    }
}

void Automaton::initMetadata(CellMetadata *buffer, CoordInt x,
    CoordInt y)
{
    CellMetadata *cell = &buffer[x+_width*y];
    cell->blocked = false;
    cell->obj = 0;
}

void Automaton::initCell(Cell *buffer, CoordInt x, CoordInt y,
    double initialPressure, double initialTemperature)
{
    Cell *cell = &buffer[x+_width*y];
    cell->airPressure = initialPressure;
    cell->heatEnergy = initialTemperature * (airTempCoeffPerPressure * cell->airPressure);
    cell->flow[0] = 0;
    cell->flow[1] = 0;
    cell->fog = 0;
}

void Automaton::initThreads()
{
    for (unsigned int i = 0; i < _threadCount - 1; i++) {
        _finishedSignals[i] = new Semaphore();
        _sharedZones[i] = new Mutex();
        _forwardSignals[i] = new Semaphore();
    }
    _finishedSignals[_threadCount - 1] = new Semaphore();

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
    const CoordInt sliceSize = _height / (_threadCount <= 64 ? _threadCount : 64);
    assert(sliceSize > 0);

    CoordInt sliceY0 = 0;

    for (unsigned int i = 0; i < _threadCount - 1; i++) {
        _threads[i] = new AutomatonThread(this,
            sliceY0, sliceY0 + sliceSize-1,  // range on which this thread works
            _finishedSignals[i],
            (i>0) ? _forwardSignals[i-1] : 0,
            _forwardSignals[i],
            (i>0) ? _sharedZones[i-1] : 0,
            _sharedZones[i]
        );
        sliceY0 += sliceSize;
    }
    _threads[_threadCount-1] = new AutomatonThread(this,
        sliceY0, _height-1,
        _finishedSignals[_threadCount-1],
        (_threadCount > 1) ? _forwardSignals[_threadCount-2] : 0,
        0,
        (_threadCount > 1) ? _sharedZones[_threadCount-2] : 0,
        0
    );
}

void Automaton::clearCells(const CoordInt dx, const CoordInt dy,
    const Stamp *stamp)
{
    assert(!_resumed);

    uintptr_t stampCellsLen = 0;
    const CoordPair *stampCells = stamp->getMapCoords(&stampCellsLen);

    for (uintptr_t i = 0; i < stampCellsLen; i++) {
        const CoordPair p = stampCells[i];
        const CoordInt x = p.x + dx;
        const CoordInt y = p.y + dy;

        Cell *const currCell = safeCellAt(x, y);
        if (!currCell) {
            continue;
        }
        CellMetadata *const currMeta = metaAt(x, y);
        initCell(_cells, x, y, 0, 0);
        initCell(_backbuffer, x, y, 0, 0);
        currMeta->blocked = false;
    }
}

void Automaton::applyTemperatureStamp(const CoordInt x, const CoordInt y,
    const Stamp &stamp, const double temperature)
{
    uintptr_t stampCellsLen = 0;
    const CoordPair *cellCoord = stamp.getMapCoords(&stampCellsLen);
    cellCoord--;

    for (uintptr_t i = 0; i < stampCellsLen; i++) {
        cellCoord++;

        const CoordInt cx = x + cellCoord->x;
        const CoordInt cy = y + cellCoord->y;

        Cell *cell = safeCellAt(cx, cy);
        if (!cell) {
            continue;
        }

        CellMetadata *meta = metaAt(cx, cy);
        if (meta->blocked) {
            cell->heatEnergy = temperature * meta->obj->tempCoefficient;
        } else {
            cell->heatEnergy = temperature * (airTempCoeffPerPressure * cell->airPressure);
        }
    }
}

void Automaton::getCellStampAt(const CoordInt left, const CoordInt top,
    CellStamp *stamp)
{
    Cell **currCell = (Cell**)stamp;
    for (CoordInt y = 0; y < subdivisionCount; y++) {
        Cell *srcCell = &_cells[top + y];
        for (CoordInt x = 0; x < subdivisionCount; x++) {
            *currCell = srcCell;
            currCell++;
            srcCell++;
        }
    }
}

void Automaton::moveStamp(const CoordInt oldx, const CoordInt oldy,
    const CoordInt newx, const CoordInt newy,
    const Stamp *stamp, const CoordPair *const vel)
{
    assert(!_resumed);

    static CellInfo cells[cellStampLength];

    uintptr_t writeIndex = 0;

    uintptr_t stampCellsLen = 0;
    const CoordPair *stampCells = stamp->getMapCoords(&stampCellsLen);
    stampCells--;

    for (uintptr_t i = 0; i < stampCellsLen; i++) {
        assert(writeIndex < cellStampLength);
        stampCells++;

        const CoordInt x = oldx + stampCells->x;
        const CoordInt y = oldy + stampCells->y;

        Cell *cell = safeCellAt(x, y);
        if (!cell) {
            continue;
        }
        CellMetadata *meta = metaAt(x, y);

        CellInfo *dst = &cells[writeIndex];
        memcpy(&dst->offs, stampCells, sizeof(CoordPair));
        memcpy(&dst->phys, cell, sizeof(Cell));
        memcpy(&dst->meta, meta, sizeof(CellMetadata));
        writeIndex++;
        initCell(_cells, x, y, 0, 0);
        initCell(_backbuffer, x, y, 0, 0);
        meta->blocked = false;
        meta->obj = 0;
    }

    placeStamp(newx, newy, cells, writeIndex, vel);
}

void Automaton::placeObject(const CoordInt dx, const CoordInt dy,
    const GameObject *obj, const double initialTemperature)
{
    assert(!_resumed);

    static CellInfo cells[cellStampLength];

    uintptr_t stampCellsLen = 0;
    const CoordPair *stampCells = obj->stamp->getMapCoords(&stampCellsLen);

    double heatEnergy = initialTemperature * obj->tempCoefficient;

    for (uintptr_t i = 0; i < stampCellsLen; i++) {
        CellInfo *const dst = &cells[i];
        dst->offs = stampCells[i];
        memset(&(dst->phys), 0, sizeof(Cell));
        dst->phys.heatEnergy = heatEnergy;
        dst->meta.blocked = true;
        dst->meta.obj = obj;
    }

    placeStamp(dx, dy, cells, stampCellsLen);
}

void Automaton::placeStamp(const CoordInt atx, const CoordInt aty,
    const CellInfo *cells, const uintptr_t cellsLen,
    const CoordPair *const vel)
{
    assert(!_resumed);

    const intptr_t indexRowLength = subdivisionCount+2;
    const intptr_t indexLength = indexRowLength * indexRowLength;
    static intptr_t *borderIndicies = (intptr_t*)malloc(indexLength * sizeof(intptr_t));
    static Cell **borderCells = (Cell**)malloc(indexLength * sizeof(Cell*));
    static double *borderCellWeights = (double*)malloc(indexLength * sizeof(double));

    // to iterate over neighbouring cells
    static const intptr_t offs[4][2] = {
        {0, -1}, {-1, 0}, {1, 0}, {0, 1}
    };

    memset(borderIndicies, -1, indexLength * sizeof(intptr_t));
    memset(borderCells, 0, indexLength * sizeof(Cell*));

    // collect surplus matter here
    double airToDistribute = 0.;
    double heatToDistribute = 0.;
    double fogToDistribute = 0.;
    intptr_t borderCellWriteIndex = 0;
    intptr_t borderCellCount = 0;
    double borderCellWeight = 0;

    const double vel_norm = (vel != nullptr ? (vel->norm() != 0 ? vel->norm() : 0) : 0);
    const double vel_x = (vel_norm > 0 ? vel->x / vel_norm : 0);
    const double vel_y = (vel_norm > 0 ? vel->y / vel_norm : 0);

    for (uintptr_t i = 0; i < cellsLen; i++) {
        const CoordPair p = cells[i].offs;
        const CoordInt x = p.x + atx;
        const CoordInt y = p.y + aty;

        Cell *const currCell = safeCellAt(x, y);
        if (!currCell) {
            continue;
        }
        CellMetadata *const currMeta = metaAt(x, y);
        assert(!currMeta->blocked);

        if (!currMeta->blocked) {
            airToDistribute += currCell->airPressure;
            heatToDistribute += currCell->heatEnergy;
            fogToDistribute += currCell->fog;
        }
        memcpy(currCell, &cells[i].phys, sizeof(Cell));
        memcpy(currMeta, &cells[i].meta, sizeof(CellMetadata));

        for (uintptr_t j = 0; j < 4; j++) {
            const intptr_t indexCell = (p.y + offs[j][1] + 1) * indexRowLength + p.x + 1 + offs[j][0];

            if (borderIndicies[indexCell] != -1) {
                // inspected earlier, no need to check again
                continue;
            }

            const intptr_t nx = x + offs[j][0];
            const intptr_t ny = y + offs[j][1];

            Cell *const neighCell = safeCellAt(nx, ny);
            if (!neighCell) {
                borderIndicies[indexCell] = -2;
                continue;
            }
            CellMetadata *const neighMeta = metaAt(nx, ny);
            if (neighMeta->blocked) {
                borderIndicies[indexCell] = -2;
                continue;
            }

            //~ const CoordPair curr_offs = CoordPair(p.x - halfOffset, p.y - halfOffset);
//~
            //~ double cellWeight = (vel_norm > 0 ? curr_offs.normed_float_dotp(vel_x, vel_y) : 1);
            //~ cellWeight = (cellWeight > 0 ? cellWeight : 0);

            const double cellWeight = max((vel_norm > 0 ? offs[j][0] * vel_x + offs[j][1] * vel_y : 1), 0);

            borderIndicies[indexCell] = borderCellWriteIndex;
            borderCells[borderCellWriteIndex] = neighCell;
            borderCellWeights[borderCellWriteIndex] = cellWeight;
            assert(borderCellWriteIndex < indexLength);
            borderCellWriteIndex++;
            borderCellCount++;
            borderCellWeight += cellWeight;
        }

        const uintptr_t indexCell = (p.y + 1) * indexRowLength + (p.x + 1);
        if (borderIndicies[indexCell] >= 0) {
            borderCellCount--;
            borderCells[borderIndicies[indexCell]] = 0;
            borderCellWeight -= borderCellWeights[borderIndicies[indexCell]];

        }
        borderIndicies[indexCell] = -2;

        assert(!isnan(currCell->heatEnergy));
    }

    if (airToDistribute == 0)
        return;

    if (borderCellCount == 0) {
        fprintf(stderr, "[PHY!] [NC] no cells to move stuff to\n");
        return;
    }

    const double weightToUse = (borderCellWeight > 0 ? borderCellWeight : borderCellCount);

    const double airPerCell = airToDistribute / weightToUse;
    const double heatPerCell = heatToDistribute / weightToUse;
    const double fogPerCell = fogToDistribute / weightToUse;

    unsigned int j = 0;
    double *neighCellWeight = &borderCellWeights[0];
    for (Cell **neighCell = &borderCells[0]; j < borderCellCount; neighCell++) {
        if (!(*neighCell)) {
            neighCellWeight++;
            continue;
        }
        const double cellWeight = (borderCellWeight > 0 ? *neighCellWeight : 1);
        (*neighCell)->airPressure += airPerCell * cellWeight;
        (*neighCell)->heatEnergy += heatPerCell * cellWeight;
        (*neighCell)->fog += fogPerCell * cellWeight;
        //~ if (vel != nullptr) {
            //~ (*neighCell)->flow[0] += vel->x;
            //~ (*neighCell)->flow[1] += vel->y;
        //~ }

        assert(!isnan((*neighCell)->heatEnergy));
        j++;
        neighCellWeight++;
    }
}

void Automaton::resume()
{
    for (unsigned int i = 0; i < _threadCount; i++) {
        _threads[i]->resume();
    }
    _resumed = true;
}

void Automaton::setBlocked(CoordInt x, CoordInt y, bool blocked)
{
    assert(!_resumed);
    _metadata[x+_width*y].blocked = true;
}

void Automaton::waitFor()
{
    if (!_resumed)
        return;
    for (unsigned int i = 0; i < _threadCount; i++) {
        _finishedSignals[i]->wait();
    }
    _resumed = false;
    Cell *tmp = _backbuffer;
    _backbuffer = _cells;
    _cells = tmp;
}

void Automaton::printCells(const double min, const double max,
    const char **map, const int mapLen)
{
    assert(!_resumed);
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            Cell *cell = cellAt(x, y);
            CellMetadata *meta = metaAt(x, y);
            if (meta->blocked) {
                std::cout << map[mapLen];
            } else {
                const double val = cell->airPressure;
                const double scaled = (val - min) / (max - min);
                int index = (int)(scaled * mapLen);
                if (index < 0)
                    index = 0;
                if (index >= mapLen)
                    index = mapLen-1;

                std::cout << map[index];
            }
        }
        std::cout << "\n";
        std::cout.flush();
    }
}

void Automaton::printCellsBlock(const double min, const double max)
{
    static const char *map[6] = {
        " ", "░", "▒", "▓", "█", "x"
    };
    static const int maxMap = 5;
    printCells(min, max, map, maxMap);
}

void Automaton::printCells256(const double min, const double max)
{
    static const char *map[25] = {
        "\x1b[38;5;232m█",
        "\x1b[38;5;233m█",
        "\x1b[38;5;234m█",
        "\x1b[38;5;235m█",
        "\x1b[38;5;236m█",
        "\x1b[38;5;237m█",
        "\x1b[38;5;238m█",
        "\x1b[38;5;239m█",
        "\x1b[38;5;240m█",
        "\x1b[38;5;241m█",
        "\x1b[38;5;242m█",
        "\x1b[38;5;243m█",
        "\x1b[38;5;244m█",
        "\x1b[38;5;245m█",
        "\x1b[38;5;246m█",
        "\x1b[38;5;247m█",
        "\x1b[38;5;248m█",
        "\x1b[38;5;249m█",
        "\x1b[38;5;250m█",
        "\x1b[38;5;251m█",
        "\x1b[38;5;252m█",
        "\x1b[38;5;253m█",
        "\x1b[38;5;254m█",
        "\x1b[38;5;255m█",
        "\x1b[38;5;255mx"
    };
    static const int maxMap = 24;
    printCells(min, max, map, maxMap);
}

void Automaton::printFlow()
{
    static const char *map[8] = {
        "→", "↘", "↓", "↙", "←", "↖", "↑", "↗"
    };
    static const char *none = "⋅";

    assert(!_resumed);
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            Cell *cell = cellAt(x, y);
            CellMetadata *meta = metaAt(x, y);
            if (meta->blocked) {
                std::cout << "x";
                continue;
            }

            if (sqrt(cell->flow[0]*cell->flow[0] + cell->flow[1]*cell->flow[1]) < 0.0001) {
                std::cout << none;
            } else {
                double angle = atan2(cell->flow[0], cell->flow[1]) + M_PI - M_PI / 4.;
                if (angle >= 2*M_PI) {
                    angle -= 2*M_PI;
                }
                const int index = (int)(4.*angle / M_PI);
                assert(index >= 0 && index < 8);
                std::cout << map[index];
            }
        }
        std::cout << std::endl;
    }
}

void Automaton::toGLTexture(const double min, const double max,
    bool threadRegions)
{
    if (!_rgbaBuffer) {
        _rgbaBuffer = (uint32_t*)malloc(_width*_height*4);
    }

    const CoordInt half = _width / 2;

    uint32_t *target = _rgbaBuffer;
    Cell *source = _backbuffer;
    CellMetadata *metaSource = _metadata;
    for (CoordInt i = 0; i < _width*_height; i++) {
        if (metaSource->blocked) {
            *target = 0x0000FF;
        } else {
            const bool right = (i % _height) >= half;
            const unsigned char pressColor = (unsigned char)(clamp((source->airPressure - min) / (max - min), 0.0, 1.0) * 255.0);
            //~ const double temperature = (metaSource->blocked ? source->heatEnergy / metaSource->obj->tempCoefficient : source->heatEnergy / (source->airPressure * airTempCoeffPerPressure));
            const double fog = (metaSource->blocked ? 0 : source->fog);
            //~ const unsigned char tempColor = (unsigned char)(clamp((temperature - min) / (max - min), 0.0, 1.0) * 255.0);
            const unsigned char fogColor = (unsigned char)(clamp((fog - min) / (max - min), 0.0, 1.0) *255.0);
            const unsigned char b = (right ? fogColor : pressColor);
            const unsigned char r = b;
            if (threadRegions) {
                const unsigned char g = (unsigned char)((double)(int)(((double)(i / _width)) / _height * _threadCount) / _threadCount * 255.0);
                //const unsigned char b = (unsigned char)(clamp((source->flow[1] - min) / (max - min), -1.0, 1.0) * 127.0 + 127.0);
                *target = r | (g << 8) | (r << 16);
            } else {
                *target = r | (r << 8) | (b << 16);
            }
        }
        target++;
        source++;
        metaSource++;
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)_rgbaBuffer);
}

/* AutomatonThread::AutomatonThread */

AutomatonThread::AutomatonThread(Automaton *dataClass, CoordInt sliceY0,
        CoordInt sliceY1, Semaphore *finishedSignal,
        Semaphore *topSharedReady, Semaphore *bottomSharedForward,
        Mutex *topSharedZone, Mutex *bottomSharedZone):
    Thread::Thread(),
    _finishedSignal(finishedSignal),
    _topSharedReady(topSharedReady),
    _bottomSharedForward(bottomSharedForward),
    _topSharedZone(topSharedZone),
    _bottomSharedZone(bottomSharedZone),
    _dataClass(dataClass),
    _width(dataClass->_width),
    _height(dataClass->_height),
    _sliceY0(sliceY0),
    _sliceY1(sliceY1),
    _sim(dataClass->_config),
    _backbuffer(dataClass->_backbuffer),
    _cells(dataClass->_cells),
    _metadata(dataClass->_metadata)
{

}

inline void AutomatonThread::activateCell(Cell *front, Cell *back)
{
    //~ if (back->airPressure <= 1e-100 && back->airPressure != 0) {
        //~ front->airPressure = 0;
        //~ front->flow[0] = 0;
        //~ front->flow[1] = 0;
        //~ (back+_width)->flow[1] = 0;
        //~ (back+1)->flow[0] = 0;
        //~ (front+_width)->flow[1] = 0;
        //~ (front+1)->flow[0] = 0;
        //~ front->heatEnergy = 0;
    //~ } else {
    front->airPressure = back->airPressure;
    for (int i = 0; i < 2; i++) {
        const double flow = back->flow[i];
        if (!isinf(flow) && abs(flow) < 1e10) {
            front->flow[i] = flow;
        } else {
            front->flow[i] = 0;
            back->flow[i] = 0;
        }
    }
    front->heatEnergy = back->heatEnergy;
    front->fog = back->fog;
    //~ }
    assert(!isnan(back->airPressure));
    assert(!isnan(back->fog));
    assert(!isnan(back->heatEnergy));
}

template<class CType>
void AutomatonThread::getCellAndNeighbours(CType *buffer, CType **cell,
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
    const double dPressure = b_cellA->airPressure - b_cellB->airPressure;
    const double dTemp = (direction == 1 ? b_cellA->heatEnergy - b_cellB->heatEnergy : 0);
    const double tempFlow = (dTemp > 0 ? dTemp * _sim.convectionFriction : 0);
    const double pressFlow = dPressure * _sim.flowFriction;
    const double oldFlow = b_cellA->flow[direction];
    const double flow = oldFlow * _sim.flowDamping + (tempFlow + pressFlow) * (1.0 - _sim.flowDamping);
    double applicableFlow = clamp(
        flow,
        -b_cellB->airPressure / 4.,
        b_cellA->airPressure / 4.
    );

    const double tcA = b_cellA->airPressure;
    const double tcB = b_cellB->airPressure;

    f_cellA->flow[direction] = flow;

    f_cellA->airPressure -= applicableFlow;
    f_cellB->airPressure += applicableFlow;

    if ((applicableFlow >= 0 && tcA == 0) || (applicableFlow <= 0 && tcB == 0)) {
        return applicableFlow;
    }

    const double tcFlow = applicableFlow;
    const double energyFlow = (applicableFlow > 0 ? b_cellA->heatEnergy / tcA * tcFlow : b_cellB->heatEnergy / tcB * tcFlow);

    assert(!isnan(energyFlow));

    f_cellA->heatEnergy -= energyFlow;
    f_cellB->heatEnergy += energyFlow;

    const double fogFlow = (applicableFlow > 0 ? b_cellA->fog / tcA * tcFlow : b_cellB->fog / tcB * tcFlow);
    assert(!isnan(fogFlow));

    f_cellA->fog -= fogFlow;
    f_cellB->fog += fogFlow;

    return applicableFlow;
}

inline void AutomatonThread::temperatureFlow(
    const CellMetadata *m_cellA, const Cell *b_cellA, Cell *f_cellA,
    const CellMetadata *m_cellB, const Cell *b_cellB, Cell *f_cellB,
    CoordInt direction)
{
    const double tcA = (m_cellA->blocked ? m_cellA->obj->tempCoefficient : b_cellA->airPressure * airTempCoeffPerPressure);
    const double tcB = (m_cellB->blocked ? m_cellB->obj->tempCoefficient : b_cellB->airPressure * airTempCoeffPerPressure);

    if (tcA < 1e-17 || tcB < 1e-17) {
        return;
    }

    const double tempA = b_cellA->heatEnergy / tcA;
    const double tempB = b_cellB->heatEnergy / tcB;

    const double tempGradient = tempB - tempA;

    const double energyFlowRaw = (tempGradient > 0 ? tcB * tempGradient : tcA * tempGradient);
    const double energyFlow = clamp(
        energyFlowRaw * _sim.heatFlowFriction,
        -b_cellA->heatEnergy / 4.,
        b_cellB->heatEnergy / 4.
    );

    f_cellA->heatEnergy += energyFlow;
    f_cellB->heatEnergy -= energyFlow;
    assert(abs(energyFlow) < 100);

    if ((energyFlow > 0 && tempB < tempA) || (energyFlow <= 0 && tempA < tempB)) {
        const double total = b_cellA->heatEnergy + b_cellB->heatEnergy;
        const double avgTemp = total / (tcA + tcB);

        f_cellA->heatEnergy = avgTemp * tcA;
        f_cellB->heatEnergy = avgTemp * tcB;
    }
}

inline void AutomatonThread::fogFlow(
    const Cell *b_cellA, Cell *f_cellA,
    const Cell *b_cellB, Cell *f_cellB)
{
    const double dFog = b_cellA->fog - b_cellB->fog;
    const double flow = dFog * _sim.fogFlowFriction;
    double applicableFlow = clamp(
        flow,
        -b_cellB->fog / 4.,
        b_cellA->fog / 4.
    );

    f_cellA->fog -= applicableFlow;
    f_cellB->fog += applicableFlow;
}

inline void AutomatonThread::updateCell(CoordInt x, CoordInt y, bool activate)
{
    Cell *b_self, *f_self;
    CellMetadata *m_self;
    Cell *b_neighbours[2], *f_neighbours[2];
    CellMetadata *m_neighbours[2];
    getCellAndNeighbours(_metadata, &m_self, &m_neighbours, x, y);
    getCellAndNeighbours(_backbuffer, &b_self, &b_neighbours, x, y);
    getCellAndNeighbours(_cells, &f_self, &f_neighbours, x, y);

    if (activate) {
        activateCell(f_self, b_self);
    }
    for (CoordInt i = 0; i < 2; i++) {
        if (b_neighbours[i]) {
            if (!m_self->blocked && !m_neighbours[i]->blocked)
            {
                flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i);
                fogFlow(b_self, f_self, b_neighbours[i], f_neighbours[i]);
            }
            temperatureFlow(
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
        Cell *const f_start = &_cells[_sliceY1*_width];
        Cell *const b_start = &_backbuffer[_sliceY1*_width];
        Cell *const f_end = &_cells[(_sliceY1+1)*_width];
        Cell *back = b_start;
        for (Cell *front = f_start; front != f_end; front++) {
            activateCell(front, back);
            back++;
        }
        if (_bottomSharedForward)
            _bottomSharedForward->post();
    }

    if (_topSharedReady)
        _topSharedReady->wait();

    if (_topSharedZone)
        _topSharedZone->lock();
    for (CoordInt x = 0; x < _width; x++) {
        updateCell(x, _sliceY0);
    }
    if (_topSharedZone)
        _topSharedZone->unlock();

    for (CoordInt y = _sliceY0+1; y < _sliceY1; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            updateCell(x, y);
        }
    }

    if (_bottomSharedZone)
        _bottomSharedZone->lock();
    for (CoordInt x = 0; x < _width; x++) {
        updateCell(x, _sliceY1, false);
    }
    if (_bottomSharedZone)
        _bottomSharedZone->unlock();

    _finishedSignal->post();
}

void *AutomatonThread::execute()
{
    while (true) {
        suspend();
        update();
    }
    return 0;
}
