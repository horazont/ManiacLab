#include "Movements.hpp"

#include <cassert>

#include "Errors.hpp"

/* Movement */

Movement::Movement(GameObject *obj):
    _time(0),
    _obj(obj)
{

}

Movement::~Movement()
{

}

void Movement::deleteSelf()
{
    _obj->_movement = 0;
    delete this;
}

/* MovementStraight */

MovementStraight::MovementStraight(LevelCell *from, LevelCell *to,
        int offsetX, int offsetY):
    Movement::Movement(from->here),
    _from(from),
    _to(to),
    _offX(offsetX),
    _offY(offsetY),
    _startX(_obj->_x),
    _startY(_obj->_y)
{
    if (abs(offsetX) + abs(offsetY) == 0) {
        throw ProgrammingError("Cannot move zero fields.");
    } else if (abs(offsetX) + abs(offsetY) > 1) {
        throw ProgrammingError("Cannot move diagonally.");
    }

    // yes, this comparision is evil, as _obj->_x is a float actually.
    // However, in this case _x should be close enough to a whole number, if
    // not, something went utterly wrong.
    assert(_obj->_x == _startX);
    assert(_obj->_y == _startY);
    assert(from->here);
    assert(!from->reservedBy);
    assert(!to->here);
    assert(!to->reservedBy);

    from->reservedBy = _obj;
    from->here = 0;
    to->here = _obj;
}

MovementStraight::~MovementStraight()
{
    _from->reservedBy = 0;
}

bool MovementStraight::update(PyEngine::TimeFloat interval)
{
    _time += interval;
    if (_time >= 1.0) {
        _obj->_x = _startX + _offX;
        _obj->_y = _startY + _offY;
        deleteSelf();
        return true;
    } else {
        _obj->_x = _startX + _offX * _time;
        _obj->_y = _startX + _offY * _time;
        return false;
    }
}

/* MovementRoll */

MovementRoll::MovementRoll(LevelCell *from, LevelCell *via, LevelCell *to,
        int offsetX, int offsetY):
    Movement::Movement(from->here),
    _from(from),
    _via(via),
    _to(to),
    _offX(offsetX),
    _offY(offsetY),
    _startX(_obj->_x),
    _startY(_obj->_y)
{

}

MovementRoll::~MovementRoll()
{
    _via->reservedBy = 0;
    _to->reservedBy = 0;
}

bool MovementRoll::update(PyEngine::TimeFloat interval)
{
    assert(false);
    deleteSelf();
    return true;
}
