#include "Level.hpp"

#include <cmath>

#include "CEngine/Misc/Exception.hpp"

/* Level */

Level::Level(CoordInt width, CoordInt height, bool mp):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]()),
    _physics(Automaton(width*subdivisionCount, height*subdivisionCount, 0.5, 0.995, mp)),
    _objects(),
    _timeSlice(0.01),
    _time(0)
{
    initCells();
}

Level::~Level()
{
    delete[] _cells;
}

void Level::getFallChannel(const CoordInt x, const CoordInt y,
        LevelCell **aside, LevelCell **asideBelow)
{
    *aside = &_cells[x+y*_width];
    if ((*aside)->here || (*aside)->reservedBy)
    {
        *aside = 0;
        *asideBelow = 0;
        return;
    }
    else
        *asideBelow = &_cells[x+(y+1)*_width];
    if ((*asideBelow)->here || (*asideBelow)->reservedBy)
    {
        *aside = 0;
        *asideBelow = 0;
    }
}

bool Level::handleCAInteraction(const CoordInt x, const CoordInt y,
    LevelCell *cell, GameObject *obj)
{
    return true;
}

bool Level::handleGravity(const CoordInt x, const CoordInt y, LevelCell *cell,
    GameObject *obj)
{
    if (y == _height) {
        // TODO: allow objects to leave the gamescope
        return true;
    }

    LevelCell *below = &_cells[x+(y+1)*_width];
    if (!below->here && !below->reservedBy) {
        obj->setMovement(new MovementStraight(cell, below, 0, 1));
    } else if (below->here
        && below->here->getIsRollable()
        && obj->getIsRollable())
    {
        LevelCell *left = 0, *leftBelow = 0;
        LevelCell *right = 0, *rightBelow = 0;
        if (x > 0) {
            getFallChannel(x-1, y, &left, &leftBelow);
        }
        if (x < _width-1) {
            getFallChannel(x+1, y, &right, &rightBelow);
        }

        LevelCell *selected = 0, *selectedBelow = 0;
        CoordInt xOffset = 0;
        if (left) {
            // TODO: Use random here?
            selected = left;
            selectedBelow = leftBelow;
            xOffset = -1;
        } else if (right) {
            selected = right;
            selectedBelow = rightBelow;
            xOffset = 1;
        }

        if (selected) {
            obj->setMovement(new MovementRoll(cell, selected, selectedBelow, xOffset, 1));
        }
    }
    return true;
}

void Level::initCells()
{
    const LevelCell *end = &_cells[_width*_height];
    for (LevelCell *cell = _cells; cell != end; cell++) {
        cell->here = 0;
        cell->reservedBy = 0;
    }
}

void Level::getPhysicsCellsAt(const double x, const double y, CellStamp *stamp,
    CoordInt *px, CoordInt *py)
{
    *px = (CoordInt)(x * subdivisionCount) - subdivisionCount / 2;
    *py = (CoordInt)(y * subdivisionCount) - subdivisionCount / 2;
    _physics.getCellStampAt(*px, *py, stamp);
}

void Level::setStamp(const double x, const double y, const Stamp &stamp,
    bool block)
{
    const CoordInt px = (CoordInt)(x * subdivisionCount) - subdivisionCount / 2;
    const CoordInt py = (CoordInt)(y * subdivisionCount) - subdivisionCount / 2;
    _physics.applyBlockStamp(px, py, stamp, block);
}

void Level::debug_testStamp(const double x, const double y, bool block)
{
    static const BoolCellStamp stampMap = {
        false, true, true, false,
        true, true, true, true,
        true, true, true, true,
        false, true, true, false
    };
    static const Stamp stamp(stampMap);
    _physics.waitFor();
    setStamp(x, y, stamp, block);
}

void Level::debug_testBlockStamp()
{
    static const double x = 23.0;
    static const double y = 42.0;
    debug_testStamp(x, y, true);
}

void Level::debug_testUnblockStamp()
{
    static const double x = 23.5;
    static const double y = 42.0;
    debug_testStamp(x, y, false);
}

void Level::physicsToGLTexture(bool threadRegions)
{
    _physics.toGLTexture(0.0, 2.0, threadRegions);
}

void Level::update()
{
    _physics.waitFor();
    LevelCell *cell = &_cells[-1];
    for (CoordInt x = 0; x < _width; x++)
    {
        for (CoordInt y = 0; y < _height; y++)
        {
            cell++;
            GameObject *obj = cell->here;
            if (!obj)
                continue;

            if (!handleCAInteraction(x, y, cell, obj)) {
                // object destroyed
                continue;
            }

            Movement *movement = obj->getCurrentMovement();
            if (movement) {
                if (movement->update(_timeSlice)) {
                    movement = 0;
                }
            }

            if (obj->getIsGravityAffected() && !movement) {
                if (!handleGravity(x, y, cell, obj)) {
                    // object destroyed.
                    continue;
                }
            }
        }
    }

    static const double origins[4][2] = {
        {50.0, 50.0},
        {50.0, 50.0},
        {50.0, 50.0},
        {50.0, 50.0}
    };
    static const double phaseFreq[4][2] = {
        {0.4, 2.0},
        {0.45, 2.0},
        {0.5, -1.5},
        {0.55, -1.5},
    };
    for (int i = 0; i < 4; i++) {
        const double cx = origins[i][0], cy = origins[i][1];
        const double phase = phaseFreq[i][0], freq = phaseFreq[i][1];

        const double r = sin(_time * 5.0) * 1.0 + 10.0;
        const double y = cy + sin(_time * freq + phase) * r;
        const double x = cx + cos(_time * freq + phase) * r;

        debug_testStamp(x, y, false);
    }

    _time += _timeSlice;

    for (int i = 0; i < 4; i++) {
        const double cx = origins[i][0], cy = origins[i][1];
        const double phase = phaseFreq[i][0], freq = phaseFreq[i][1];

        const double r = sin(_time * 5.0) * 1.0 + 10.0;
        const double y = cy + sin(_time * freq + phase) * r;
        const double x = cx + cos(_time * freq + phase) * r;

        debug_testStamp(x, y, true);
    }

    /*const double y0 = 10;
    for (int i = 0; i < 80; i++) {
        const double x = 50 + sin(_time * 2.0) * 10.0;
        const double y = y0 + i * 0.5;

        debug_testStamp(x, y, false);
    }

    _time += _timeSlice;

    for (int i = 0; i < 80; i++) {
        const double x = 50 + sin(_time * 2.0) * 10.0;
        const double y = y0 + i * 0.5;

        debug_testStamp(x, y, true);
    }*/

    _physics.resume();
}
