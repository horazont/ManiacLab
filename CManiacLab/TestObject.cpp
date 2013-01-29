#include "TestObject.hpp"

#include <cstdlib>

#include "Stamp.hpp"

/* TestObject */

TestObject::TestObject():
    GameObject()
{
    static const BoolCellStamp boolStamp[1] = {
        /*{
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
        }*/

        /*{
            false, false, true, false, false,
            false, true,  true,  true, false,
            true , true,  true,  true,  true,
            false, true,  true,  true, false,
            false, false, true, false, false,
        }*/
        {
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
        }
    };
    stamp = new Stamp(boolStamp[0]);
    isGravityAffected = true;
    radius = 0.5;
}


void TestObject::setIsAffectedByGravity(bool flag) {
    isGravityAffected = flag;
}
