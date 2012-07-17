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
		double initialPressure, double initialTemperature):
	_width(width),
	_height(height),
	_cells(new Cell[width*height]()),
	_backbuffer(new Cell[width*height]()),
	_flowSpeed(0.01),
	_flowDamping(0.2),
	_flowFriction(0.1),
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

double Automaton::flow(const Cell *b_cellA, Cell *f_cellA, 
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
	return flow;
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
			double fl = flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i);
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

void Automaton::printCells()
{
	static const char *map[5] = {
		" ", "░", "▒", "▓", "█"
	};
	static const int maxMap = 5;
	static const double minVal = 0.0;
	static const double maxVal = 2.0;
	
	for (unsigned int y = 0; y < _height; y++) {
		for (unsigned int x = 0; x < _width; x++) {
			Cell *cell = cellAt(x, y);
			const double val = cell->airPressure;
			const double scaled = (val - minVal) / (maxVal - minVal);
			/*int index = (int)(scaled * 9);
			char ch = index + 48;
			if (index < 0) 
				ch = '-';
			if (index > 9)
				ch = '+';*/
			
			int index = (int)(scaled * maxMap);
			if (index < 0)
				index = 0;
			if (index >= maxMap)
				index = maxMap-1;
			
			std::cout << map[index];
		}
		std::cout << std::endl;
	}
}

void Automaton::printFlow()
{
	static const char *map[8] = {
		"→", "↘", "↓", "↙", "←", "↖", "↑", "↗"
	};
	static const char *none = "•";
	
	for (unsigned int y = 0; y < _height; y++) {
		for (unsigned int x = 0; x < _width; x++) {
			Cell *cell = cellAt(x, y);
			if (sqrt(cell->flow[0]*cell->flow[0] + cell->flow[1]*cell->flow[1]) < 0.0001) {
				std::cout << none;
			} else {
				double angle = atan2(cell->flow[0], cell->flow[1]) + M_PI + M_PI / 4.;
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
