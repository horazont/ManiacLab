#include "Physics.hpp"

#include <iostream>
#include <cmath>
#include <cassert>

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

    const CoordInt sliceSize = _height / _threadCount;
    // XXX: This will break this game on systems with more threads than we have
    // lines in the automaton.
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

void Automaton::getCellStampAt(const CoordInt left, const CoordInt top,
    CellStamp *stamp)
{
    Cell **currCell = (Cell**)stamp;
    for (CoordInt x = left; x < left + subdivisionCount; x++)
    {
        for (CoordInt y = top; y < top + subdivisionCount; y++)
        {
            *currCell = (x >= 0 && x < _width && y >= 0 && y < _height) ? cellAt(x, y) : 0;
            currCell++;
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
}

void Automaton::printCells(const double min, const double max,
    const char **map, const int mapLen)
{
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

void Automaton::toGLTexture(const double min, const double max)
{
    if (!_rgbaBuffer) {
        _rgbaBuffer = (uint32_t*)malloc(_width*_height*4);
    }

    uint32_t *target = _rgbaBuffer;
    Cell *source = _cells;
    CellMetadata *metaSource = _metadata;
    for (unsigned int i = 0; i < _width*_height; i++) {
        if (metaSource->blocked) {
            *target = 0x0000FF;
        } else {
            const unsigned char r = (unsigned char)(clamp((source->airPressure - min) / (max - min), 0.0, 1.0) * 255.0);
            //const unsigned char g = (unsigned char)((double)(int)(((double)(i / _width)) / _height * _threadCount) / _threadCount * 255.0);
            //const unsigned char b = (unsigned char)(clamp((source->flow[1] - min) / (max - min), -1.0, 1.0) * 127.0 + 127.0);
            *target = r | (r << 8) | (r << 16);
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
    
    f_cellA->airPressure -= applicableFlow;
    f_cellB->airPressure += applicableFlow;
    f_cellA->flow[direction] = flow;
    
    /*Cell *const flowChange = (flow > 0) ? f_cellB : f_cellA;
    const double factor = flow / flowChange->airPressure;
    flowChange->flow[direction] =
        applicableFlow * factor + flowChange->flow[direction] * (1.0 - factor);*/
}

template<class CType>
void AutomatonThread::getCellAndNeighbours(CType *buffer, CType **cell, 
        CType *(*neighbours)[2], CoordInt x, CoordInt y)
{
    *cell = &buffer[x+_width*y];
    (*neighbours)[0] = (x > 0) ? &buffer[(x-1)+_width*y] : 0;
    (*neighbours)[1] = (y > 0) ? &buffer[x+_width*(y-1)] : 0;
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
