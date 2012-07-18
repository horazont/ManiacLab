#include "automaton.hpp"

#include <iostream>
#include <cmath>
#include <cassert>

double inline clamp(const double value, const double min, const double max)
{
    if (value > max)
        return max;
    else if (value < min)
        return min;
    else
        return value;
}

Automaton::Automaton(unsigned int width, unsigned int height,
        double flowFriction, double flowDamping,
        double initialPressure, double initialTemperature):
    _width(width),
    _height(height),
    _cells(new Cell[width*height]()),
    _backbuffer(new Cell[width*height]()),
    _flowFriction(flowFriction),
    _flowDamping(flowDamping),
    _odd(false)
{
    for (unsigned int y = 0; y < _height; y++) {
        for (unsigned int x = 0; x < _width; x++) {
            initCell(_cells, x, y, initialPressure, initialTemperature, _odd);
            initCell(_backbuffer, x, y, initialPressure, initialTemperature, !_odd);
        }
    }
}

Automaton::~Automaton()
{
    delete[] _cells;
}

void Automaton::initCell(Cell *buffer, unsigned int x, unsigned int y,
    double initialPressure, double initialTemperature, bool odd)
{
    Cell *cell = &buffer[x+_width*y];
    cell->airPressure = initialPressure;
    cell->temperature = initialTemperature;
    cell->flow[0] = 0;
    cell->flow[1] = 0;
    cell->odd = odd;
}

void Automaton::getCellAndNeighbours(Cell *buffer, Cell **cell, 
        Cell *(*neighbours)[2], unsigned int x, unsigned int y)
{
    *cell = &buffer[x+_width*y];
    (*neighbours)[0] = (x > 0) ? &_cells[(x-1)+_width*y] : 0;
    (*neighbours)[1] = (y > 0) ? &_cells[x+_width*(y-1)] : 0;
    /*(*neighbours)[0] = (x < _width-1) ? &_cells[(x+1)+_width*y] : 0;
    (*neighbours)[1] = (y < _height-1) ? &_cells[x+_width*(y+1)] : 0;*/

}

void Automaton::flow(const Cell *b_cellA, Cell *f_cellA, 
    const Cell *b_cellB, Cell *f_cellB, 
    unsigned int direction)
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

void inline activateCell(Cell *front, Cell *back, const bool odd)
{
    front->airPressure = back->airPressure;
}

void Automaton::updateCell(unsigned int x, unsigned int y)
{
    Cell *b_self, *f_self;
    Cell *b_neighbours[2], *f_neighbours[2];
    getCellAndNeighbours(_backbuffer, &b_self, &b_neighbours, x, y);
    getCellAndNeighbours(_cells, &f_self, &f_neighbours, x, y);
    
    activateCell(f_self, b_self, _odd);
    for (unsigned int i = 0; i < 2; i++) {
        if (b_neighbours[i]) {
            flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i);
        }
    }
}

void Automaton::updateCells()
{
    // flip buffers
    Cell *_tmp = _backbuffer;
    _backbuffer = _cells;
    _cells = _tmp;
    _odd = !_odd;
    
    for (unsigned int y = 0; y < _height; y++) {
        for (unsigned int x = 0; x < _width; x++) {
            updateCell(x, y);
        }
    }
}

void Automaton::printCells(const double min, const double max,
    const char **map, const int mapLen)
{
    for (unsigned int y = 0; y < _height; y++) {
        for (unsigned int x = 0; x < _width; x++) {
            Cell *cell = cellAt(x, y);
            const double val = cell->airPressure;
            const double scaled = (val - min) / (max - min);
            int index = (int)(scaled * mapLen);
            if (index < 0)
                index = 0;
            if (index >= mapLen)
                index = mapLen-1;
            
            std::cout << map[index];
        }
        std::cout << "\n";
    }
    std::cout.flush();
}

void Automaton::printCellsBlock(const double min, const double max)
{
    static const char *map[5] = {
        " ", "░", "▒", "▓", "█"
    };
    static const int maxMap = 5;
    printCells(min, max, map, maxMap);
}

void Automaton::printCells256(const double min, const double max)
{
    static const char *map[24] = {
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
        "\x1b[38;5;255m█"
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
    
    for (unsigned int y = 0; y < _height; y++) {
        for (unsigned int x = 0; x < _width; x++) {
            Cell *cell = cellAt(x, y);
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
