#include "TestObject.hpp"

#include <cstdlib>

#include "Stamp.hpp"

/* TestObject */

TestObject::TestObject():
    GameObject()
{
    static const BoolCellStamp bool_stamp[1] = {
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
        /*{
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
            true, true, true, true, true,
        }*/
        {
            true, true, true, true, true,
            true, true, true, true, true,
            false, true, true, true, false,
            false, true, true, true, false,
            false, false, true, false, false,
        }
    };
    stamp = new Stamp(bool_stamp[0]);
    is_gravity_affected = true;
    radius = 0.5;
}


void TestObject::set_is_affected_by_gravity(bool flag) {
    is_gravity_affected = flag;
}
