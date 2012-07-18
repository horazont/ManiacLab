#ifndef AUTOMATON_H
#define AUTOMATON_H

struct Cell;

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

/** \rst
Create a cellular automaton which simulates air flow between a given grid of
cells more or less correct. There are some free parameters you may change
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
    Automaton(unsigned int width, unsigned int height,
        double flowFriction = 0.1,
        double flowDamping = 0.5,
        double initialPressure = 1.0, 
        double initialTemperature = 1.0);
    ~Automaton();
    
private:
    unsigned int _width, _height;
    Cell *_cells, *_backbuffer;
    double _flowFriction, _flowDamping;
    bool _odd;
    
private:
    void initCell(Cell *buffer, unsigned int x, unsigned int y,
        double initialPressure, double initialTemperature, bool odd);
    
protected:
    void flow(const Cell *b_cellA, Cell *f_cellA, 
        const Cell *b_cellB, Cell *f_cellB,
        unsigned int direction);
    void getCellAndNeighbours(Cell *buffer, Cell **cell, 
        Cell *(*neighbours)[2], unsigned int x, unsigned int y);
    void updateCell(unsigned int x, unsigned int y);
    
public:
    Cell inline *cellAt(unsigned int x, unsigned int y) { return &_cells[x+_width*y]; };
    void printCells(const double min, const double max,
        const char **map, const int mapLen);
    void printCellsBlock(const double min, const double max);
    void printCells256(const double min, const double max);
    void printFlow();
    void updateCells();
};

#endif
