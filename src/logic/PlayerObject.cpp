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
    acting(NONE)
{

}

bool PlayerObject::idle()
{
    move(acting, true);
    acting = NONE;

    return true;
}

std::unique_ptr<ObjectView> PlayerObject::setup_view(
    TileMaterialManager &matman)
{
    return std::unique_ptr<ObjectView>(new PlayerView(matman));
}
