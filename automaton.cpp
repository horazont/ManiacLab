#include "automaton.hpp"

#include <iostream>
#include <cmath>


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
	_flowSpeed(0.005),
	_minFlow(0.000001),
	_flowDamping(0.0),
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
		Cell *(*neighbours)[4], unsigned int x, unsigned int y)
{
	*cell = &buffer[x+_width*y];
	(*neighbours)[0] = (x > 0) ? &_cells[(x-1)+_width*y] : 0;
	(*neighbours)[1] = (y > 0) ? &_cells[x+_width*(y-1)] : 0;
	(*neighbours)[2] = (x < _width-1) ? &_cells[(x+1)+_width*y] : 0;
	(*neighbours)[3] = (y < _height-1) ? &_cells[x+_width*(y+1)] : 0;
}

double Automaton::flow(const Cell *b_cellA, Cell *f_cellA, 
	const Cell *b_cellB, Cell *f_cellB, 
	unsigned int direction)
{
	const double dPressure = b_cellA->airPressure - b_cellB->airPressure;
	//std::cout << b_cellA->airPressure << " " << b_cellB->airPressure << std::endl;
	const double newFlow = clamp(dPressure * _flowSpeed,
		-b_cellB->airPressure / 8.0, 
		b_cellA->airPressure / 8.0);
	//const double oldFlow = (b_cellA->flow[direction] + b_cellB->flow[direction]) / 2.;
	
	//const double flow = oldFlow * _flowDamping + newFlow * (1.0 - _flowDamping);
	const double flow = newFlow;
	f_cellA->airPressure -= flow;
	f_cellB->airPressure += flow;
	return flow;
}

void inline activateCell(Cell *cell, const bool odd)
{
	if (cell->odd != odd) {
		cell->flow[0] = 0.;
		cell->flow[1] = 0.;
	}
}

void Automaton::updateCell(unsigned int x, unsigned int y)
{
	Cell *b_self, *f_self;
	Cell *b_neighbours[4], *f_neighbours[4];
	getCellAndNeighbours(_backbuffer, &b_self, &b_neighbours, x, y);
	getCellAndNeighbours(_cells, &f_self, &f_neighbours, x, y);
	
	// activateCell(f_self, _odd);
	for (unsigned int i = 0; i < 4; i++) {
		if (b_neighbours[i]) {
			activateCell(f_neighbours[i], _odd);
			double fl = flow(b_self, f_self, b_neighbours[i], f_neighbours[i], i%2);
			if ((x >= _width - 1) and (y >= _height - 1)) 
			{
				std::cout << x << " " << y << " " << i << ": " << fl << std::endl;
			}
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
	static const char map[] = " .:x%#";
	static const int maxMap = sizeof(map) / sizeof(char) - 1;
	static const double minVal = 0.0;
	static const double maxVal = 2.0;
	
	for (unsigned int y = 0; y < _height; y++) {
		for (unsigned int x = 0; x < _width; x++) {
			Cell *cell = cellAt(x, y);
			const double val = cell->airPressure;
			const double scaled = (val - minVal) / (maxVal - minVal);
			int index = (int)(scaled * maxMap);
			/*char ch = index + 48;
			if (index < 0) 
				ch = '-';
			if (index > 9)
				ch = '+';*/
			if (index < 0)
				index = 0;
			if (index >= maxMap)
				index = maxMap-1;
			
			std::cout << map[index];
		}
		std::cout << std::endl;
	}
}
