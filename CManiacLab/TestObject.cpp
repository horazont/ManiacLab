#include "TestObject.hpp"

#include <cstdlib>

#include "Stamp.hpp"

/* TestObject */

TestObject::TestObject():
    GameObject()
{
    static const BoolCellStamp boolStamp[5] = {
        {
            false, true, true, false,
            true, true, true, true,
            false, true, true, false,
            false, true, true, false
        },
        {
            false, true, true, false,
            true, true, true, true,
            true, true, true, true,
            false, true, true, false
        },
        {
            true, true, true, true,
            true, true, true, true,
            true, true, true, true,
            true, true, true, true
        },
        {
            false, true, true, false,
            false, true, true, true,
            false, true, true, false,
            false, true, true, true
        },
        {
            true, true, false, true,
            true, false, false, true,
            true, false, true, true,
            true, false, false, true
        }
    };
    stamp = new Stamp(boolStamp[(int)((double)rand() / RAND_MAX) * 5]);
    isGravityAffected = true;
    radius = 0.5;
}
