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

Stamp::Stamp(const CellStamp &stamp)
{
    for (unsigned int i = 0; i < cell_stamp_length; i++) {
        m_map[i] = (stamp.data[i].type == CELL_BLOCK);
    }
    generate_map_coords();
    find_border();
}

Stamp::Stamp(const std::initializer_list<bool> &blocking_ref)
{
    if (blocking_ref.size() != cell_stamp_length) {
        throw std::invalid_argument(
                    "initializer_list must be exactly " +
                    std::to_string(cell_stamp_length) +
                    " elements long");
    }

    unsigned int i = 0;
    for (bool flag: blocking_ref) {
        m_map[i++] = flag;
    }
}

void Stamp::generate_map_coords()
{
    m_map_coords.reserve(cell_stamp_length);
    bool *map_ptr = &m_map[0];
    for (int y = 0; y < subdivision_count; y++) {
        for (int x = 0; x < subdivision_count; x++) {
            if (*map_ptr) {
                m_map_coords.emplace_back(x, y);
            }
            map_ptr++;
        }
    }
    m_map_coords.shrink_to_fit();
}

void Stamp::find_border()
{
    const unsigned int border_edge_length = (subdivision_count+2);
    const unsigned int max_border_cells = border_edge_length * border_edge_length;

    m_border.reserve(max_border_cells);
    for (int y = -1; y <= subdivision_count; y++) {
        for (int x = -1; x <= subdivision_count; x++) {
            const bool validX = (x >= 0) && (x < subdivision_count);
            const bool validY = (y >= 0) && (y < subdivision_count);
            const bool *const above = ((y > 0 && validX)
                                        ? &m_map[(y-1)*subdivision_count+x]
                                        : nullptr);
            const bool *const below = ((y < subdivision_count -1 && validX)
                                        ? &m_map[(y+1)*subdivision_count+x]
                                        : nullptr);
            const bool *const left  = ((x > 0 && validY)
                                        ? &m_map[y*subdivision_count+(x-1)]
                                        : nullptr);
            const bool *const right = ((x < subdivision_count - 1 && validY)
                                        ? &m_map[y*subdivision_count+(x+1)]
                                        : nullptr);
            const bool *const at    = ((validX && validY)
                                        ? &m_map[y*subdivision_count+x]
                                        : nullptr);
            const bool is_border = ((above && *above)
                                    || (below && *below)
                                    || (left && *left)
                                    || (right && *right))
                                  && (!(at && *at));
            if (is_border) {
                m_border.emplace_back(x, y);
            }
        }
    }
    m_border.shrink_to_fit();
}
