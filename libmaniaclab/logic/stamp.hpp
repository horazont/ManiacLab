#ifndef _ML_STAMP_H
#define _ML_STAMP_H

#include <initializer_list>

#include "logic/types.hpp"
#include "logic/physicsconfig.hpp"

enum CellType {
    /* clear cell
     *
     * A clear cell does nothing and is skipped in all stamping.
     */
    CELL_CLEAR = 0,

    /* blocking cell
     *
     * A blocking cell is used to place physics blocking cell flags. The
     * temperature coefficient is taken from the object.
     */
    CELL_BLOCK = 1,

    /* source cell
     *
     * each game frame, a defined amount of material (defined in the
     * cell) is spilled into the game
     */
    CELL_SOURCE = 2,

    /* sink cell
     *
     * each game frame, a defined amount of material (defined in the
     * cell) is taken out of the game, if available.
     */
    CELL_SINK = 3,

    /* flow cell
     *
     * a flow cell manipulates the air flow to the adjacent cells.
     */
    CELL_FLOW = 4
};

enum SinkSourceType {
    SINK_SOURCE_AIR = 0,
    SINK_SOURCE_FOG = 1
};

struct CellTemplate {
    CellType type;

    /* used for sinks and sources */
    SinkSourceType sink_what;
    float amplitude;

    /* generic air flow in clear cells */
    float flow_north, flow_west;
};

typedef CellTemplate CellStampRaw[cell_stamp_length];

struct CellStamp {
public:
    CellStamp();
    CellStamp(const CellStampRaw &ref);
    CellStamp(const std::initializer_list<bool> &blocking_ref);
    CellStamp(const CellStamp &ref);
    CellStamp& operator=(const CellStamp &ref);
    ~CellStamp();

public:
    CellStampRaw data;

    void reset();

    inline CellTemplate &operator[](int index) {
        return data[index];
    };

    inline CellTemplate operator[](int index) const {
        return data[index];
    };

    inline CellTemplate &xy(int x, int y) {
        return data[x + y * subdivision_count];
    };

    inline CellTemplate get_xy(int x, int y) const {
        return data[x + y * subdivision_count];
    };

    inline bool get_blocking(int x, int y) const {
        return get_xy(x, y).type == CELL_BLOCK;
    };
};

struct Stamp {
public:
    explicit Stamp(const CellStamp &stamp);
    Stamp(const Stamp& ref);
    Stamp& operator =(const Stamp& other);
    ~Stamp();

private:
    CoordPair *_map_coords, *_border;
    uintptr_t _map_coords_len, _border_len;
    bool _map[cell_stamp_length];

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
    //~ inline const BoolCellStamp& get_map() const {
        //~ return _map;
    //~ }
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

    inline uintptr_t popcount() const {
        return _map_coords_len;
    }

};

#endif
