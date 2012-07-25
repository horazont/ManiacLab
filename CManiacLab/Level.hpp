#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"

struct Cell;

struct LevelCell {
    Cell *physicsCells[9];
    GameObject *here, *reservedBy;
};

class Level {
public:
    Level(CoordInt width, CoordInt height);
    ~Level();
private:
    CoordInt _width, _height;
    LevelCell *_cells;
private:
    void initCells();
public:
    
};

#endif
