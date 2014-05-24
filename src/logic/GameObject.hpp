/**********************************************************************
File name: GameObject.hpp
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
#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

#include <CEngine/IO/Stream.hpp>
#include <CEngine/Misc/Exception.hpp>
#include <CEngine/GL/GeometryBuffer.hpp>

#include "Movements.hpp"
#include "Stamp.hpp"

#include "io/TilesetData.hpp"

struct Cell;
class Level;
class Movement;

/**
 * The FrameState holds a set of flags and values which are calculated for each
 * object each frame. Each object has its own FrameState and its updated by the
 * engine before calling the GameObject::frame() method. This allows objects to
 * react on events in their surroundings simultanously (the data is latched).
 */
struct FrameState
{
    /** The object is part of an explosion area */
    bool explode;

    /**
     * The object is being hit by igniting particles. This has nothing to do
     * with any igniting thresholds defined in the object, but it can be used
     * by an object to trigger ignition, if combustion cannot be triggered by
     * temperature alone.
     *
     * Examples of ignition triggers are explosions, flames and sparks.
     */
    bool ignite;

    /**
     * Average temperature of the object. This only applies if it has blocking
     * parts in its stamp.
     */
    float own_temperature;

    /**
     * Surrounding temperature in the fields neighbouring the object.
     */
    float surr_temperature;

    /**
     * Reset the frame state to defaults. This should be called by the
     * GameObject after each processing. The Engine will only set some flags,
     * not reset them by itself.
     */
    void reset();
};

struct ObjectInfo: public TileData
{
    explicit ObjectInfo(const CellStamp &stamp);
    explicit ObjectInfo(const TileData &src_data);
    ObjectInfo(const ObjectInfo &ref) = default;
    ObjectInfo &operator=(const ObjectInfo &ref) = default;

    Stamp stamp;
};

struct ObjectView
{
    ObjectView();

    bool invalidated;
    PyEngine::GL::VertexIndexListHandle vertices;
};

enum Acting {
    NONE = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT
};

struct GameObject
{
public:
    explicit GameObject(const ObjectInfo &info);
    GameObject(const GameObject &ref) = delete;
    GameObject& operator=(const GameObject &ref) = delete;

public:
    FrameState frame_state;
    const ObjectInfo &info;

    double x, y, phi;
    std::unique_ptr<Movement> movement;

    CoordPair phy;

    ObjectView view;

    Acting acting;
};

#endif
