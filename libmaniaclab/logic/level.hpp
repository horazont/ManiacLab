#ifndef ML_LEVEL_H
#define ML_LEVEL_H

#include <queue>
#include <vector>
#include <random>

#include <sigc++/sigc++.h>

#include "logic/types.hpp"
#include "logic/game_object.hpp"
#include "logic/physics.hpp"
#include "logic/particles.hpp"

class Level;

struct LevelCell {
    LevelCell();

    std::unique_ptr<GameObject> here;
    GameObject *reserved_by;
};

typedef std::function<void(Level&, LevelCell*)> TimerFunc;

struct Timer {
    Timer(const TickCounter trigger_at,
          const CoordInt cellx,
          const CoordInt celly,
          TimerFunc func);
    Timer(const Timer &ref) = delete;
    Timer &operator=(const Timer &ref) = delete;
    Timer(Timer &&ref) noexcept;
    Timer &operator=(Timer &&ref) noexcept;
    ~Timer();

    TickCounter trigger_at;
    CoordInt x, y;
    TimerFunc func;

    inline bool operator<(const Timer &other) const
    {
        return trigger_at > other.trigger_at;
    }

};


// from https://stackoverflow.com/a/22050490/1248008
template <typename Type, typename Compare = std::less<Type>>
class MoveQueue
{
    static_assert(std::is_nothrow_move_constructible<Type>::value,
                  "Type must be nothrow move constructible");

private:
    std::vector<Type> m_elements;
    Compare m_compare;

public:
    explicit MoveQueue(const Compare &compare = Compare()):
        m_compare{compare}
    {

    }

    void push(Type element)
    {
        m_elements.push_back(std::move(element));
        std::push_heap(m_elements.begin(), m_elements.end(), m_compare);
    }

    template <typename... arg_ts>
    void emplace(arg_ts&&... args)
    {
        m_elements.emplace_back(args...);
        std::push_heap(m_elements.begin(), m_elements.end(), m_compare);
    }

    inline bool empty() const
    {
        return m_elements.empty();
    }

    const Type &top() const
    {
        return m_elements.front();
    }

    Type pop()
    {
        std::pop_heap(m_elements.begin(), m_elements.end(), m_compare);
        Type result = std::move(m_elements.back());
        m_elements.pop_back();
        return std::move(result);
    }
};


class LevelOperator
{
public:
    virtual ~LevelOperator();

public:
    virtual void operator()(Level &level) = 0;

};

using LevelOperatorPtr = std::unique_ptr<LevelOperator>;


typedef sigc::signal<void(Level&, GameObject*)> PlayerDeathEvent;
typedef sigc::signal<void(Level&, GameObject&)> ObjectSpawnEvent;

class Level {
public:
    static constexpr double time_slice = 0.004;

public:
    Level(CoordInt width, CoordInt height);

private:
    std::mt19937 m_rnge;

    const CoordInt m_width, m_height;
    std::vector<LevelCell> m_cells;
    NativeLabSim m_physics;

    GameObject *m_player;
    PlayerDeathEvent m_on_player_death;
    ObjectSpawnEvent m_on_object_spawn;

    ParticleSystem m_physics_particles;

    TickCounter m_ticks;
    MoveQueue<Timer> m_timers;

public:
    void add_explosion(const CoordInt x,
                       const CoordInt y);

    void add_large_explosion(
            const CoordInt x0,
            const CoordInt y0,
            const CoordInt xradius,
            const CoordInt yradius);

    void add_large_particle_explosion(
            const CoordInt x0,
            const CoordInt y0,
            const CoordInt xradius,
            const CoordInt yradius);

    void cleanup_cell(LevelCell *cell);

    void debug_test_stamp(const double x, const double y);

    void debug_output(const double x, const double y);

    inline LevelCell *get_cell(
        CoordInt x, CoordInt y)
    {
        return &m_cells[x+y*m_width];
    }

    void get_fall_channel(
        const CoordInt x,
        const CoordInt y,
        LevelCell *&aside,
        LevelCell *&asidebelow);

    inline CoordInt height() const
    {
        return m_height;
    }

    CoordPair get_physics_coords(const double x, const double y) const;

    inline TickCounter get_ticks() const
    {
        return m_ticks;
    }

    inline CoordInt width() const
    {
        return m_width;
    }

    inline ParticleSystem &particles()
    {
        return m_physics_particles;
    }

    inline NativeLabSim &physics()
    {
        return m_physics;
    }

    SimFloat measure_object_avg(const GameObject &obj,
            const std::function<SimFloat(const LabCell&)> &sensor
            ) const;

    SimFloat measure_stamp_avg(const CoordInt x, const CoordInt y, const Stamp &stamp,
            const std::function<SimFloat(const LabCell&)> &sensor
            ) const;

    SimFloat measure_border_avg(
            const double x, const double y,
            const std::function<SimFloat(const LabCell&)> &sensor
            , bool exclude_blocked) const;

    Vector<SimFloat, 2> measure_object_gradient(const GameObject &obj,
            const std::function<SimFloat(const LabCell&)> &sensor,
            bool exclude_blocked) const;

    void physics_to_gl_texture(bool thread_regions);

    void place_object(
            std::unique_ptr<GameObject> obj,
            const CoordInt x,
            const CoordInt y,
            const SimFloat initial_temperature);

    void place_player(
            std::unique_ptr<GameObject> player,
            const CoordInt x,
            const CoordInt y);

    template <typename T, typename... arg_ts>
    inline T *emplace_object(const CoordInt x, const CoordInt y,
                             const SimFloat initial_temperature,
                             arg_ts&&... args)
    {
        auto obj = std::make_unique<T>(*this, args...);
        T *obj_ptr = obj.get();
        place_object(std::move(obj), x, y, initial_temperature);
        return obj_ptr;
    }

    template <typename T, typename... arg_ts>
    inline T *emplace_player(const CoordInt x, const CoordInt y,
                             arg_ts&&... args)
    {
        auto obj = std::make_unique<T>(*this, args...);
        T *obj_ptr = obj.get();
        place_player(std::move(obj), x, y);
        return obj_ptr;
    }

    void step_singlebuffered_simulation();

public:
    inline PlayerDeathEvent &on_player_death()
    {
        return m_on_player_death;
    }

    inline ObjectSpawnEvent &on_object_spawn()
    {
        return m_on_object_spawn;
    }

};

#endif
