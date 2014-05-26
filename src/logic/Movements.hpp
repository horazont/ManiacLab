/**********************************************************************
File name: Movements.hpp
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
#ifndef _ML_MOVEMENTS_H
#define _ML_MOVEMENTS_H

#include "CEngine/IO/Time.hpp"

#include "Physics.hpp"
#include "Level.hpp"

struct LevelCell;
class GameObject;

class Movement {
public:
    Movement(GameObject *obj);
    virtual ~Movement();

protected:
    TickCounter _time;
    GameObject *_obj;
    GameObject *_dependency;

protected:
    void delete_self();

public:
    virtual void skip() = 0;
    virtual bool update() = 0;
    virtual CoordPair velocity_vector() = 0;

};

class MovementStraight: public Movement {
public:
    MovementStraight(
        LevelCell *from, LevelCell *to,
        int offsetX, int offsetY);
    virtual ~MovementStraight();

private:
    LevelCell *_from, *_to;
    int _offX, _offY;
    CoordInt _startX, _startY;

public:
    void skip() override;
    bool update() override;
    virtual CoordPair velocity_vector() override;

public:
    static const double duration;

};

class MovementRoll: public Movement {
public:
    MovementRoll(
        LevelCell *from, LevelCell *via, LevelCell *to,
        int offsetX, int offsetY);
    virtual ~MovementRoll();

private:
    LevelCell *_from, *_via, *_to;
    int _offX, _offY;
    CoordInt _startX, _startY;

public:
    void skip() override;
    bool update() override;
    CoordPair velocity_vector() override;

};

#endif
