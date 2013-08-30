/**********************************************************************
File name: TestObject.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "TestObject.hpp"

#include <cstdlib>

#include "Stamp.hpp"

/* TestObject */

TestObject::TestObject():
    GameObject()
{
    static const CellStamp bool_stamp(
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
    );
    stamp = new Stamp(bool_stamp);
    is_gravity_affected = true;
    radius = 0.5;
}


void TestObject::set_is_affected_by_gravity(bool flag) {
    is_gravity_affected = flag;
}
