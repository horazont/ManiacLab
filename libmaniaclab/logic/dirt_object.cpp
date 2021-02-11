#include "dirt_object.hpp"

static const CellStamp dirt_object_stamp(
    {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
    }
);

const ObjectInfo rock_object_info(
    true,
    true,
    true,
    false,
    false,
    false,
    false,
    0.5,
    dirt_object_stamp);

DirtObject::DirtObject(Level &level):
    GameObject(rock_object_info, level, 1.f)
{

}
