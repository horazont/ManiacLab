#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <vector>

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"
#include "Physics.hpp"

struct Cell;

struct LevelCell {
    GameObject *here, *reservedBy;
};

class Level {
public:
    Level(CoordInt width, CoordInt height);
    ~Level();
private:
    CoordInt _width, _height;
    LevelCell *_cells;
    Automaton _physics;
    std::vector<GameObject*> _objects;

    double _timeSlice;
private:
    void getFallChannel(const CoordInt x, const CoordInt y, LevelCell **aside, LevelCell **asideBelow);
    void initCells();
protected:
    void getPhysicsCellsAt(const double x, const double y, CellStamp *stamp);
public:
    void update();
};

#endif
