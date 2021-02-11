#ifndef ML_GAME_OBJECT_H
#define ML_GAME_OBJECT_H

#include <ffengine/common/types.hpp>

#include "logic/movements.hpp"
#include "logic/stamp.hpp"
#include "logic/physics.hpp"
#include "logic/sensor.hpp"

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
     * The object is not moving.
     */
    bool idle;

    /**
     * Reset the frame state to defaults. This should be called by the
     * GameObject after each processing. The Engine will only set some flags,
     * not reset them by itself.
     */
    void reset();
};

struct ObjectInfo
{
    ObjectInfo(
            bool is_blocking,
            bool is_destructible,
            bool is_collectable,
            bool is_gravity_affected,
            bool is_movable,
            bool is_round,
            bool is_sticky,
            float roll_radius,
            const CellStamp &stamp);
    ObjectInfo(const ObjectInfo &ref) = default;
    ObjectInfo &operator=(const ObjectInfo &ref) = default;

    bool is_actor;
    bool is_blocking;
    bool is_destructible;
    bool is_collectable;
    bool is_gravity_affected;
    bool is_movable;
    bool is_round;
    bool is_sticky;

    float roll_radius;

    Stamp stamp;
};

class TileMaterialManager;
struct GameObject;

struct ObjectView
{
public:
    ObjectView();
    ObjectView(const ObjectView &ref) = default;
    ObjectView &operator=(const ObjectView &ref) = default;
    virtual ~ObjectView();

protected:
    bool m_object_view_invalidated;

public:
    void invalidate();

};

struct ViewableObject
{
public:
    ViewableObject();
    ViewableObject(const ViewableObject &ref) = delete;
    ViewableObject &operator=(const ViewableObject &ref) = delete;
    virtual ~ViewableObject();

private:
    std::unique_ptr<ObjectView> m_view;

public:
    void set_view(std::unique_ptr<ObjectView> &&view);

    inline void invalidate_view() const
    {
        if (m_view) {
            m_view->invalidate();
        }
    }

};

enum MoveDirection {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT
};

struct GameObject: public ViewableObject
{
public:
    explicit GameObject(const ObjectInfo &info,
                        Level &level,
                        const SimFloat heat_capacity);
    GameObject(const GameObject &ref) = delete;
    GameObject& operator=(const GameObject &ref) = delete;

public:
    Level &level;
    FrameState frame_state;
    const ObjectInfo &info;
    CoordPair cell;
    SimFloat x, y, phi;
    Vector2f visual_pos;
    bool flip;
    std::unique_ptr<Movement> movement;
    CoordPair phy;
    SimFloat heat_capacity;

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
    inline const ObjectInfo &get_info() const
    {
        return info;
    }

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
     * @param on_object The object which was hit by this object. May be nullptr
     * if the object hits the boundaries of the level.
     * @return false if headache() and subsequent handlers should not be called,
     * true otherwise.
     */
    virtual bool impact(GameObject *on_object);

    /**
     * Instruct the object to move into the given direction. Return whether
     * movement initiation worked.
     *
     * @param dir Direction to move to
     * @param chain_move Whether to allow moving a blocking object.
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


class GenericGameObject: public GameObject
{
private:
    std::vector<std::unique_ptr<Sensor> > m_sensors;

public:
    template <typename T, typename... arg_ts>
    T &emplace_sensor(arg_ts&&... args)
    {
        auto obj = std::make_unique<T>(args...);
        T &result = *obj;
        m_sensors.emplace_back(std::move(obj));
        return result;
    }

};


CoordPair move_direction_to_vector(MoveDirection dir);

#endif
