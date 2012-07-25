#include "Level.hpp"

/* Level */

Level::Level(CoordInt width, CoordInt height):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]())
{
    initCells();
}

void Level::initCells()
{
    
    const LevelCell *end = &_cells[_width*_height];
    for (LevelCell *cell = _cells; cell != end; cell++) {
        cell->here = 0;
        cell->reservedBy = 0;
        // cell->physicsCells = ;
    }
}
