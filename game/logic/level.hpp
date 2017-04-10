#ifndef _ML_LEVEL_H
#define _ML_LEVEL_H

#include <queue>
#include <vector>

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
    Timer(Timer &&ref);
    Timer &operator=(Timer &&ref);
    ~Timer();

    TickCounter trigger_at;
    CoordInt x, y;
    TimerFunc func;

    inline bool operator<(const Timer &other) const
    {
        return trigger_at > other.trigger_at;
    }

};

typedef sigc::signal<void, Level*, GameObject*> PlayerDeathEvent;

class Level {
public:
    static constexpr double time_slice = 0.005;

public:
    Level(CoordInt width, CoordInt height, bool mp = true);

private:
    CoordInt m_width, m_height;
    std::vector<LevelCell> m_cells;
    NativeLabSim m_physics;
    std::vector<GameObject*> m_objects;

    GameObject *m_player;
    PlayerDeathEvent m_on_player_death;

    ParticleSystem m_physics_particles;

    TickCounter m_ticks;
    std::priority_queue<Timer> m_timers;

public:
    void add_explosion(const CoordInt x,
                       const CoordInt y);

    void add_large_explosion(
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

    inline CoordInt get_height() const
    {
        return m_height;
    }

    CoordPair get_physics_coords(const double x, const double y);

    inline TickCounter get_ticks() const
    {
        return m_ticks;
    }

    inline CoordInt get_width() const
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
        auto obj = std::make_unique<T>(this, args...);
        T *obj_ptr = obj.get();
        place_object(std::move(obj), x, y, initial_temperature);
        return obj_ptr;
    }

    void update();

public:
    inline PlayerDeathEvent &on_player_death()
    {
        return m_on_player_death;
    }

};

#endif
