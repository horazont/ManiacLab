/**********************************************************************
File name: GameObject.hpp
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
#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

#include "Movements.hpp"

struct Cell;
class Level;
class Movement;

class GameObject {
public:
    GameObject();
    virtual ~GameObject() {};
protected:
    bool _isGravityAffected, _isMoving, _isRollable;
    double _tempCoefficient;
    double _x, _y, _phi, _radius;
    Movement *_movement;
protected:
    void setMovement(Movement *movement);
public:
    Movement *getCurrentMovement() const { return _movement; };
    bool inline getIsGravityAffected() const { return _isGravityAffected; };
    bool inline getIsMoving() const { return _isMoving; };
    bool inline getIsRollable() const { return _isRollable; };
    double inline getRadius() const { return _radius; };
    double inline getTemperatureCoefficient() const { return _tempCoefficient; };
friend class MovementStraight;
friend class MovementRoll;
friend class Movement;
friend class Level;
};

#endif
