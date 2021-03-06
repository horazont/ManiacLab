#include "RockObject.hpp"

static const CellStamp rock_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static ObjectInfo rock_object_info(
    true,
    true,
    false,
    true,
    true,
    true,
    false,
    0.5,
    1.0,
    rock_object_stamp);

/* RockView */

RockView::RockView(TileMaterialManager &matman):
    MetatextureView(matman.require_material(mat_rock))
{

}

/* RockObject */

RockObject::RockObject(Level *level):
    GameObject(rock_object_info, level)
{

}

std::unique_ptr<ObjectView> RockObject::setup_view(TileMaterialManager &matman)
{
    return std::unique_ptr<ObjectView>(new RockView(matman));
}
