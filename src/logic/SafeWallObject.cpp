#include "SafeWallObject.hpp"

static const CellStamp safewall_object_stamp(
    {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
    }
);

static ObjectInfo safewall_object_info(
    true,
    true,
    false,
    false,
    false,
    false,
    false,
    0.0,
    1.0,
    safewall_object_stamp);

/* SafeWallView */

SafeWallView::SafeWallView(TileMaterialManager &matman):
    MetatextureView(matman.require_material(mat_safewall_standalone))
{

}

/* SafeWallObject */

SafeWallObject::SafeWallObject(Level *level):
    GameObject(safewall_object_info, level)
{

}

void SafeWallObject::setup_view(TileMaterialManager &matman)
{
    view = std::unique_ptr<ObjectView>(new SafeWallView(matman));
}
