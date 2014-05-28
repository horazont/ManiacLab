/**********************************************************************
File name: GameObject.cpp
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
#include "GameObject.hpp"

#include "Errors.hpp"
#include "Physics.hpp"

/* FrameState */

void FrameState::reset()
{
    explode = false;
    ignite = false;
    own_temperature = NAN;
    surr_temperature = NAN;
}

/* ObjectInfo */

ObjectInfo::ObjectInfo(
        bool is_blocking_,
        bool is_destructible_,
        bool is_edible_,
        bool is_gravity_affected_,
        bool is_movable_,
        bool is_rollable_,
        bool is_sticky_,
        float roll_radius_,
        float temp_coefficient_,
        const CellStamp &stamp):
    TileData(),
    stamp(stamp)
{
    is_blocking = is_blocking_;
    is_destructible = is_destructible_;
    is_edible = is_edible_;
    is_gravity_affected = is_gravity_affected_;
    is_movable = is_movable_;
    is_rollable = is_rollable_;
    is_sticky = is_sticky_;
    roll_radius = roll_radius_;
    temp_coefficient = temp_coefficient_;
}

ObjectInfo::ObjectInfo(const CellStamp &stamp):
    TileData(),
    stamp(stamp)
{
    TileData::stamp = stamp;
}

ObjectInfo::ObjectInfo(const TileData &src):
    TileData(src),
    stamp(src.stamp)
{

}

/* ObjectView */

ObjectView::ObjectView():
    _invalidated(false)
{

}

ObjectView::~ObjectView()
{

}

void ObjectView::invalidate()
{
    _invalidated = true;
}

void ObjectView::update(
    GameObject &object,
    PyEngine::TimeFloat deltaT)
{

}

/* ViewableObject */

ViewableObject::ViewableObject():
    _view(nullptr)
{

}

ViewableObject::~ViewableObject()
{

}

std::unique_ptr<ObjectView> ViewableObject::setup_view(
    TileMaterialManager &matman)
{
    return nullptr;
}

ObjectView *ViewableObject::get_view(TileMaterialManager &matman)
{
    if (!_view) {
        _view = setup_view(matman);
    }
    return _view.get();
}

/* GameObject */

GameObject::GameObject(const ObjectInfo &info,
                       Level *level):
    ViewableObject(),
    level(level),
    frame_state(),
    info(info),
    x(0),
    y(0),
    phi(0),
    movement(nullptr),
    phy(),
    ticks(0)
{

}

void GameObject::destruct_self()
{
    // cleanup_cell calls delete
    level->cleanup_cell(level->get_cell(cell.x, cell.y));
}

bool GameObject::handle_gravity()
{
    if (cell.y == level->get_height() - 1) {
        return true;
    }

    assert(!movement);

    LevelCell *my_cell = level->get_cell(cell.x, cell.y);
    LevelCell *below = level->get_cell(cell.x, cell.y+1);
    if (!below->here && !below->reserved_by) {
        movement = std::unique_ptr<Movement>(
            new MovementStraight(my_cell, below, 0, 1));
        return true;
    }

    if (info.is_rollable && below->here && below->here->info.is_rollable)
    {
        LevelCell *left = 0, *left_below = 0;
        LevelCell *right = 0, *right_below = 0;
        if (cell.x > 0) {
            level->get_fall_channel(cell.x-1, cell.y, left, left_below);
        }
        if (cell.x < level->get_width() - 1) {
            level->get_fall_channel(cell.x+1, cell.y, right, right_below);
        }

        if (left && right) {
            if ((float)rand() / RAND_MAX >= 0.5) {
                left = nullptr;
            } else {
                right = nullptr;
            }
        }

        LevelCell *selected = 0, *selected_below = 0;
        CoordInt xoffset = 0;
        if (left) {
            selected = left;
            selected_below = left_below;
            xoffset = -1;
        } else {
            selected = right;
            selected_below = right_below;
            xoffset = 1;
        }

        if (selected) {
            movement = std::unique_ptr<Movement>(
                new MovementRoll(
                    my_cell,
                    selected,
                    selected_below,
                    xoffset, 1));
        }
        return true;
    }

    return true;
}

bool GameObject::after_movement(Movement *prev_movement)
{
    if (!info.is_gravity_affected) {
        return true;
    }

    if (cell.y < level->get_height() - 1
        && prev_movement->offset_y > 0)
    {
        LevelCell *const cell = level->get_cell(this->cell.x,
                                                this->cell.y+1);
        GameObject *below = cell->here;
        if (!below) {
            return true;
        }

        if (!impact(below)) {
            return false;
        }

        below = cell->here;
        if (below) {
            below->headache(this);
        }
    }

    return true;
}

void GameObject::before_movement(Movement *movement)
{

}

void GameObject::explosion_touch()
{
    if (info.is_destructible) {
        destruct_self();
    }
}

void GameObject::headache(GameObject *from_object)
{

}

bool GameObject::idle()
{
    if (movement) {
        return true;
    }

    if (info.is_gravity_affected && cell.y < level->get_height() - 1)
    {
        if (!handle_gravity()) {
            return false;
        }
    }

    return true;
}

void GameObject::ignition_touch()
{

}

bool GameObject::impact(GameObject *on_object)
{
    return true;
}

bool GameObject::move(MoveDirection dir, bool chain_move)
{
    if (!info.is_movable || movement || dir == NONE) {
        return false;
    }

    CoordInt offsx = 0;
    CoordInt offsy = 0;
    switch (dir) {
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
    default: {
        return false;
    }
    }

    const CoordInt neighx = offsx + x;
    const CoordInt neighy = offsy + y;

    if ((offsx != 0 || offsy != 0)
        && neighx >= 0 && neighx < level->get_width()
        && neighy >= 0 && neighy < level->get_height())
    {
        LevelCell *neighbour = level->get_cell(neighx, neighy);
        if (!neighbour->reserved_by
            && (!neighbour->here || (chain_move
                                     && neighbour->here->move(dir, false))))
        {
            movement = std::unique_ptr<Movement>(
                new MovementStraight(
                    level->get_cell(cell.x, cell.y),
                    neighbour,
                    offsx, offsy));
            return true;
        }
    }

    return false;
}

bool GameObject::projectile_impact()
{
    return false;
}

void GameObject::update()
{
    if (ticks == level->get_ticks()) {
        return;
    }
    ticks = level->get_ticks();

    if (movement) {
        if (!movement->update()) {
            return;
        }
    }

    CoordPair new_coords = level->get_physics_coords(x, y);
    if (new_coords != phy) {
        if (info.stamp.non_empty()) {
            const CoordPair vel{
                new_coords.x - phy.x,
                new_coords.y - phy.y};
            level->physics().move_stamp(
                phy.x, phy.y,
                new_coords.x, new_coords.y,
                info.stamp,
                &vel);
        }
        phy = new_coords;
    }

    if (!movement) {
        idle();
    }
}
