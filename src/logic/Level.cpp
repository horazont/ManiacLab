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

#include "TestObject.hpp"

/* Level */

#define WALL_CENTER_X 45

Level::Level(CoordInt width, CoordInt height, bool mp):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]()),
    _physics(Automaton(width*subdivision_count, height*subdivision_count, SimulationConfig(
        0.3,        // flow friction
        0.991,      // flow damping
        0.3,        // convection friction
        0.05,       // heat flow friction
        0.1         // fog flow friction
    ), mp)),
    _objects(),
    _time_slice(0.01),
    _time(0)
{
    init_cells();
    for (CoordInt y = 0; y < _height; y++) {
        if (y < 4 || y > 46 || y % 4 != 0) {
            TestObject *obj = debug_place_object(WALL_CENTER_X-1, y);
            obj->set_is_affected_by_gravity(false);
        }
        if (y < 4 || y > 46 || y % 4 != 2) {
            TestObject *obj = debug_place_object(WALL_CENTER_X+1, y);
            obj->set_is_affected_by_gravity(false);
        }
    }
}

Level::~Level()
{
    delete[] _cells;
}

void Level::get_fall_channel(const CoordInt x, const CoordInt y,
        LevelCell **aside, LevelCell **asidebelow)
{
    *aside = &_cells[x+y*_width];
    if ((*aside)->here || (*aside)->reserved_by)
    {
        *aside = 0;
        *asidebelow = 0;
        return;
    }
    else
        *asidebelow = &_cells[x+(y+1)*_width];
    if ((*asidebelow)->here || (*asidebelow)->reserved_by)
    {
        *aside = 0;
        *asidebelow = 0;
    }
}

bool Level::handle_ca_interaction(const CoordInt x, const CoordInt y,
    LevelCell *cell, GameObject *obj)
{
    return true;
}

bool Level::handle_gravity(const CoordInt x, const CoordInt y, LevelCell *cell,
    GameObject *obj)
{
    if (y == _height - 1) {
        // TODO: allow objects to leave the gamescope
        return true;
    }
    assert(!(obj->movement));

    LevelCell *below = &_cells[x+(y+1)*_width];
    if (!below->here && !below->reserved_by) {
        obj->movement = new MovementStraight(cell, below, 0, 1);
    } else if (below->here
        && below->here->is_rollable
        && obj->is_rollable)
    {
        LevelCell *left = 0, *left_below = 0;
        LevelCell *right = 0, *right_below = 0;
        if (x > 0) {
            get_fall_channel(x-1, y, &left, &left_below);
        }
        if (x < _width-1) {
            get_fall_channel(x+1, y, &right, &right_below);
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
            obj->movement = new MovementRoll(cell, selected, selected_below, xoffset, 1);
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

CoordPair Level::get_physics_coords(const double x, const double y)
{
    CoordPair result;
    result.x = (CoordInt)(x * subdivision_count);// - subdivision_count / 2;
    result.y = (CoordInt)(y * subdivision_count);// - subdivision_count / 2;
    return result;
}

void Level::cleanup_cell(LevelCell *cell)
{
    GameObject *const obj = cell->here;
    if (obj)
    {
        _physics.clear_cells(obj->phy.x, obj->phy.y, obj->stamp);
        delete obj;
    }
}

TestObject *Level::debug_place_object(const CoordInt x, const CoordInt y)
{
    LevelCell *cell = &_cells[y*_width + x];
    if (!cell->here && !cell->reserved_by) {
        TestObject *const obj = new TestObject();
        cell->here = obj;
        obj->x = x;
        obj->y = y;
        obj->phy = get_physics_coords(x, y);
        _physics.wait_for();
        _physics.place_object(obj->phy.x, obj->phy.y, obj, 5);
        return obj;
    }
    return 0;
}

void Level::debug_test_object()
{
    // const CoordInt x = ((double)rand() / RAND_MAX) * (_width - 1);
    // const CoordInt y = ((double)rand() / RAND_MAX) * (_height - 1);
    debug_place_object(WALL_CENTER_X, 0);
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

        const double tc = (meta->blocked ? meta->obj->temp_coefficient : airtempcoeff_per_pressure * cell->air_pressure);

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

void Level::physics_to_gl_texture(bool thread_regions)
{
    _physics.to_gl_texture(0.0, 2.0, thread_regions);
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

            if (!handle_ca_interaction(x, y, cell, obj)) {
                cleanup_cell(cell);
                continue;
            }

            Movement *movement = obj->movement;
            if (movement) {
                if (movement->update(_time_slice)) {
                    movement = 0;
                }
                CoordPair new_coords = get_physics_coords(obj->x, obj->y);
                if (new_coords != obj->phy) {
                    if (obj->stamp && obj->stamp->non_empty()) {
                        /*_physics.clear_cells(obj->phy.x, obj->phy.y, obj->stamp);
                        _physics.place_object(new_coords.x, new_coords.y, obj);*/
                        const CoordPair vel = CoordPair(0, 1);
                        _physics.move_stamp(
                            obj->phy.x, obj->phy.y,
                            new_coords.x, new_coords.y,
                            obj->stamp,
                            &vel
                        );
                    }
                    obj->phy = new_coords;
                }
            }

            if (obj->is_gravity_affected && !movement) {
                if (!handle_gravity(x, y, cell, obj)) {
                    cleanup_cell(cell);
                    continue;
                }
            }
        }
    }

    /*static const double origins[4][2] = {
        {50.0, 50.0},
        {50.0, 50.0},
        {50.0, 50.0},
        {50.0, 50.0}
    };
    static const double phase_freq[4][2] = {
        {0.4, 2.0},
        {0.45, 2.0},
        {0.5, -1.5},
        {0.55, -1.5},
    };
    for (int i = 0; i < 4; i++) {
        const double cx = origins[i][0], cy = origins[i][1];
        const double phase = phase_freq[i][0], freq = phase_freq[i][1];

        const double r = sin(_time * 5.0) * 1.0 + 10.0;
        const double y = cy + sin(_time * freq + phase) * r;
        const double x = cx + cos(_time * freq + phase) * r;

        debug_test_stamp(x, y, false);
    }

    _time += _time_slice;

    for (int i = 0; i < 4; i++) {
        const double cx = origins[i][0], cy = origins[i][1];
        const double phase = phase_freq[i][0], freq = phase_freq[i][1];

        const double r = sin(_time * 5.0) * 1.0 + 10.0;
        const double y = cy + sin(_time * freq + phase) * r;
        const double x = cx + cos(_time * freq + phase) * r;

        debug_test_stamp(x, y, true);
    }*/

    /*const double y0 = 10;
    for (int i = 0; i < 80; i++) {
        const double x = 50 + sin(_time * 2.0) * 10.0;
        const double y = y0 + i * 0.5;

        debug_test_stamp(x, y, false);
    }

    _time += _time_slice;

    for (int i = 0; i < 80; i++) {
        const double x = 50 + sin(_time * 2.0) * 10.0;
        const double y = y0 + i * 0.5;

        debug_test_stamp(x, y, true);
    }*/

    _time += _time_slice;
    // debug_test_heat_stamp((sin(_time) + 1.0));
    // debug_test_heat_source();

    _physics.resume();
}
