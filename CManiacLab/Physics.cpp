#include "Physics.hpp"

#include <iostream>
#include <cmath>
#include <cassert>

#include "GameObject.hpp"

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
        double initialPressure, double initialTemperature):
    _width(width),
    _height(height),
    _metadata(new CellMetadata[width*height]()),
    _cells(new Cell[width*height]()),
    _backbuffer(new Cell[width*height]()),
    _flowFriction(0.1),
    _flowDamping(0.5)
{
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
			initMetadata(_metadata, x, y);
            initCell(_cells, x, y, initialPressure, initialTemperature);
            initCell(_backbuffer, x, y, initialPressure, initialTemperature);
        }
    }
}

Automaton::~Automaton()
{
    delete[] _cells;
    delete[] _backbuffer;
    delete[] _metadata;
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

void Automaton::activateCell(Cell *front, Cell *back)
{
    front->airPressure = back->airPressure;
}

void Automaton::flow(const Cell *b_cellA, Cell *f_cellA, 
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
    
    Cell *const flowChange = (flow > 0) ? f_cellB : f_cellA;
    const double factor = flow / flowChange->airPressure;
    flowChange->flow[direction] =
        applicableFlow * factor + flowChange->flow[direction] * (1.0 - factor);
}

template<class CType>
void Automaton::getCellAndNeighbours(CType *buffer, CType **cell, 
        CType *(*neighbours)[2], CoordInt x, CoordInt y)
{
    *cell = &buffer[x+_width*y];
    (*neighbours)[0] = (x > 0) ? &buffer[(x-1)+_width*y] : 0;
    (*neighbours)[1] = (y > 0) ? &buffer[x+_width*(y-1)] : 0;
}

void Automaton::updateCell(CoordInt x, CoordInt y)
{
    Cell *b_self, *f_self;
    CellMetadata *m_self;
    Cell *b_neighbours[2], *f_neighbours[2];
    CellMetadata *m_neighbours[2];
    getCellAndNeighbours(_metadata, &m_self, &m_neighbours, x, y);
    getCellAndNeighbours(_backbuffer, &b_self, &b_neighbours, x, y);
    getCellAndNeighbours(_cells, &f_self, &f_neighbours, x, y);
    
    activateCell(f_self, b_self);
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

void Automaton::setBlocked(CoordInt x, CoordInt y, bool blocked)
{
	_metadata[x+_width*y].blocked = true;
}

void Automaton::update()
{
    // flip buffers
    Cell *_tmp = _backbuffer;
    _backbuffer = _cells;
    _cells = _tmp;
    
    for (CoordInt y = 0; y < _height; y++) {
        for (CoordInt x = 0; x < _width; x++) {
            updateCell(x, y);
        }
    }
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
