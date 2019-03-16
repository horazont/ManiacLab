#include "stamp.hpp"

#include <iostream>
#include <stdexcept>

#include <cstdlib>
#include <cstring>

/* CellStamp */

CellStamp::CellStamp():
    data()
{
    reset();
}

CellStamp::CellStamp(const CellStampRaw &ref):
    data()
{
    memcpy(data, ref, sizeof(data));
}

CellStamp::CellStamp(const std::initializer_list<bool> &blocking_ref):
    data()
{
    reset();
    unsigned int offs = 0;
    for (auto &value: blocking_ref) {
        if (offs >= cell_stamp_length) {
            throw std::out_of_range("Too many arguments in initializer_"
                                    "list for CellStamp()");
        }
        if (value) {
            data[offs].type = CELL_BLOCK;
        }
        offs++;
    }
}

CellStamp::CellStamp(const CellStamp &ref):
    data()
{
    memcpy(data, ref.data, sizeof(data));
}

CellStamp &CellStamp::operator=(const CellStamp &ref)
{
    memcpy(data, ref.data, sizeof(data));
    return *this;
}

CellStamp::~CellStamp()
{

}

void CellStamp::reset()
{
    memset(data, 0, sizeof(data));
}

/* Stamp */

Stamp::Stamp(const CellStamp &stamp):
    _map_coords(0),
    _border(0),
    _map_coords_len(0),
    _border_len(0),
    _map()
{
    for (unsigned int i = 0; i < cell_stamp_length; i++) {
        _map[i] = (stamp.data[i].type == CELL_BLOCK);
    }
    generate_map_coords();
    find_border();
}

Stamp::Stamp(const Stamp &ref)
{
    memcpy(_map, ref._map, sizeof(_map));

    _map_coords_len = ref._map_coords_len;
    const uintptr_t map_coords_size = _map_coords_len * sizeof(CoordPair);
    _map_coords = (CoordPair*)malloc(map_coords_size);
    memcpy(_map_coords, ref._map_coords, map_coords_size);

    _border_len = ref._border_len;
    const uintptr_t border_size = _border_len * sizeof(CoordPair);
    _border = (CoordPair*)malloc(border_size);
    memcpy(_border, ref._border, border_size);
}

Stamp::~Stamp()
{
    if (_border) {
        free(_border);
    }
    if (_map_coords) {
        free(_map_coords);
    }
}

void Stamp::generate_map_coords()
{
    const unsigned int max_cells = cell_stamp_length;

    unsigned int count = 0;

    _map_coords = (CoordPair*)malloc(sizeof(CoordPair) * max_cells);
    bool *map_ptr = &_map[0];
    for (int y = 0; y < subdivision_count; y++) {
        for (int x = 0; x < subdivision_count; x++) {
            if (*map_ptr) {
                _map_coords[count].x = x;
                _map_coords[count].y = y;
                count++;
            }
            map_ptr++;
        }
    }
    _map_coords = (CoordPair*)realloc(_map_coords, sizeof(CoordPair) * count);

    _map_coords_len = count;
}

void Stamp::find_border()
{
    const unsigned int border_edge_length = (subdivision_count+2);
    const unsigned int max_border_cells = border_edge_length * border_edge_length;

    unsigned int count = 0;

    _border = (CoordPair*)malloc(sizeof(CoordPair) * max_border_cells);
    for (int y = -1; y <= subdivision_count; y++) {
        for (int x = -1; x <= subdivision_count; x++) {
            const bool validX = (x >= 0) && (x < subdivision_count);
            const bool validY = (y >= 0) && (y < subdivision_count);
            const bool *const above = ((y > 0 && validX)
                                        ? &_map[(y-1)*subdivision_count+x]
                                        : 0);
            const bool *const below = ((y < subdivision_count -1 && validX)
                                        ? &_map[(y+1)*subdivision_count+x]
                                        : 0);
            const bool *const left  = ((x > 0 && validY)
                                        ? &_map[y*subdivision_count+(x-1)]
                                        : 0);
            const bool *const right = ((x < subdivision_count - 1 && validY)
                                        ? &_map[y*subdivision_count+(x+1)]
                                        : 0);
            const bool *const at    = ((validX && validY)
                                        ? &_map[y*subdivision_count+x]
                                        : 0);
            const bool is_border = ((above && *above)
                                    || (below && *below)
                                    || (left && *left)
                                    || (right && *right))
                                  && (!(at && *at));
            if (is_border) {
                _border[count].x = x;
                _border[count].y = y;
                count++;
            }
        }
    }

    _border = (CoordPair*)realloc(_border, sizeof(CoordPair) * count);
    _border_len = count;
}
