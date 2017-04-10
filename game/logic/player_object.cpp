#include "PlayerObject.hpp"

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
    false,
    false,
    0.0,
    1.0,
    player_object_stamp);


/* PlayerView */

PlayerView::PlayerView(TileMaterialManager &matman):
    MetatextureView(matman.require_material(mat_player))
{

}

/* PlayerObject */

PlayerObject::PlayerObject(Level *level):
    GameObject(player_object_info, level),
    action(ACTION_NONE),
    move_direction(MOVE_LEFT),
    active_weapon(nullptr),
    flamethrower()
{

}

bool PlayerObject::idle()
{
    switch (action) {
    case ACTION_MOVE:
    {
        move(move_direction, true);
        return true;
    }
    case ACTION_FIRE_WEAPON:
    {
        if (active_weapon) {
            active_weapon->fire(level,
                                cell,
                                move_direction_to_vector(move_direction));
        }
        return true;
    }
    default:
    {
        return true;
    }
    }
    return true;
}

std::unique_ptr<ObjectView> PlayerObject::setup_view(
    TileMaterialManager &matman)
{
    return std::unique_ptr<ObjectView>(new PlayerView(matman));
}
