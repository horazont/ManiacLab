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

void Movement::delete_self()
{
    _obj->after_movement();
    _obj->movement = 0;
    // movement is a unique_ptr, this will call the destructor
}

/* MovementStraight */

MovementStraight::MovementStraight(
    LevelCell *from,
    LevelCell *to,
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
    assert(!from->reserved_by);
    assert(!to->here);
    assert(!to->reserved_by);

    from->reserved_by = _obj;
    from->here = nullptr;
    to->here = _obj;

    _obj->cell = CoordPair{_startX + _offX,
                           _startY + _offY};
}

MovementStraight::~MovementStraight()
{
    _from->reserved_by = nullptr;
}

void MovementStraight::skip()
{
    _obj->x = _startX + _offX;
    _obj->y = _startY + _offY;
    delete_self();
}

bool MovementStraight::update(TimeFloat interval)
{
    _time += interval / duration;
    if (_time >= 2.0) {
        _time = 2.0;
    }

    _obj->x = _startX + _offX * _time / 2;
    _obj->y = _startY + _offY * _time / 2;

    if (_time >= 2.0) {
        delete_self();
        return true;
    }

    _obj->view->invalidate();
    return false;
}

CoordPair MovementStraight::velocity_vector()
{
    return CoordPair{_offX, _offY};
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
    _via->reserved_by = 0;
    _to->reserved_by = 0;
}

void MovementRoll::skip()
{
    assert(false);
    delete_self();
}

bool MovementRoll::update(TimeFloat interval)
{
    _obj->view->invalidate();
    assert(false);
    delete_self();
    return true;
}

CoordPair MovementRoll::velocity_vector()
{
    assert(false);
    return CoordPair();
}

const double MovementStraight::duration = 0.5;
