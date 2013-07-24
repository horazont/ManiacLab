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

#include <CEngine/IO/Stream.hpp>
#include <CEngine/Misc/Exception.hpp>

#include "Movements.hpp"
#include "Stamp.hpp"

struct Cell;
class Level;
class Movement;

// If this ever exceeds two bytes, an increase in the version number is
// mandatory!
enum TemplateBinaryFlags {
    TBF_HAS_STAMP             = 0x0001,
    TBF_GRAVITY_AFFECTED      = 0x0002,
    TBF_ROLLABLE              = 0x0004,
};

struct Template {
public:
    Template();
    Template(Template const &ref);
    Template& operator=(Template const &ref);
    virtual ~Template();

public:
    Stamp *stamp;
    bool is_gravity_affected, is_rollable;
    double temp_coefficient;
    double radius;

};

struct GameObject: public Template {
public:
    GameObject();
    GameObject(GameObject const &ref);
    GameObject& operator=(GameObject const &ref);
    virtual ~GameObject();
public:
    double x, y, phi;
    Movement *movement;

    CoordPair phy;
};

#endif
