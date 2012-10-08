/**********************************************************************
File name: Movements.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "Movements.hpp"

#include <cassert>

#include "Errors.hpp"

using namespace PyEngine;

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
    _obj->movement = 0;
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
    _startX(_obj->x),
    _startY(_obj->y)
{
    if (abs(offsetX) + abs(offsetY) == 0) {
        throw ProgrammingError("Cannot move zero fields.");
    } else if (abs(offsetX) + abs(offsetY) > 1) {
        throw ProgrammingError("Cannot move diagonally or more than one field.");
    }

    // yes, this comparision is evil, as _obj->_x is a float actually.
    // However, in this case _x should be close enough to a whole number, if
    // not, something went utterly wrong.
    assert(_obj->x == _startX);
    assert(_obj->y == _startY);
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

bool MovementStraight::update(TimeFloat interval)
{
    _time += interval / duration;
    if (_time >= 1.0) {
        _time = 1.0;
    }

    _obj->x = _startX + _offX * _time;
    _obj->y = _startY + _offY * _time;

    if (_time >= 1.0) {
        deleteSelf();
        return true;
    }
    return false;
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
    _startX(_obj->x),
    _startY(_obj->y)
{

}

MovementRoll::~MovementRoll()
{
    _via->reservedBy = 0;
    _to->reservedBy = 0;
}

bool MovementRoll::update(TimeFloat interval)
{
    assert(false);
    deleteSelf();
    return true;
}

const double MovementStraight::duration = 0.5;
