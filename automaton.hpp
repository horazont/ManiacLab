#ifndef AUTOMATON_H
#define AUTOMATON_H

class Cell;

class GameObject {
	protected:
	double _tempCoefficient;
	
	public:
	double inline getTemperatureCoefficient() { return _tempCoefficient; };
	virtual void update(const Cell *cell);
};

struct Cell {
	bool blocked;
	bool odd;
	double airPressure;
	double temperature;
	
	// flow is in relation to upper left neighbour!
	double flow[2];
	double fogDensity;
	
	GameObject *obj;
};

class Automaton {
	public:
	Automaton(unsigned int width, unsigned int height, 
		double initialPressure = 1.0, 
		double initialTemperature = 1.0);
	~Automaton();
	
	private:
	unsigned int _width, _height;
	Cell *_cells, *_backbuffer;
	double _flowSpeed, _flowDamping, _flowFriction;
	bool _odd;
	
	private:
	void initCell(Cell *buffer, unsigned int x, unsigned int y,
		double initialPressure, double initialTemperature, bool odd);
	
	protected:
	double flow(const Cell *b_cellA, Cell *f_cellA, 
		const Cell *b_cellB, Cell *f_cellB,
		unsigned int direction);
	void getCellAndNeighbours(Cell *buffer, Cell **cell, 
		Cell *(*neighbours)[2], unsigned int x, unsigned int y);
	void updateCell(unsigned int x, unsigned int y);
	
	public:
	Cell inline *cellAt(unsigned int x, unsigned int y) { return &_cells[x+_width*y]; };
	void printCells();
	void printFlow();
	void updateCells();
};

#endif
