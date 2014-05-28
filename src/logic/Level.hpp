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

#include <queue>
#include <vector>

#include <sigc++/sigc++.h>

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"
#include "Physics.hpp"
#include "Particles.hpp"

struct Cell;
class Level;

struct LevelCell {
    GameObject *here, *reserved_by;
};

typedef std::function<void(Level&, LevelCell*)> TimerFunc;

struct Timer {
    Timer(const TickCounter trigger_at,
          const CoordInt cellx,
          const CoordInt celly,
          TimerFunc func);
    Timer(const Timer &ref) = delete;
    Timer &operator=(const Timer &ref) = delete;
    Timer(Timer &&ref);
    Timer &operator=(Timer &&ref);
    ~Timer();

    TickCounter trigger_at;
    CoordInt x, y;
    TimerFunc func;

    inline bool operator<(const Timer &other) const
    {
        return trigger_at > other.trigger_at;
    }

};

typedef sigc::signal<void, Level*, GameObject*> PlayerDeathEvent;

class Level {
public:
    static constexpr double time_slice = 0.01;

public:
    Level(CoordInt width, CoordInt height, bool mp = true);
    ~Level();

private:
    CoordInt _width, _height;
    LevelCell *_cells;
    Automaton _physics;
    std::vector<GameObject*> _objects;

    GameObject *_player;
    PlayerDeathEvent _on_player_death;

    ParticleSystem _physics_particles;

    TickCounter _ticks;
    std::priority_queue<Timer> _timers;

private:
    void init_cells();

public:
    void add_explosion(const CoordInt x,
                       const CoordInt y);

    void add_large_explosion(
        const CoordInt x0,
        const CoordInt y0,
        const CoordInt xradius,
        const CoordInt yradius);

    void cleanup_cell(LevelCell *cell);

    void debug_test_stamp(const double x, const double y);

    void debug_output(const double x, const double y);

    inline LevelCell *get_cell(
        CoordInt x, CoordInt y)
    {
        return &_cells[x+y*_width];
    }

    void get_fall_channel(
        const CoordInt x,
        const CoordInt y,
        LevelCell *&aside,
        LevelCell *&asidebelow);

    inline CoordInt get_height() const
    {
        return _height;
    }

    CoordPair get_physics_coords(const double x, const double y);

    inline TickCounter get_ticks() const
    {
        return _ticks;
    }

    inline CoordInt get_width() const
    {
        return _width;
    }

    inline ParticleSystem &particles()
    {
        return _physics_particles;
    }

    inline Automaton &physics()
    {
        return _physics;
    }

    void physics_to_gl_texture(bool thread_regions);

    void place_object(
        GameObject *obj,
        const CoordInt x,
        const CoordInt y);

    void place_player(
        GameObject *player,
        const CoordInt x,
        const CoordInt y);

    void update();

public:
    inline PlayerDeathEvent &on_player_death()
    {
        return _on_player_death;
    }

};

#endif
