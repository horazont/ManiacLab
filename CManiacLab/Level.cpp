#include "Level.hpp"

#include "CEngine/Misc/Exception.hpp"

/* Level */

Level::Level(CoordInt width, CoordInt height):
    _width(width),
    _height(height),
    _cells(new LevelCell[width*height]()),
    _physics(Automaton(width*4, height*4)),
    _objects()
{
    _physics.setFlowFriction(0.5);
    _physics.setFlowDamping(0.995);
    initCells();
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

void Level::initCells()
{
    const LevelCell *end = &_cells[_width*_height];
    for (LevelCell *cell = _cells; cell != end; cell++) {
        cell->here = 0;
        cell->reservedBy = 0;
    }
}

void Level::getPhysicsCellsAt(const double x, const double y, CellStamp *stamp)
{
    const CoordInt px = (CoordInt)(x * subdivisionCount) - subdivisionCount / 2;
    const CoordInt py = (CoordInt)(y * subdivisionCount) - subdivisionCount / 2;
    _physics.getCellStampAt(px, py, stamp);
}

void Level::update()
{
    LevelCell *cell = &_cells[-1];
    for (CoordInt x = 0; x < _width; x++)
    {
        for (CoordInt y = 0; y < _height; y++)
        {
            cell++;
            GameObject *obj = cell->here;
            if (!obj)
                continue;

            // TODO: insert CA interaction here

            Movement movement = obj->getCurrentMovement();
            if (movement) {
                if (movement->update()) {
                    obj->clearMovement();
                }
            }

            if (obj->getIsGravityAffected() && !movement) {
                LevelCell *below = &_cells[x+(y+1)*_width];
                if (y == _height) {
                    // below is invalid!
                    // TODO: allow objects to leave the gamescope
                } else if (!below->here && !below->reservedBy) {
                    below->here = obj;
                    cell->here = 0;
                    cell->reservedBy = obj;
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
                        obj->setMovement(new MovementRoll(cell, selected, selectedRight, xOffset, 1));
                    }
                }
            }
        }
    }
    _physics.update();
}
