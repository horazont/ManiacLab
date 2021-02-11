#include "player_object.hpp"

#include <ffengine/io/log.hpp>

#include "level.hpp"

static io::Logger &logger = io::logging().get_logger("maniaclab.player");

static const CellStamp player_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);


static ObjectInfo player_object_info(
    true,
    true,
    false,
    false,
    true,
    false,  /* FIXME: should be round, but not rolling when moving ^_^ */
    false,
    0.5,
    player_object_stamp);

PlayerController::PlayerController():
    action_request(AR_NONE)
{

}


PlayerObject::PlayerObject(Level &level):
    GameObject(player_object_info, level, 1.f)
{

}

bool PlayerObject::try_collect(LevelCell &target)
{
    if (target.here && target.reserved_by) {
        // FIXME?
        return false;
    }

    GameObject *obj = target.here.get();
    if (!obj) {
        obj = target.reserved_by;
    }

    if (!obj) {
        return true;
    }

    if (obj && obj->info.is_collectable) {
        // TODO: handle collection
        level.cleanup_cell(&target);
        return true;
    }

    return false;
}

bool PlayerObject::idle()
{
    switch (m_controller.action_request) {
    case PlayerController::AR_NONE:
    {
        break;
    }
    case PlayerController::AR_MOVE_UP:
    {
        if (cell.y <= 1) {
            break;
        }
        LevelCell *const my_cell = level.get_cell(cell.x, cell.y);
        LevelCell *const above = level.get_cell(cell.x, cell.y-1);
        if (try_collect(*above)) {
            movement = std::make_unique<MovementStraight>(my_cell, above, 0, -1);
            /* we orient up/down based on the current orientation */
            const bool was_straight = phi >= -1e-2 && phi <= 1e-2;
            const bool was_up = (flip && phi < 0.f) || (!flip && phi > 0.f);
            bool orient_right =
                    (flip && was_straight) ||
                    (was_up && flip) ||
                    (!was_up && !flip);
            if (orient_right) {
                // we were moving right before
                flip = true;
                phi = -M_PI_2f32;
            } else {
                flip = false;
                phi = M_PI_2f32;
            }
        }
        break;
    }
    case PlayerController::AR_MOVE_DOWN:
    {
        if (cell.y >= level_height - 1) {
            break;
        }
        LevelCell *const my_cell = level.get_cell(cell.x, cell.y);
        LevelCell *const below = level.get_cell(cell.x, cell.y+1);
        if (try_collect(*below)) {
            movement = std::make_unique<MovementStraight>(my_cell, below, 0, 1);
            const bool was_straight = phi >= -1e-2 && phi <= 1e-2;
            const bool was_down = (flip && phi > 0.f) || (!flip && phi < 0.f);
            bool orient_right =
                    (was_straight && flip) ||
                    (was_down && flip) ||
                    (!was_down && !flip);
            if (orient_right) {
                flip = true;
                phi = M_PI_2f32;
            } else {
                flip = false;
                phi = -M_PI_2f32;
            }
        }
        break;
    }
    case PlayerController::AR_MOVE_LEFT:
    {
        if (cell.x <= 1) {
            break;
        }
        LevelCell *const my_cell = level.get_cell(cell.x, cell.y);
        LevelCell *const left = level.get_cell(cell.x-1, cell.y);
        if (try_collect(*left)) {
            movement = std::make_unique<MovementStraight>(my_cell, left, -1, 0);
            flip = false;
            phi = 0.f;
            break;
        }

        if (cell.x <= 2) {
            break;
        }

        LevelCell *const next_left = level.get_cell(cell.x-2, cell.y);
        if (!next_left->here && !next_left->reserved_by &&
                left->here && !left->here->movement &&
                left->here->info.is_movable)
        {
            /* we need to take a reference to the object to the left, because
             * left->here is written to by the construction of MovementStraight
             */
            GameObject &left_object = *left->here;
            left_object.movement = std::make_unique<MovementStraight>(left, next_left, -1, 0);
            movement = std::make_unique<MovementStraight>(my_cell, left, -1, 0);
            flip = false;
            phi = 0.f;
            break;
        }
        break;
    }
    case PlayerController::AR_MOVE_RIGHT:
    {
        if (cell.x >= level_width - 1) {
            break;
        }
        LevelCell *const my_cell = level.get_cell(cell.x, cell.y);
        LevelCell *const right = level.get_cell(cell.x+1, cell.y);
        if (try_collect(*right)) {
            movement = std::make_unique<MovementStraight>(my_cell, right, 1, 0);
            flip = true;
            phi = 0.f;
            break;
        }

        if (cell.x >= level_width - 2) {
            break;
        }

        LevelCell *const next_right = level.get_cell(cell.x+2, cell.y);
        if (!next_right->here && !next_right->reserved_by &&
                right->here && !right->here->movement &&
                right->here->info.is_movable)
        {
            /* we need to take a reference to the object to the left, because
             * right->here is written to by the construction of MovementStraight
             */
            GameObject &right_object = *right->here;
            right_object.movement = std::make_unique<MovementStraight>(right, next_right, 1, 0);
            movement = std::make_unique<MovementStraight>(my_cell, right, 1, 0);
            flip = true;
            phi = 0.f;
            break;
        }
        break;
    }
    default:
    {
        logger.logf(io::LOG_WARNING, "unhandled action request %d",
                    m_controller.action_request);
        break;
    }
    }

    return true;
}
