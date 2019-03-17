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

#include <cassert>
#include <cmath>
#include <cstdint>
#include <ostream>

typedef int32_t CoordInt;

typedef float SimFloat;

static inline uint32_t coord_int_to_unsigned(CoordInt i) {
    assert(i >= 0);
    return static_cast<uint32_t>(i);
}

typedef uint32_t TickCounter;

struct CoordPair {
    CoordPair(): x(-1), y(-1) {};
    CoordPair(const CoordInt x, const CoordInt y): x(x), y(y) {};
    CoordPair(const CoordPair &ref) = default;
    CoordPair& operator=(const CoordPair &ref) = default;

    inline bool operator != (const CoordPair &oth) const
    {
        return (x != oth.x) || (y != oth.y);
    }

    inline bool operator == (const CoordPair &oth) const
    {
        return (x == oth.x) && (y == oth.y);
    }

    inline CoordInt operator * (const CoordPair &oth) const
    {
        return (x * oth.x) + (y * oth.y);
    }

    inline double norm() const
    {
        return sqrt(x*x + y*y);
    }

    inline double normed_float_dotp(const double oth_x, const double oth_y) const
    {
        const double my_norm = norm();
        if (my_norm == 0)
            return 0;
        return ((x * oth_x) + (y * oth_y)) / my_norm;
    }

    CoordInt x, y;
};

inline std::ostream& operator<<(std::ostream &dest, const CoordPair &cp) {
    return dest << "CoordPair{" << cp.x << ", " << cp.y << "}";
}

#endif
