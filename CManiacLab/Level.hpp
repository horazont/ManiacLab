#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <vector>

#include <boost/shared_ptr.hpp>

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
    Level(CoordInt width, CoordInt height, bool mp = true);
    ~Level();
private:
    CoordInt _width, _height;
    LevelCell *_cells;
    Automaton _physics;
    std::vector<GameObject*> _objects;

    double _timeSlice;
    double _time;
private:
    void getFallChannel(const CoordInt x, const CoordInt y, LevelCell **aside, LevelCell **asideBelow);
    bool handleCAInteraction(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    bool handleGravity(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    void initCells();
protected:
    void getPhysicsCellsAt(const double x, const double y, CellStamp *stamp,
        CoordInt *px, CoordInt *py);
public:
    void debug_testStamp(const double x, const double y, bool block);
    void debug_testBlockStamp();
    void debug_testUnblockStamp();
    void physicsToGLTexture(bool threadRegions);
    void setStamp(const double x, const double y, const Stamp &stamp, bool block);
    void update();
};

typedef boost::shared_ptr<Level> LevelHandle;

#endif
