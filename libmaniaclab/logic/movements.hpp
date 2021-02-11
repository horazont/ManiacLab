#ifndef _ML_MOVEMENTS_H
#define _ML_MOVEMENTS_H

#include "logic/physics.hpp"

struct LevelCell;
struct GameObject;

class Movement {
public:
    Movement(GameObject *obj,
             CoordInt offset_x, CoordInt offset_y);
    virtual ~Movement();

protected:
    TickCounter m_time;
    GameObject *m_obj;
    GameObject *m_dependency;

protected:
    void delete_self();
    bool finalize();

public:
    const CoordInt offset_x;
    const CoordInt offset_y;

public:
    virtual void skip() = 0;

    /**
     * Advance the movement by one tick. Return either true if the movement is
     * still in progress, or the result of the after_movement handler from the
     * object if the movement is finished.
     */
    virtual bool update() = 0;

};

class MovementStraight: public Movement {
public:
    MovementStraight(
        LevelCell *from, LevelCell *to,
        int offsetX, int offsetY);
    ~MovementStraight() override;

private:
    LevelCell *m_from, *m_to;
    CoordInt m_start_x, m_start_y;

public:
    void skip() override;
    bool update() override;

public:
    static const double duration;
    static const TickCounter duration_ticks;

};

class MovementRoll: public Movement {
public:
    MovementRoll(
        LevelCell *from, LevelCell *via, LevelCell *to,
        int offsetX, int offsetY);
    ~MovementRoll() override;

private:
    LevelCell *m_from, *m_via, *m_to;
    CoordInt m_start_x, m_start_y;
    SimFloat m_start_phi;
    const Vector2f m_midpoint;
    const float m_radius;
    bool m_cleared_from;

public:
    void skip() override;
    bool update() override;

    static const double half_duration;
    static const TickCounter half_duration_ticks;
};

#endif
