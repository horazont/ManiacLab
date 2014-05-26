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
    ObjectInfo(
        bool is_blocking,
        bool is_destructible,
        bool is_edible,
        bool is_gravity_affected,
        bool is_movable,
        bool is_rollable,
        bool is_sticky,
        float roll_radius,
        float temp_coefficient,
        const CellStamp &stamp);
    explicit ObjectInfo(const CellStamp &stamp);
    explicit ObjectInfo(const TileData &src_data);
    ObjectInfo(const ObjectInfo &ref) = default;
    ObjectInfo &operator=(const ObjectInfo &ref) = default;

    Stamp stamp;
};

class TileMaterialManager;
struct GameObject;

struct ObjectView
{
public:
    ObjectView();
    ObjectView(const ObjectView &ref) = delete;
    ObjectView &operator=(const ObjectView &ref) = delete;
    virtual ~ObjectView();

private:
    bool _invalidated;

public:
    void invalidate();

    virtual void update(
        GameObject &object,
        PyEngine::TimeFloat deltaT);

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
    explicit GameObject(const ObjectInfo &info,
                        Level *level);
    GameObject(const GameObject &ref) = delete;
    GameObject& operator=(const GameObject &ref) = delete;
    virtual ~GameObject();

public:
    Level *const level;
    FrameState frame_state;
    const ObjectInfo &info;
    CoordPair cell;
    double x, y, phi;
    std::unique_ptr<Movement> movement;
    CoordPair phy;
    std::unique_ptr<ObjectView> view;

protected:
    /**
     * Destruct this object.
     */
    void destruct_self();

    /**
     * Let the object fall if possible.
     *
     * @return true if handling shall continue, false otherwise (e.g. if the
     * object has been destroyed).
     */
    bool handle_gravity();

public:
    /**
     * Notify the object of the end of a movement.
     *
     * At the time of call, the movement member is still set to the movement
     * just finished.
     *
     * @return true if further handlers (to be specified) shall be called, false
     * otherwise.
     */
    virtual bool after_movement();

    /**
     * Notify the object of a movement which is about to start.
     *
     * @param movement The movement which is going to happen.
     */
    virtual void before_movement(Movement *movement);

    /**
     * Notify the object that it has been touched by an explosion.
     *
     * The callee is allowed to delete itself, if it properly removes itself
     * from the Level.
     */
    virtual void explosion_touch();

    /**
     * Notify the object that another object has landed on top of it, driven by
     * gravity.
     *
     * headache() is called after impact() is called. Both are called from
     * after_movement() of the moving object.
     *
     * @param from_object The object which has hit this object.
     */
    virtual void headache(GameObject *from_object);

    /**
     * Called during each tick in which no movement took place from
     * update().
     *
     * @return true if the object still exists after idle().
     */
    virtual bool idle();

    /**
     * Notify the object that it has been touched by igniting particles (sparks
     * etc.).
     */
    virtual void ignition_touch();

    /**
     * Notify the object that it has landed on another object, driven by
     * gravity.
     *
     * impact() is called before headache() is called. Both are called from
     * after_movement() of the moving object.
     *
     * @param on_object The object which was hit by this object.
     * @return false if headache() and subsequent handlers should not be called,
     * true otherwise.
     */
    virtual bool impact(GameObject *on_object);

    /**
     * Notify the object that it got hit by an explosive projectile.
     *
     * The object must return true if it triggers an explosion at its site,
     * otherwise it must return false. In the false case, the projectile assumes
     * that the object cannot be destructed by projectiles and will thus explode
     * on the tile it was on before.
     *
     * @return true if the object was destructed, false otherwise.
     */
    virtual bool projectile_impact();

    /**
     * This is called once for each GameObject, upon instanciation in the game
     * view.
     *
     * For objects being present from the beginning, this is called after all
     * starting objects have been placed. For objects being created during
     * runtime, this method is called right after the object has been created.
     */
    virtual void setup_view(TileMaterialManager &matman);

    /**
     * Update the objects state by advancing by one tick.
     *
     * The default implementation takes care of quite a few things and should
     * always be called.
     */
    virtual void update();

};

#endif
