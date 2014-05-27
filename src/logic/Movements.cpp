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

Movement::Movement(
        GameObject *obj,
        CoordInt offset_x,
        CoordInt offset_y):
    _time(0),
    _obj(obj),
    offset_x(offset_x),
    offset_y(offset_y)
{

}

Movement::~Movement()
{

}

void Movement::delete_self()
{
    _obj->after_movement();
    _obj->movement = nullptr;
    // movement is a unique_ptr, this will call the destructor
}

/* MovementStraight */

MovementStraight::MovementStraight(
        LevelCell *from,
        LevelCell *to,
        int offset_x, int offset_y):
    Movement::Movement(from->here, offset_x, offset_y),
    _from(from),
    _to(to),
    _startX(_obj->x),
    _startY(_obj->y)
{
    if (abs(offset_x) + abs(offset_y) == 0) {
        throw ProgrammingError("Cannot move zero fields.");
    } else if (abs(offset_x) + abs(offset_y) > 1) {
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
    // assert(!to->reserved_by);

    from->reserved_by = _obj;
    from->here = nullptr;
    to->here = _obj;

    _obj->cell = CoordPair{_startX + offset_x,
                           _startY + offset_y};
}

MovementStraight::~MovementStraight()
{
    _from->reserved_by = nullptr;
}

void MovementStraight::skip()
{
    _obj->x = _startX + offset_x;
    _obj->y = _startY + offset_y;
    delete_self();
}

bool MovementStraight::update()
{
    _time += 1;

    if (_to->reserved_by)
    {
        // if another object is currently moving out ouf the cell we’re moving
        // in, we have to make sure it gets updated before us, to avoid
        // collisions.
        _to->reserved_by->update();
    }

    if (_obj->info.is_rollable)
    {
        if (offset_x != 0) {
            _obj->phi += Level::time_slice / _obj->info.roll_radius * offset_x;
        } else {
            _obj->phi += sin(_time * Level::time_slice * 2*3.14159) / 100;
        }
    }

    if (_time >= 100) {
        _obj->x = _startX + offset_x;
        _obj->y = _startY + offset_y;
        _obj->view->invalidate();
        delete_self();
        return true;
    } else {
        _obj->x = _startX + offset_x * (_time * Level::time_slice / duration);
        _obj->y = _startY + offset_y * (_time * Level::time_slice / duration);
        _obj->view->invalidate();
        return false;
    }
}

/* MovementRoll */

MovementRoll::MovementRoll(
        LevelCell *from, LevelCell *via, LevelCell *to,
        int offset_x, int offset_y):
    Movement::Movement(from->here,
                       offset_x, offset_y),
    _from(from),
    _via(via),
    _to(to),
    _startX(_obj->x),
    _startY(_obj->y)
{
    if (abs(offset_x) != 1 || offset_y != 1) {
        throw ProgrammingError(
            "Cannot roll-move with offset_y != 1 or abs(offset_x != 1)");
    }

    assert(from->here);
    assert(!from->reserved_by);
    assert(!to->here);
    assert(!via->here);

    from->here = nullptr;
    from->reserved_by = _obj;
    via->reserved_by = _obj;
    to->here = _obj;

    _obj->cell = CoordPair{_startX + offset_x,
                           _startY + offset_y};
}

MovementRoll::~MovementRoll()
{
    _via->reserved_by = nullptr;
    _from->reserved_by = nullptr;
}

void MovementRoll::skip()
{
    _obj->x = _startX + offset_x;
    _obj->y = _startY + offset_y;
    _obj->view->invalidate();
    delete_self();
}

bool MovementRoll::update()
{
    _time += 1;

    if (_time >= 100) {
        skip();
        return true;
    }

    if (_time >= 50) {
        _obj->x = _startX + offset_x;
        _obj->y = _startY + offset_y * ((_time-50) * Level::time_slice * 2);
    } else {
        _obj->x = _startX + offset_x * (_time * Level::time_slice * 2);
        _obj->y = _startY;
    }

    _obj->view->invalidate();
    return false;
}

const double MovementStraight::duration = 1.0;
