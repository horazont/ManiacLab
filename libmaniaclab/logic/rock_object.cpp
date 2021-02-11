#include "rock_object.hpp"


static const CellStamp rock_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

const ObjectInfo rock_object_info(
    true,
    true,
    false,
    true,
    true,
    true,
    false,
    0.5,
    rock_object_stamp);

RockObject::RockObject(Level &level):
    GameObject(rock_object_info, level, 1.f)
{

}
