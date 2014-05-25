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
    _time_slice(0.01),
    _time(0),
    _player(nullptr),
    _physics_particles(*this)
{
    init_cells();
}

Level::~Level()
{
    delete[] _cells;
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
    }
    else
        asidebelow = &_cells[x+(y+1)*_width];

    if (asidebelow->here || asidebelow->reserved_by)
    {
        aside = 0;
        asidebelow = 0;
    }
}

bool Level::handle_ca_interaction(
    const CoordInt x,
    const CoordInt y,
    LevelCell &cell,
    GameObject &obj)
{
    return true;
}

bool Level::handle_gravity(
    const CoordInt x,
    const CoordInt y,
    LevelCell &cell,
    GameObject &obj)
{
    if (y == _height - 1) {
        // TODO: allow objects to leave the gamescope
        return true;
    }
    assert(!(obj.movement));

    LevelCell *below = &_cells[x+(y+1)*_width];
    if (!below->here && !below->reserved_by) {
        obj.movement = std::unique_ptr<Movement>(
            new MovementStraight(&cell, below, 0, 1));
    } else if (below->here
        && below->here->info.is_rollable
        && obj.info.is_rollable)
    {
        LevelCell *left = 0, *left_below = 0;
        LevelCell *right = 0, *right_below = 0;
        if (x > 0) {
            get_fall_channel(x-1, y, left, left_below);
        }
        if (x < _width-1) {
            get_fall_channel(x+1, y, right, right_below);
        }

        LevelCell *selected = 0, *selected_below = 0;
        CoordInt xoffset = 0;
        if (left) {
            // TODO: Use random here?
            selected = left;
            selected_below = left_below;
            xoffset = -1;
        } else if (right) {
            selected = right;
            selected_below = right_below;
            xoffset = 1;
        }

        if (selected) {
            obj.movement = std::unique_ptr<Movement>(
                new MovementRoll(
                    &cell,
                    selected,
                    selected_below,
                    xoffset, 1));
        }
    }
    return true;
}

void Level::init_cells()
{
    const LevelCell *end = &_cells[_width*_height];
    for (LevelCell *cell = _cells; cell != end; cell++) {
        cell->here = 0;
        cell->reserved_by = 0;
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

void Level::place_player(
    GameObject *player,
    const CoordInt x,
    const CoordInt y)
{
    if (_player) {
        return;
    }

    _physics.wait_for();

    LevelCell *dest = &_cells[x+y*_width];
    if (dest->reserved_by) {
        GameObject *obj = dest->reserved_by;
        const CoordPair oldpos = get_physics_coords(
            obj->x,
            obj->y);
        obj->movement->abort();
        const CoordPair newpos = get_physics_coords(
            obj->x,
            obj->y);
        _physics.move_stamp(
            oldpos.x, oldpos.y,
            newpos.x, newpos.y,
            obj->info.stamp);
    }
    cleanup_cell(dest);

    _player = player;

    _player->x = x;
    _player->y = y;
    _player->phy = get_physics_coords(x, y);

    std::cout << _player->phy.x << " " << _player->phy.y << std::endl;

    _physics.place_object(_player->phy.x, _player->phy.y, _player, 1.0);
    dest->here = _player;
}

void Level::update()
{
    _physics.wait_for();
    LevelCell *cell = &_cells[-1];
    for (CoordInt y = 0; y < _height; y++)
    {
        for (CoordInt x = 0; x < _width; x++)
        {
            cell++;
            GameObject *obj = cell->here;
            if (!obj)
                continue;

            if (!handle_ca_interaction(x, y, *cell, *obj)) {
                cleanup_cell(cell);
                continue;
            }

            Movement *movement = obj->movement.get();
            if (movement) {
                const CoordPair vel = movement->velocity_vector();
                if (movement->update(_time_slice)) {
                    movement = nullptr;
                }
                CoordPair new_coords = get_physics_coords(obj->x, obj->y);
                if (new_coords != obj->phy) {
                    if (obj->info.stamp.non_empty()) {
                        _physics.move_stamp(
                            obj->phy.x, obj->phy.y,
                            new_coords.x, new_coords.y,
                            obj->info.stamp,
                            &vel
                        );
                    }
                    obj->phy = new_coords;
                }
            }

            if (obj->info.is_gravity_affected && !movement) {
                if (!handle_gravity(x, y, *cell, *obj)) {
                    cleanup_cell(cell);
                    continue;
                }
            }

            if (obj->acting != NONE && !movement) {
                CoordInt offsx = 0;
                CoordInt offsy = 0;
                switch (obj->acting) {
                case MOVE_UP:
                {
                    offsy = -1;
                    break;
                };
                case MOVE_DOWN:
                {
                    offsy = 1;
                    break;
                }
                case MOVE_LEFT:
                {
                    offsx = -1;
                    break;
                }
                case MOVE_RIGHT:
                {
                    offsx = 1;
                    break;
                }
                default: {}
                }

                obj->acting = NONE;

                const CoordInt neighx = offsx + x;
                const CoordInt neighy = offsy + y;

                if ((offsx != 0 || offsy != 0)
                    && neighx >= 0 && neighx < _width
                    && neighy >= 0 && neighy < _height)
                {
                    LevelCell *neighbour = &_cells[neighx+neighy*_width];
                    obj->movement = std::unique_ptr<Movement>(
                        new MovementStraight(
                            cell, neighbour,
                            offsx, offsy));
                }
            }
        }
    }

    _time += _time_slice;

    _physics_particles.update(0.01);

    _physics.resume();
}
