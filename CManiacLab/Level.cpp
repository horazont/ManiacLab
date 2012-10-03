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


void Level::physicsToGLTexture()
{
    _physics.toGLTexture(0.9, 1.1);
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
    _time += _timeSlice;
    for (CoordInt y = 0; y < _height*subdivisionCount; y++) {
        // const double factor = (y >= _height * subdivisionCount / 2) ? 4 : 5;
        const double factor = 6;
        _physics.cellAt(0, y)->airPressure = sin(_time * factor) + 1.1;
        _physics.cellAt(0, y)->airPressure = sin(_time * factor) + 1.1;
        _physics.cellAt(0, y)->airPressure = sin(_time * factor) + 1.1;

        if (y % 15 != 0 && (y+1) % 15 != 0 && (y-1) % 15 != 0) {
            _physics.setBlocked(15, y, true);
        }
    }
    _physics.resume();
}
