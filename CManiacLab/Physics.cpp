#include "Physics.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>

#include <glew.h>

#include <CEngine/IO/Thread.hpp>

#include "GameObject.hpp"

using namespace PyEngine;

double inline clamp(const double value, const double min, const double max)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    else
        return value;
}

Automaton::Automaton(CoordInt width, CoordInt height,
        const double flowFriction, const double flowDamping,
        bool mp,
        double initialPressure, double initialTemperature):
    _resumed(false),
    _width(width),
    _height(height),
    _metadata(new CellMetadata[width*height]()),
    _cells(new Cell[width*height]()),
    _backbuffer(new Cell[width*height]()),
    _flowFriction(flowFriction),
    _flowDamping(flowDamping),
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
    cell->temperature = initialTemperature;
    cell->flow[0] = 0;
    cell->flow[1] = 0;
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

void Automaton::applyBlockStamp(const CoordInt dx, const CoordInt dy,
    const Stamp &stamp, bool block)
{
    assert(!_resumed);

    const intptr_t indexRowLength = subdivisionCount+2;
    const intptr_t indexLength = indexRowLength * indexRowLength;
    static intptr_t *borderIndicies = (intptr_t*)malloc(indexLength * sizeof(intptr_t));
    static Cell **borderCells = (Cell**)malloc(indexLength * sizeof(Cell*));
    static const intptr_t offs[4][2] = {
        {0, -1}, {-1, 0}, {1, 0}, {0, 1}
    };

    uintptr_t stampCellsLen = 0;
    const CoordPair *stampCells = stamp.getMapCoords(&stampCellsLen);

    memset(borderIndicies, -1, indexLength * sizeof(intptr_t));
    memset(borderCells, 0, indexLength * sizeof(Cell*));

    // collect surplus matter here
    double toDistribute = 0.;
    uintptr_t borderCellWriteIndex = 0;
    uintptr_t borderCellCount = 0;

    for (uintptr_t i = 0; i < stampCellsLen; i++) {
        const CoordPair p = stampCells[i];
        const CoordInt x = p.x + dx;
        const CoordInt y = p.y + dy;

        Cell *const currCell = safeCellAt(x, y);
        if (!currCell) {
            continue;
        }
        CellMetadata *const currMeta = metaAt(x, y);
        if (!currMeta->blocked && block) {
            toDistribute += currCell->airPressure;
            currCell->airPressure = 0;

            for (uintptr_t j = 0; j < 4; j++) {
                const uintptr_t indexCell = (p.y + offs[j][1] + 1) * indexRowLength + p.x + 1 + offs[j][0];

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

                borderIndicies[indexCell] = borderCellWriteIndex;
                borderCells[borderCellWriteIndex] = neighCell;
                borderCellWriteIndex++;
                borderCellCount++;
            }

            const uintptr_t indexCell = (p.y + 1) * indexRowLength + (p.x + 1);
            if (borderIndicies[indexCell] >= 0) {
                borderCellCount--;
                borderCells[borderIndicies[indexCell]] = 0;
            }
            borderIndicies[indexCell] = -2;
        } else if (!block && currMeta->blocked) {
            initCell(_cells, x, y, 0, 0);
            initCell(_backbuffer, x, y, 0, 0);
        }

        currMeta->blocked = block;
    }

    if (toDistribute == 0 || !block)
        return;

    if (borderCellCount == 0) {
        fprintf(stderr, "[PHY!] [NC] no cells to move stuff to\n");
        return;
    }

    const double perCell = toDistribute / borderCellCount;

    unsigned int j = 0;
    for (Cell **neighCell = &borderCells[0]; j < borderCellCount; neighCell++) {
        if (!(*neighCell)) {
            continue;
        }
        (*neighCell)->airPressure += perCell;
        j++;
    }
}

