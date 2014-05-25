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

#include <sigc++/sigc++.h>

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"
#include "Physics.hpp"
#include "Particles.hpp"

struct Cell;

class TestObject;

struct LevelCell {
    GameObject *here, *reserved_by;
};

class Level;

typedef sigc::signal<void, Level*, GameObject*> PlayerDeathEvent;

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

    GameObject *_player;
    PlayerDeathEvent _on_player_death;

    ParticleSystem _physics_particles;

private:
    void get_fall_channel(
        const CoordInt x,
        const CoordInt y,
        LevelCell *&aside,
        LevelCell *&asidebelow);
    bool handle_ca_interaction(
        const CoordInt x,
        const CoordInt y,
        LevelCell &cell,
        GameObject &obj);
    bool handle_gravity(
        const CoordInt x,
        const CoordInt y,
        LevelCell &cell,
        GameObject &obj);
    void init_cells();

public:
    void cleanup_cell(LevelCell *cell);
    void debug_test_stamp(const double x, const double y);
    void debug_output(const double x, const double y);
    CoordPair get_physics_coords(const double x, const double y);

    inline ParticleSystem &particles()
    {
        return _physics_particles;
    }

    inline Automaton &physics()
    {
        return _physics;
    }

    void place_player(
        GameObject *player,
        const CoordInt x,
        const CoordInt y);
    void physics_to_gl_texture(bool thread_regions);
    void update();

    inline LevelCell *get_cell(
        CoordInt x, CoordInt y)
    {
        return &_cells[x+y*_width];
    }

    inline CoordInt get_width() const
    {
        return _width;
    }

    inline CoordInt get_height() const
    {
        return _height;
    }

public:
    inline PlayerDeathEvent &on_player_death()
    {
        return _on_player_death;
    }

};

#endif
