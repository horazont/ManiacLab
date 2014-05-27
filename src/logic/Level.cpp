/**********************************************************************
File name: Level.cpp
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
#include "Level.hpp"

#include <cmath>
#include <cstdlib>

#include "CEngine/Misc/Exception.hpp"

#include "ExplosionObject.hpp"

/* Timer */

Timer::Timer(const TickCounter trigger_at,
             const CoordInt cellx,
             const CoordInt celly,
             const TimerFunc &func):
    trigger_at(trigger_at),
    x(cellx),
    y(celly),
    func(func)
{

}

/* Level */

#define WALL_CENTER_X 45

Level::Level(CoordInt width, CoordInt height, bool mp):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]()),
    _physics(
        width*subdivision_count,
        height*subdivision_count,
        SimulationConfig(
            0.3,        // flow friction
            0.991,      // flow damping
            0.3,        // convection friction
            0.05,       // heat flow friction
            0.1         // fog flow friction
        ),
        mp
    ),
    _objects(),
    _player(nullptr),
    _physics_particles(*this),
    _ticks(0)
{
    init_cells();
}

Level::~Level()
{
    delete[] _cells;
}


void Level::init_cells()
{
    const LevelCell *end = &_cells[_width*_height];
    for (LevelCell *cell = _cells; cell != end; cell++) {
        cell->here = 0;
        cell->reserved_by = 0;
    }
}

void Level::add_explosion(const CoordInt x,
                          const CoordInt y)
{
    _timers.emplace(
        _ticks + EXPLOSION_TRIGGER_TIMEOUT,
        x, y,
        [x, y](Level &level, LevelCell *cell) {
            if (cell->here) {
                cell->here->explosion_touch();
            }
            if (!cell->here) {
                ExplosionObject *obj = new ExplosionObject(&level);
                obj->setup_view();
                level.place_object(
                    obj,
                    x, y);
            }
        });

    _physics_particles.spawn_generator(
        4,
        [x, y](PhysicsParticle *part) {
            part->type = ParticleType::FIRE;
            const float offsx = ((float)rand() / RAND_MAX)*0.5-0.25;
            const float offsy = ((float)rand() / RAND_MAX)*0.5-0.25;
            part->x = x + 0.5 + offsx;
            part->y = y + 0.5 + offsy;
            part->vx = offsx / 2;
            part->vy = offsy / 2;
            part->ax = 0;
            part->ay = 0;
            part->lifetime = (EXPLOSION_BLOCK_LIFETIME + EXPLOSION_TRIGGER_TIMEOUT) / 100.;
        });
}

void Level::add_large_explosion(const CoordInt x0,
                                const CoordInt y0,
                                const CoordInt xradius,
                                const CoordInt yradius)
{
    const CoordInt minx = x0 > xradius-1 ? x0 - xradius : x0;
    const CoordInt miny = y0 > yradius-1 ? y0 - yradius : y0;
    const CoordInt maxx = x0 < _width - xradius ? x0 + xradius : x0;
    const CoordInt maxy = y0 < _height - yradius ? y0 + yradius : y0;

    for (CoordInt x = minx; x <= maxx; x++) {
        for (CoordInt y = miny; y <= maxy; y++) {
            add_explosion(x, y);
        }
    }

}

void Level::cleanup_cell(LevelCell *cell)
{
    GameObject *const obj = cell->here;
    if (obj)
    {
        if (obj == _player) {
            _on_player_death(this, obj);
            _player = nullptr;
        }
        _physics.clear_cells(
            obj->phy.x, obj->phy.y,
            obj->info.stamp);
        delete obj;
    }
    cell->here = nullptr;
}

void Level::debug_test_stamp(const double x, const double y)
{
    static CellInfo info_arr[cell_stamp_length];
    CellInfo *info = &info_arr[0];
    for (CoordInt x = 0; x < subdivision_count; x++) {
        for (CoordInt y = 0; y < subdivision_count; y++) {
            info->offs = CoordPair(x, y);
            info->meta.blocked = false;
            info->meta.obj = nullptr;
            info->phys.air_pressure = 1.0;
            info->phys.fog = 10.;
            info->phys.heat_energy = 1.0;
            info->phys.flow[0] = 0;
            info->phys.flow[1] = 0;
            info++;
        }
    }

    CoordPair coord = get_physics_coords(x, y);
    _physics.wait_for();
    _physics.place_stamp(coord.x, coord.y, &info_arr[0], cell_stamp_length);
}

