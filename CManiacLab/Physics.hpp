#ifndef _ML_PHYSICS_H
#define _ML_PHYSICS_H

struct Cell;

struct Cell {
    double airPressure;
    double temperature;
    
    // flow is in relation to upper left neighbour!
    double flow[2];
    double fogDensity;
};

struct CellMetadata {
    bool blocked;
    GameObject *obj;
};

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
    Automaton(unsigned int width, unsigned int height,
        double flowFriction = 0.1,
        double flowDamping = 0.5,
        double initialPressure = 1.0, 
        double initialTemperature = 1.0);
    ~Automaton();
    
private:
    unsigned int _width, _height;
    CellMetadata *_metadata;
    Cell *_cells, *_backbuffer;
    double _flowFriction, _flowDamping;
    
private:
	void initMetadata(CellMetadata *buffer, unsigned int x, unsigned int y);
    void initCell(Cell *buffer, unsigned int x, unsigned int y,
        double initialPressure, double initialTemperature);
    
protected:
    void flow(const Cell *b_cellA, Cell *f_cellA, 
        const Cell *b_cellB, Cell *f_cellB,
        unsigned int direction);
    template<class CType>
    void getCellAndNeighbours(CType *buffer, CType **cell, 
        CType *(*neighbours)[2], unsigned int x, unsigned int y);
    void updateCell(unsigned int x, unsigned int y);
    
public:
    Cell inline *cellAt(unsigned int x, unsigned int y) { return &_cells[x+_width*y]; };
    CellMetadata inline *metaAt(unsigned int x, unsigned int y) { return &_metadata[x+_width*y]; };
    void printCells(const double min, const double max,
        const char **map, const int mapLen);
    void printCellsBlock(const double min, const double max);
    void printCells256(const double min, const double max);
    void printFlow();
    void setBlocked(unsigned int x, unsigned int y, bool blocked);
    void updateCells();
};

#endif
