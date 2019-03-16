#include "movements.hpp"

#include <cassert>
#include <iostream>

#include "errors.hpp"

/* Movement */

Movement::Movement(
        GameObject *obj,
        CoordInt offset_x,
        CoordInt offset_y):
    m_time(0),
    m_obj(obj),
    offset_x(offset_x),
    offset_y(offset_y)
{

}

Movement::~Movement()
{

}

void Movement::delete_self()
{
    m_obj->movement = nullptr;
    // movement is a unique_ptr, this will call the destructor
}

bool Movement::finalize()
{
    std::unique_ptr<Movement> me = std::move(m_obj->movement);
    bool result = m_obj->after_movement(this);
    return result;
}

/* MovementStraight */

MovementStraight::MovementStraight(
        LevelCell *from,
        LevelCell *to,
        int offset_x, int offset_y):
    Movement::Movement(from->here.get(), offset_x, offset_y),
    m_from(from),
    m_to(to),
    m_start_x(m_obj->x),
    m_start_y(m_obj->y)
{
    if (abs(offset_x) + abs(offset_y) == 0) {
        throw std::runtime_error("Cannot move zero fields.");
    } else if (abs(offset_x) + abs(offset_y) > 1) {
        throw std::runtime_error("Cannot move diagonally or more than one field.");
    }

    // yes, this comparision is evil, as _obj->_x is a float actually.
    // However, in this case _x should be close enough to a whole number, if
    // not, something went utterly wrong.
    assert(m_obj->x == m_start_x);
    assert(m_obj->y == m_start_y);
    assert(from->here);
    assert(!from->reserved_by);
    assert(!to->here);
    // assert(!to->reserved_by);

    from->reserved_by = m_obj;
    to->here = std::move(from->here);

    m_obj->cell = CoordPair{
            m_start_x + offset_x,
            m_start_y + offset_y};
}

MovementStraight::~MovementStraight()
{
    std::cout << "delete straight" << std::endl;
    m_from->reserved_by = nullptr;
}

void MovementStraight::skip()
{
    m_obj->x = m_start_x + offset_x;
    m_obj->y = m_start_y + offset_y;
    delete_self();
}

bool MovementStraight::update()
{
    m_time += 1;

    if (m_to->reserved_by)
    {
        // if another object is currently moving out ouf the cell we’re moving
        // in, we have to make sure it gets updated before us, to avoid
        // collisions.
        m_to->reserved_by->update();
    }

    if (m_obj->info.is_round)
    {
        if (offset_x != 0) {
            m_obj->phi += Level::time_slice / m_obj->info.roll_radius * offset_x;
        } else {
            m_obj->phi += sin(m_time * Level::time_slice * 2*3.14159) / 100;
        }
    }

    if (m_time >= duration_ticks) {
        m_obj->x = m_start_x + offset_x;
        m_obj->y = m_start_y + offset_y;
        m_obj->invalidate_view();
        return finalize();
    } else {
        m_obj->x = m_start_x + offset_x * ((double)m_time / duration_ticks);
        m_obj->y = m_start_y + offset_y * ((double)m_time / duration_ticks);
        m_obj->invalidate_view();
        return true;
    }
}

const double MovementStraight::duration = 1.0;
const TickCounter MovementStraight::duration_ticks = MovementStraight::duration / Level::time_slice;

/* MovementRoll */

MovementRoll::MovementRoll(
        LevelCell *from, LevelCell *via, LevelCell *to,
        int offset_x, int offset_y):
    Movement::Movement(from->here.get(),
                       offset_x, offset_y),
    m_from(from),
    m_via(via),
    m_to(to),
    m_start_x(m_obj->x),
    m_start_y(m_obj->y),
    m_cleared_from(false)
{
    if (abs(offset_x) != 1 || offset_y != 1) {
        throw std::runtime_error(
            "Cannot roll-move with offset_y != 1 or abs(offset_x != 1)");
    }

    assert(from->here);
    assert(!from->reserved_by);
    assert(!to->here);
    assert(!via->here);

    to->here = std::move(from->here);
    from->reserved_by = m_obj;
    via->reserved_by = m_obj;

    m_obj->cell = CoordPair{
            m_start_x + offset_x,
            m_start_y + offset_y};
}

MovementRoll::~MovementRoll()
{
    m_via->reserved_by = nullptr;
    if (!m_cleared_from) {
        m_from->reserved_by = nullptr;
    }
}

void MovementRoll::skip()
{
    m_obj->x = m_start_x + offset_x;
    m_obj->y = m_start_y + offset_y;
    m_obj->invalidate_view();
    delete_self();
}

bool MovementRoll::update()
{
    m_time += 1;

    if (m_time >= half_duration_ticks*2) {
        m_obj->x = m_start_x + offset_x;
        m_obj->y = m_start_y + offset_y;
        m_obj->invalidate_view();
        return finalize();
    }

    if (m_time >= half_duration_ticks) {
        m_obj->x = m_start_x + offset_x;
        m_obj->y = m_start_y + offset_y * ((m_time-half_duration_ticks) * Level::time_slice * 2);
        /* m_cleared_from = true;
        m_from->reserved_by = nullptr; */
    } else {
        m_obj->x = m_start_x + offset_x * (m_time * Level::time_slice * 2);
        m_obj->y = m_start_y;
    }

    m_obj->invalidate_view();
    return true;
}

const double MovementRoll::half_duration = MovementStraight::duration / 2;
const TickCounter MovementRoll::half_duration_ticks = MovementRoll::half_duration / Level::time_slice;
