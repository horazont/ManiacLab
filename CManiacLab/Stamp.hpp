/**********************************************************************
File name: Stamp.hpp
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
#ifndef _ML_STAMP_H
#define _ML_STAMP_H

#include "Types.hpp"
#include "PhysicsConfig.hpp"

typedef bool BoolCellStamp[cell_stamp_length];

struct Stamp {
public:
    Stamp(const BoolCellStamp stamp);
    Stamp(const Stamp& ref);
    Stamp& operator =(const Stamp& other);
    ~Stamp();
private:
    CoordPair *_map_coords, *_border;
    uintptr_t _map_coords_len, _border_len;
    BoolCellStamp _map;
private:
    /**
     * Extract the coordinates of all map cells with the value true. This is
     * useful for quick looping over these.
     */
    void generate_map_coords();

    /**
     * Use the map to detect the border of the map and store the coordinate
     * offsets of the border (relative to the top left field of the map)
     * fields in border.
     *
     * Allocates border and sets border_len. Does not deallocate border
     * beforehands. Supposed to be called in the constructor only.
     */
    void find_border();
public:
    inline const BoolCellStamp& get_map() const {
        return _map;
    }
    inline const CoordPair* get_map_coords(uintptr_t *map_coords_len) const {
        *map_coords_len = _map_coords_len;
        return _map_coords;
    }
    inline const CoordPair* get_border(uintptr_t *border_len) const {
        *border_len = _border_len;
        return _border;
    }
public:
    inline bool non_empty() const {
        return _map_coords_len != 0;
    }
};

#endif
