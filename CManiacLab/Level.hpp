/**********************************************************************
File name: Level.hpp
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
#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include <CEngine/IO/Stream.hpp>

#include "Types.hpp"
#include "GameObject.hpp"
#include "Physics.hpp"

struct Cell;

struct LevelCell {
    GameObject *here, *reservedBy;
};

class Level {
public:
    Level(CoordInt width, CoordInt height, bool mp = true);
    ~Level();
private:
    CoordInt _width, _height;
    LevelCell *_cells;
    Automaton _physics;
    std::vector<GameObject*> _objects;

    double _timeSlice;
    double _time;
private:
    void getFallChannel(const CoordInt x, const CoordInt y, LevelCell **aside, LevelCell **asideBelow);
    bool handleCAInteraction(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    bool handleGravity(const CoordInt x, const CoordInt y, LevelCell *cell, GameObject *obj);
    void initCells();
protected:
    CoordPair getPhysicsCoords(const double x, const double y);
public:
    void cleanupCell(LevelCell *cell);
    void debug_testHeatStamp(const double temperature);
    void debug_testObject();
    void debug_testStamp(const double x, const double y, bool block);
    void debug_output(const double x, const double y);
    void physicsToGLTexture(bool threadRegions);
    void update();
};

typedef boost::shared_ptr<Level> LevelHandle;

#endif
