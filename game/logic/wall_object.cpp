#include "wall_object.hpp"

static const CellStamp squarewall_object_stamp(
    {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
    }
);

static const CellStamp roundwall_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static ObjectInfo safewallsq_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    true,
    0.0,
    1.0,
    squarewall_object_stamp);

static ObjectInfo safewallrd_object_info(
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    0.5,
    1.0,
    roundwall_object_stamp);

/* WallObject */

WallObject::WallObject(ObjectInfo &info, Level *level):
    GameObject(info, level)
{
}

/* SafeWallObject */

SafeWallObject::SafeWallObject(Level *level):
    WallObject(safewallsq_object_info, level)
{

}
