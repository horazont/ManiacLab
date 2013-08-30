/**********************************************************************
File name: Level.hpp
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
#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"
#include "Physics.hpp"

struct Cell;

class TestObject;

struct LevelCell {
    GameObject *here, *reserved_by;
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

    double _time_slice;
    double _time;
private:
    void get_fall_channel(const CoordInt x, const CoordInt y, LevelCell **aside, LevelCell **asidebelow);
    bool handle_ca_interaction(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    bool handle_gravity(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    void init_cells();
protected:
    CoordPair get_physics_coords(const double x, const double y);
public:
    void cleanup_cell(LevelCell *cell);
    void debug_test_object();
    TestObject *debug_place_object(const CoordInt x, const CoordInt y);
    void debug_test_stamp(const double x, const double y);
    void debug_output(const double x, const double y);
    void physics_to_gl_texture(bool thread_regions);
    void update();
};

typedef boost::shared_ptr<Level> LevelHandle;

#endif
