#include "Stamp.hpp"

#include <iostream>

#include <cstdlib>
#include <cstring>

/* Stamp */

Stamp::Stamp(const BoolCellStamp stamp):
    _mapCoords(0),
    _border(0),
    _mapCoordsLen(0),
    _borderLen(0)
{
    memcpy(_map, stamp, sizeof(BoolCellStamp));
    generateMapCoords();
    findBorder();
}

Stamp::Stamp(const Stamp &ref)
{
    memcpy(_map, ref._map, sizeof(BoolCellStamp));

    _mapCoordsLen = ref._mapCoordsLen;
    const uintptr_t mapCoordsSize = _mapCoordsLen * sizeof(CoordPair);
    _mapCoords = (CoordPair*)malloc(mapCoordsSize);
    memcpy(_mapCoords, ref._mapCoords, mapCoordsSize);

    _borderLen = ref._borderLen;
    const uintptr_t borderSize = _borderLen * sizeof(CoordPair);
    _border = (CoordPair*)malloc(borderSize);
    memcpy(_border, ref._border, borderSize);
}

Stamp::~Stamp()
{
    if (_border) {
        free(_border);
    }
    if (_mapCoords) {
        free(_mapCoords);
    }
}

void Stamp::generateMapCoords()
{
    const unsigned int maxCells = cellStampLength;

    unsigned int count = 0;

    _mapCoords = (CoordPair*)malloc(sizeof(CoordPair) * maxCells);
    bool *mapPtr = &_map[0];
    for (int y = 0; y < subdivisionCount; y++) {
        for (int x = 0; x < subdivisionCount; x++) {
            if (*mapPtr) {
                _mapCoords[count].x = x;
                _mapCoords[count].y = y;
                count++;
            }
            mapPtr++;
        }
        std::cout << std::endl;
    }
    _mapCoords = (CoordPair*)realloc(_mapCoords, sizeof(CoordPair) * count);

    _mapCoordsLen = count;
}

void Stamp::findBorder()
{
    const unsigned int borderEdgeLength = (subdivisionCount+2);
    const unsigned int maxBorderCells = borderEdgeLength * borderEdgeLength;

    unsigned int count = 0;

    _border = (CoordPair*)malloc(sizeof(CoordPair) * maxBorderCells);
    for (int y = -1; y <= subdivisionCount; y++) {
        for (int x = -1; x <= subdivisionCount; x++) {
            const bool validX = (x >= 0) && (x < subdivisionCount);
            const bool validY = (y >= 0) && (y < subdivisionCount);
            const bool *const above = ((y > 0 && validX)
                                        ? &_map[(y-1)*subdivisionCount+x]
                                        : 0);
            const bool *const below = ((y < subdivisionCount -1 && validX)
                                        ? &_map[(y+1)*subdivisionCount+x]
                                        : 0);
            const bool *const left  = ((x > 0 && validY)
                                        ? &_map[y*subdivisionCount+(x-1)]
                                        : 0);
            const bool *const right = ((x < subdivisionCount - 1 && validY)
                                        ? &_map[y*subdivisionCount+(x+1)]
                                        : 0);
            const bool *const at    = ((validX && validY)
                                        ? &_map[y*subdivisionCount+x]
                                        : 0);
            const bool isBorder = ((above && *above)
                                    || (below && *below)
                                    || (left && *left)
                                    || (right && *right))
                                  && (!(at && *at));
            if (isBorder) {
                _border[count].x = x;
                _border[count].y = y;
                count++;
            }
        }
    }

    _border = (CoordPair*)realloc(_border, sizeof(CoordPair) * count);
    _borderLen = count;
}
