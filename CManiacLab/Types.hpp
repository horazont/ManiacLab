/**********************************************************************
File name: Types.hpp
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
#ifndef _ML_TYPES_H
#define _ML_TYPES_H

#include <CEngine/Misc/Int.hpp>

typedef int CoordInt;

struct CoordPair {
    CoordPair(): x(-1), y(-1) {};
    CoordPair(const CoordInt x, const CoordInt y): x(x), y(y) {};
    CoordPair(const CoordPair &ref): x(ref.x), y(ref.y) {};

    CoordPair& operator =(const CoordPair &ref)
    {
        x = ref.x;
        y = ref.y;
        return *this;
    }

    inline bool operator != (const CoordPair &oth)
    {
        return (x != oth.x) || (y != oth.y);
    }

    inline bool operator == (const CoordPair &oth)
    {
        return (x == oth.x) && (y == oth.y);
    }

    CoordInt x, y;
};

#endif
