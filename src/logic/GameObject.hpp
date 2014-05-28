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
#include "Physics.hpp"

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

struct ViewableObject
{
public:
    ViewableObject();
    ViewableObject(const ViewableObject &ref) = delete;
    ViewableObject &operator=(const ViewableObject &ref) = delete;
    virtual ~ViewableObject();

private:
    std::unique_ptr<ObjectView> _view;

protected:
    /**
     * Implementations should override _view with a new view object, using the
     * given material manager.
     *
     * @param matman Material manager to use
     */
    virtual std::unique_ptr<ObjectView> setup_view(
        TileMaterialManager &matman);

public:
    /**
     * Return the existing view or create a new one if no view exists yet.
     *
     * @param matman Material manager to use if creating a new view
     */
    ObjectView *get_view(TileMaterialManager &matman);

    inline void invalidate_view() const
    {
        if (_view) {
            _view->invalidate();
        }
    }

};

enum MoveDirection {
    NONE = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT
};

struct GameObject: public ViewableObject
{
public:
    explicit GameObject(const ObjectInfo &info,
                        Level *level);
    GameObject(const GameObject &ref) = delete;
    GameObject& operator=(const GameObject &ref) = delete;

public:
    Level *const level;
    FrameState frame_state;
    const ObjectInfo &info;
    CoordPair cell;
    double x, y, phi;
    std::unique_ptr<Movement> movement;
    CoordPair phy;

    /**
     * Stores the tick of the last full update. This is used to manage
     * dependencies between movements.
     */
    TickCounter ticks;

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
     * At the time of call, the movement member is already nullptr. If you need
     * access to the previous movement, use the argument instead.
     *
     * @param prev_movement is the previously executed movement.
     * @return true if further handlers (to be specified) shall be called, false
     * otherwise.
     */
    virtual bool after_movement(Movement *prev_movement);

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
     * Instruct the object to move into the given direction. Return whether
     * movement initiation worked.
     *
     * @param dir Direction to move to
     * @return true if the object is now moving, false otherwise.
     */
    bool move(MoveDirection dir, bool chain_move);

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
     * Update the objects state by advancing by one tick.
     *
     * The default implementation takes care of quite a few things and should
     * always be called.
     */
    virtual void update();

};

#endif
