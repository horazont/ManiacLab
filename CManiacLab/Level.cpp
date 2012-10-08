/**********************************************************************
File name: Level.cpp
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
#include "Level.hpp"

#include <cmath>
#include <cstdlib>

#include "CEngine/Misc/Exception.hpp"

#include "TestObject.hpp"

/* Level */

Level::Level(CoordInt width, CoordInt height, bool mp):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]()),
    _physics(Automaton(width*subdivisionCount, height*subdivisionCount, SimulationConfig(
        0.5,        // flow friction
        0.995,      // flow damping
        0.25,       // convection friction
        0.05        // heat flow friction
    ), mp)),
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
    if (y == _height - 1) {
        // TODO: allow objects to leave the gamescope
        return true;
    }
    assert(!(obj->movement));

    LevelCell *below = &_cells[x+(y+1)*_width];
    if (!below->here && !below->reservedBy) {
        obj->movement = new MovementStraight(cell, below, 0, 1);
    } else if (below->here
        && below->here->isRollable
        && obj->isRollable)
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
            obj->movement = new MovementRoll(cell, selected, selectedBelow, xOffset, 1);
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

CoordPair Level::getPhysicsCoords(const double x, const double y)
{
    CoordPair result;
    result.x = (CoordInt)(x * subdivisionCount);// - subdivisionCount / 2;
    result.y = (CoordInt)(y * subdivisionCount);// - subdivisionCount / 2;
    return result;
}

void Level::cleanupCell(LevelCell *cell)
{
    GameObject *const obj = cell->here;
    if (obj)
    {
        _physics.clearCells(obj->phy.x, obj->phy.y, obj->stamp);
        delete obj;
    }
}

void Level::debug_testHeatStamp(const double temperature)
{
    static const BoolCellStamp stampMap = {
        false, true, true, false,
        true, true, true, true,
        true, true, true, true,
        false, true, true, false
    };
    static const Stamp stamp(stampMap);
    CoordPair coord = getPhysicsCoords(49.5, 35);
    _physics.waitFor();
    _physics.applyTemperatureStamp(coord.x, coord.y, stamp, temperature);
}

void Level::debug_testObject()
{
    const CoordInt x = ((double)rand() / RAND_MAX) * (_width - 1);
    const CoordInt y = ((double)rand() / RAND_MAX) * (_height - 1);
    LevelCell *cell = &_cells[y*_width + x];
    if (!cell->here && !cell->reservedBy) {
        GameObject *const obj = new TestObject();
        cell->here = obj;
        obj->x = x;
        obj->y = y;
        obj->phy = getPhysicsCoords(x, y);
        _physics.waitFor();
        _physics.placeObject(obj->phy.x, obj->phy.y, obj, 15);
    }
}

void Level::physicsToGLTexture(bool threadRegions)
{
    _physics.toGLTexture(0.0, 2.0, threadRegions);
}

void Level::update()
{
    _physics.waitFor();
    LevelCell *cell = &_cells[-1];
    for (CoordInt y = 0; y < _height; y++)
    {
        for (CoordInt x = 0; x < _width; x++)
        {
            cell++;
            GameObject *obj = cell->here;
            if (!obj)
                continue;

            if (!handleCAInteraction(x, y, cell, obj)) {
                cleanupCell(cell);
                continue;
            }

            Movement *movement = obj->movement;
            if (movement) {
                if (movement->update(_timeSlice)) {
                    movement = 0;
                }
                CoordPair newCoords = getPhysicsCoords(obj->x, obj->y);
                if (newCoords != obj->phy) {
                    if (obj->stamp && obj->stamp->nonEmpty()) {
                        /*_physics.clearCells(obj->phy.x, obj->phy.y, obj->stamp);
                        _physics.placeObject(newCoords.x, newCoords.y, obj);*/
                        _physics.moveStamp(
                            obj->phy.x, obj->phy.y,
                            newCoords.x, newCoords.y,
                            obj->stamp
                        );
                    }
                    obj->phy = newCoords;
                }
            }

            if (obj->isGravityAffected && !movement) {
                if (!handleGravity(x, y, cell, obj)) {
                    cleanupCell(cell);
                    continue;
                }
            }
        }
    }

    /*static const double origins[4][2] = {
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
    }*/

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

    _time += _timeSlice;
    debug_testHeatStamp((sin(_time) + 1.0));

    _physics.resume();
}
