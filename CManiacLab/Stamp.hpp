#ifndef _ML_STAMP_H
#define _ML_STAMP_H

#include "Types.hpp"
#include "PhysicsConfig.hpp"

typedef bool BoolCellStamp[cellStampLength];

struct Stamp {
public:
    Stamp(const BoolCellStamp stamp);
    Stamp(const Stamp& ref);
    Stamp& operator =(const Stamp& other);
    ~Stamp();
private:
    CoordPair *_mapCoords, *_border;
    uintptr_t _mapCoordsLen, _borderLen;
    BoolCellStamp _map;
private:
    /**
     * Extract the coordinates of all map cells with the value true. This is
     * useful for quick looping over these.
     */
    void generateMapCoords();

    /**
     * Use the map to detect the border of the map and store the coordinate
     * offsets of the border (relative to the top left field of the map)
     * fields in border.
     *
     * Allocates border and sets borderLen. Does not deallocate border
     * beforehands. Supposed to be called in the constructor only.
     */
    void findBorder();
public:
    inline const BoolCellStamp& getMap() const {
        return _map;
    }
    inline const CoordPair* getMapCoords(uintptr_t *mapCoordsLen) const {
        *mapCoordsLen = _mapCoordsLen;
        return _mapCoords;
    }
    inline const CoordPair* getBorder(uintptr_t *borderLen) const {
        *borderLen = _borderLen;
        return _border;
    }
};

#endif