void Automaton::evacuateCellToNeighbours(const CoordInt x, const CoordInt y,
    Cell *cell)
{
    static const int offsets[4][3] = {
        {0, -1, 0}, {-1, 0, 1}, {1, 0, 1}, {0, 1, 0}
    };
    int count = 0;
    const double toDistribute = cell->airPressure;
    if (toDistribute == 0)
        // nothing to do!
        return;

    Cell* goodCells[4] = {0, 0, 0, 0};
    int directions[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};

    for (int i = 0; i < 4; i++) {
        const int d = offsets[i][0] + offsets[i][1];
        const CoordInt nx = x + offsets[i][0];
        const CoordInt ny = y + offsets[i][1];
        const CoordInt direction = offsets[i][2];

        Cell *const neigh = safeCellAt(nx, ny);
        if (!neigh)
            continue;
        CellMetadata *const neighMeta = metaAt(nx, ny);
        if (neighMeta->blocked)
            continue;

        goodCells[count] = neigh;
        directions[count][0] = d;
        directions[count][1] = direction;

        executeFlowEx(cell, neigh, d, direction, toDistribute / 4);

        count++;
    }

    assert(count <= 4 && count >= 0);
    if (count == 0) {
        // this is not good, we could not distribute the pressure anywhere. this
        // leads to non-conservative systems, as we have to drop it.
        printf("[NC] could not move stuff out of blocked area");
        return;
    }
    if (count < 4) {
        // iterate over the good cells again to distribute the remaining
        // pressure
        const double perCell = (toDistribute - toDistribute * count / 4.) / count;

        for (int i = 0; i < count; i++) {
            Cell *const neigh = goodCells[i];
            const int d = directions[i][0];
            const CoordInt direction = directions[i][1];

            executeFlowEx(cell, neigh, d, direction, perCell);
        }
    }
    assert(abs(cell->airPressure) < 1e-16);
}

inline void Automaton::executeFlowEx(Cell *cellA, Cell *cellB, const int reverse,
    const CoordInt direction, const double move)
{
    if (reverse < 0) {
        AutomatonThread::executeFlow(cellA, cellB, direction, move, 0);
    } else {
        AutomatonThread::executeFlow(cellB, cellA, direction, -move, 0);
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

    uint32_t *target = _rgbaBuffer;
    Cell *source = _backbuffer;
    CellMetadata *metaSource = _metadata;
    for (CoordInt i = 0; i < _width*_height; i++) {
        if (metaSource->blocked) {
            *target = 0x0000FF;
        } else {
            const unsigned char r = (unsigned char)(clamp((source->airPressure - min) / (max - min), 0.0, 1.0) * 255.0);
            if (threadRegions) {
                const unsigned char g = (unsigned char)((double)(int)(((double)(i / _width)) / _height * _threadCount) / _threadCount * 255.0);
                //const unsigned char b = (unsigned char)(clamp((source->flow[1] - min) / (max - min), -1.0, 1.0) * 127.0 + 127.0);
                *target = r | (r << 8) | (g << 16);
            } else {
                *target = r | (r << 8) | (r << 16);
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
    _flowFriction(dataClass->_flowFriction),
    _flowDamping(dataClass->_flowDamping),
    _backbuffer(dataClass->_backbuffer),
    _cells(dataClass->_cells),
    _metadata(dataClass->_metadata)
{

}

void AutomatonThread::activateCell(Cell *front, Cell *back)
{
    front->airPressure = back->airPressure;
    front->flow[0] = back->flow[0];
    front->flow[1] = back->flow[1];
}

inline void AutomatonThread::executeFlow(Cell *cellA, Cell *cellB,
    CoordInt direction, const double flow, const double newFlow)
{
    cellA->airPressure -= flow;
    cellB->airPressure += flow;
    cellA->flow[direction] = newFlow;
}

template<class CType>
void AutomatonThread::getCellAndNeighbours(CType *buffer, CType **cell,
        CType *(*neighbours)[2], CoordInt x, CoordInt y)
{
    *cell = &buffer[x+_width*y];
    (*neighbours)[0] = (x > 0) ? &buffer[(x-1)+_width*y] : 0;
    (*neighbours)[1] = (y > 0) ? &buffer[x+_width*(y-1)] : 0;
}

void AutomatonThread::flow(const Cell *b_cellA, Cell *f_cellA,
    const Cell *b_cellB, Cell *f_cellB,
    CoordInt direction)
{
    const double dPressure = b_cellA->airPressure - b_cellB->airPressure;
    //std::cout << b_cellA->airPressure << " " << b_cellB->airPressure << std::endl;
    const double newFlow = dPressure * _flowFriction;
    const double oldFlow = b_cellA->flow[direction];
    const double flow = oldFlow * _flowDamping + newFlow * (1.0 - _flowDamping);
    const double applicableFlow = clamp(
        flow,
        -b_cellB->airPressure / 4.,
        b_cellA->airPressure / 4.
    );

    executeFlow(f_cellA, f_cellB, direction, applicableFlow, flow);

    /*Cell *const flowChange = (flow > 0) ? f_cellB : f_cellA;
    const double factor = flow / flowChange->airPressure;
    flowChange->flow[direction] =
        applicableFlow * factor + flowChange->flow[direction] * (1.0 - factor);*/
}

void AutomatonThread::updateCell(CoordInt x, CoordInt y, bool activate)
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
    if (m_self->blocked) {
		return;
	}
    for (CoordInt i = 0; i < 2; i++) {
        if (b_neighbours[i]) {
			if (!m_neighbours[i]->blocked) {
				flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i);
			}
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
