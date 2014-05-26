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
    false,
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
    if (acting == NONE || movement) {
        return true;
    }

    CoordInt offsx = 0;
    CoordInt offsy = 0;
    switch (acting) {
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

    acting = NONE;

    const CoordInt neighx = offsx + x;
    const CoordInt neighy = offsy + y;

    if ((offsx != 0 || offsy != 0)
        && neighx >= 0 && neighx < level->get_width()
        && neighy >= 0 && neighy < level->get_height())
    {
        LevelCell *neighbour = level->get_cell(neighx, neighy);
        movement = std::unique_ptr<Movement>(
            new MovementStraight(
                level->get_cell(cell.x, cell.y),
                neighbour,
                offsx, offsy));
    }

    return true;
}

void PlayerObject::setup_view(TileMaterialManager &matman)
{
    view = std::unique_ptr<ObjectView>(new PlayerView(matman));
}
