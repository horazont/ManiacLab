#include "game_object.hpp"

#include <cassert>
#include <iostream>

#include <ffengine/io/log.hpp>

#include "physics.hpp"

static io::Logger &logger = io::logging().get_logger("maniaclab.game_object");

/* FrameState */

void FrameState::reset()
{
    idle = false;
    explode = false;
    ignite = false;
}

/* ObjectInfo */

ObjectInfo::ObjectInfo(
        bool is_blocking,
        bool is_destructible,
        bool is_collectable,
        bool is_gravity_affected,
        bool is_movable,
        bool is_round,
        bool is_sticky,
        float roll_radius,
        const CellStamp &stamp):
    is_blocking(is_blocking),
    is_destructible(is_destructible),
    is_collectable(is_collectable),
    is_gravity_affected(is_gravity_affected),
    is_movable(is_movable),
    is_round(is_round),
    is_sticky(is_sticky),
    roll_radius(roll_radius),
    stamp(stamp)
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

void ObjectView::update(GameObject&, ffe::TimeInterval)
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

std::unique_ptr<ObjectView> ViewableObject::setup_view(TileMaterialManager&)
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
                       Level &level, const SimFloat heat_capacity):
    ViewableObject(),
    level(level),
    frame_state(),
    info(info),
    x(0),
    y(0),
    phi(0),
    movement(nullptr),
    phy(),
    heat_capacity(heat_capacity),
    ticks(0)
{

}

void GameObject::destruct_self()
{
    // cleanup_cell calls delete
    level.cleanup_cell(level.get_cell(cell.x, cell.y));
}

bool GameObject::handle_gravity()
{
    if (cell.y == level.get_height() - 1) {
        return true;
    }

    assert(!movement);

    LevelCell *my_cell = level.get_cell(cell.x, cell.y);
    LevelCell *below = level.get_cell(cell.x, cell.y+1);
    if (!below->here && !below->reserved_by) {
        movement = std::unique_ptr<Movement>(
            new MovementStraight(my_cell, below, 0, 1));
        return true;
    }

    if (info.is_round && below->here && below->here->info.is_round)
    {
        LevelCell *left = nullptr, *left_below = nullptr;
        LevelCell *right = nullptr, *right_below = nullptr;
        if (cell.x > 0) {
            level.get_fall_channel(cell.x-1, cell.y, left, left_below);
        }
        if (cell.x < level.get_width() - 1) {
            level.get_fall_channel(cell.x+1, cell.y, right, right_below);
        }

        if (left && right) {
            if ((float)rand() / RAND_MAX >= 0.5f) {
                left = nullptr;
            } else {
                right = nullptr;
            }
        }

        LevelCell *selected = nullptr, *selected_below = nullptr;
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
    // falling/downward movement while gravity affected
    if (prev_movement->offset_y > 0 && info.is_gravity_affected) {
        if (cell.y < level.get_height() - 1)
        {
            // moving within normal space
            LevelCell *const below_cell = level.get_cell(
                        this->cell.x,
                        this->cell.y+1);
            GameObject *below = below_cell->here.get();
            if (!below) {
                return true;
            }

            if (!impact(below)) {
                return false;
            }

            below = below_cell->here.get();
            if (below) {
                below->headache(this);
            }
        } else {
            // hit the boundaries of the level
            if (!impact(nullptr)) {
                return false;
            }
        }
    }

    return true;
}

void GameObject::before_movement(Movement*)
{

}

void GameObject::explosion_touch()
{
    frame_state.explode = true;
    if (info.is_destructible) {
        destruct_self();
    }
}

void GameObject::headache(GameObject*)
{

}

bool GameObject::idle()
{
    if (movement) {
        return true;
    }

    if (info.is_gravity_affected)
    {
        if (cell.y < level.get_height())
        {
            if (!handle_gravity()) {
                return false;
            }
        } else {
            // TODO: destroy
        }
    }

    return true;
}

void GameObject::ignition_touch()
{
    frame_state.ignite = true;
}

bool GameObject::impact(GameObject*)
{
    return true;
}

bool GameObject::move(MoveDirection dir, bool chain_move)
{
    if (!info.is_movable || movement) {
        return false;
    }

    const CoordPair offs = move_direction_to_vector(dir);
    const CoordInt neighx = static_cast<CoordInt>(offs.x + x);
    const CoordInt neighy = static_cast<CoordInt>(offs.y + y);

    if ((offs.x != 0 || offs.y != 0)
        && neighx >= 0 && neighx < level.get_width()
        && neighy >= 0 && neighy < level.get_height())
    {
        LevelCell *neighbour = level.get_cell(neighx, neighy);
        if (!neighbour->reserved_by
            && (!neighbour->here || (chain_move
                                     && neighbour->here->move(dir, false))))
        {
            movement = std::unique_ptr<Movement>(
                new MovementStraight(
                    level.get_cell(cell.x, cell.y),
                    neighbour,
                    offs.x, offs.y));
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
    if (ticks == level.get_ticks()) {
        return;
    }
    ticks = level.get_ticks();

    if (movement) {
        if (!movement->update()) {
            return;
        }
    }

    CoordPair new_coords = level.get_physics_coords(x, y);
    if (new_coords != phy) {
        if (info.stamp.non_empty()) {
            const CoordPair vel{
                new_coords.x - phy.x,
                new_coords.y - phy.y};
            level.physics().move_stamp(
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

    frame_state.reset();
}

/* free functions */

CoordPair move_direction_to_vector(MoveDirection dir)
{
    switch (dir)
    {
    case MOVE_UP:
    {
        return CoordPair{0, -1};
    }
    case MOVE_DOWN:
    {
        return CoordPair{0, 1};
    }
    case MOVE_LEFT:
    {
        return CoordPair{-1, 0};
    }
    case MOVE_RIGHT:
    {
        return CoordPair{1, 0};
    }
    }

    return CoordPair();
}