void Level::debug_output(const double x, const double y)
{
    static const CoordInt offs[5][2] = {
        {0, -1}, {-1, 0}, {0, 0}, {1, 0}, {0, 1}
    };

    _physics.wait_for();

    const CoordPair phys = get_physics_coords(x, y);
    std::cout << "DEBUG: center at x = " << phys.x << "; y = " << phys.y << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "offs: " << offs[i][0] << ", " << offs[i][1] << std::endl;

        const CoordInt cx = phys.x + offs[i][0];
        const CoordInt cy = phys.y + offs[i][1];

        Cell *cell = _physics.safe_cell_at(cx, cy);
        if (!cell) {
            std::cout << "  out of range" << std::endl;
            continue;
        }
        CellMetadata *meta = _physics.meta_at(cx, cy);

        const double tc = (meta->blocked
                           ? meta->obj->info.temp_coefficient
                           : airtempcoeff_per_pressure * cell->air_pressure);

        if (meta->blocked) {
            std::cout << "  blocked with " << meta->obj << std::endl;
        }
        std::cout << "  p     = " << cell->air_pressure << std::endl;
        std::cout << "  U     = " << cell->heat_energy << std::endl;
        std::cout << "  T     = " << cell->heat_energy / tc << std::endl;
        std::cout << "  f     = " << cell->fog << std::endl;
        std::cout << "  f[-x] = " << cell->flow[0] << std::endl;
        std::cout << "  f[-y] = " << cell->flow[1] << std::endl;
    }
}

void Level::get_fall_channel(
    const CoordInt x,
    const CoordInt y,
    LevelCell *&aside,
    LevelCell *&asidebelow)
{
    aside = &_cells[x+y*_width];
    if (aside->here || aside->reserved_by)
    {
        aside = nullptr;
        asidebelow = nullptr;
        return;
    } else {
        asidebelow = &_cells[x+(y+1)*_width];
    }

    if (asidebelow->here || asidebelow->reserved_by)
    {
        aside = nullptr;
        asidebelow = nullptr;
    }
}

CoordPair Level::get_physics_coords(const double x, const double y)
{
    CoordPair result;
    result.x = round(x * subdivision_count);// - subdivision_count / 2;
    result.y = round(y * subdivision_count);// - subdivision_count / 2;
    return result;
}

void Level::physics_to_gl_texture(bool thread_regions)
{
    _physics.to_gl_texture(0.0, 2.0, thread_regions);
}

void Level::place_object(
    GameObject *obj,
    const CoordInt x,
    const CoordInt y)
{
    _physics.wait_for();

    LevelCell *dest = &_cells[x+y*_width];
    if (dest->reserved_by) {
        GameObject *obj = dest->reserved_by;
        const CoordPair oldpos = obj->phy;
        obj->movement->skip();
        const CoordPair newpos = get_physics_coords(
            obj->x,
            obj->y);
        _physics.move_stamp(
            oldpos.x, oldpos.y,
            newpos.x, newpos.y,
            obj->info.stamp);
    }
    cleanup_cell(dest);

    obj->x = x;
    obj->y = y;
    obj->cell = CoordPair{x, y};
    obj->phy = get_physics_coords(x, y);
    _physics.place_object(
        obj->phy.x, obj->phy.y,
        obj, 1.0);

    dest->here = obj;
}

void Level::place_player(
    GameObject *player,
    const CoordInt x,
    const CoordInt y)
{
    if (_player) {
        return;
    }

    place_object(player, x, y);
    _player = player;
}

void Level::update()
{
    _ticks += 1;

    _physics.wait_for();

    while (!_timers.empty() && _timers.top().trigger_at <= _ticks)
    {
        const Timer &timer = _timers.top();
        LevelCell *const cell = (timer.x >= 0 && timer.y >= 0)
            ? get_cell(timer.x, timer.y)
            : nullptr;

        _timers.top().func(*this, cell);
        _timers.pop();
    }

    for (CoordInt y = _height-1; y >= 0; y--)
    {
        LevelCell *cell = get_cell(0, y);
        for (CoordInt x = 0; x < _width; x++)
        {
            GameObject *obj = cell->here;
            if (!obj) {
                ++cell;
                continue;
            }
            obj->update();
            ++cell;
        }
    }

    _physics_particles.update(0.01);

    _physics.resume();
}
