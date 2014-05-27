#include "BombObject.hpp"

static const CellStamp bomb_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static ObjectInfo bomb_object_info(
    true,
    true,
    false,
    true,
    true,
    true,
    false,
    0.5,
    1.0,
    bomb_object_stamp);

/* BombView */

BombView::BombView(TileMaterialManager &matman):
    MetatextureView(matman.require_material("bomb"))
{

}

/* BombObject */

BombObject::BombObject(Level *level):
    GameObject(bomb_object_info, level)
{

}

void BombObject::headache(GameObject *from_object)
{
    explode();
}

void BombObject::explode()
{
    level->add_large_explosion(cell.x, cell.y, 1, 1);
}

bool BombObject::impact(GameObject *on_object)
{
    explode();
    return true;
}

void BombObject::setup_view(TileMaterialManager &matman)
{
    view = std::unique_ptr<ObjectView>(
        new BombView(matman));
}
