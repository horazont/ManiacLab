#ifndef _ML_MOVEMENTS_H
#define _ML_MOVEMENTS_H

#include "CEngine/IO/Time.hpp"

#include "Level.hpp"

struct LevelCell;
class GameObject;

class Movement {
public:
    Movement(GameObject *obj);
    virtual ~Movement();
protected:
    double _time;
    GameObject *_obj;
protected:
    void deleteSelf();
public:
    virtual bool update(PyEngine::TimeFloat interval) = 0;
};

class MovementStraight: public Movement {
public:
    MovementStraight(LevelCell *from, LevelCell *to, int offsetX, int offsetY);
    virtual ~MovementStraight();
private:
    LevelCell *_from, *_to;
    int _offX, _offY;
    CoordInt _startX, _startY;
public:
    virtual bool update(PyEngine::TimeFloat interval);
};

class MovementRoll: public Movement {
public:
    MovementRoll(LevelCell *from, LevelCell *via, LevelCell *to, int offsetX, int offsetY);
    virtual ~MovementRoll();
private:
    LevelCell *_from, *_via, *_to;
    int _offX, _offY;
    CoordInt _startX, _startY;
public:
    virtual bool update(PyEngine::TimeFloat interval);
};

#endif
