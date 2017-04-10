#include "level.hpp"

#include <iostream>

#include <cmath>
#include <cstdlib>

#include "logic/explosion_object.hpp"

/* Timer */

Timer::Timer(const TickCounter trigger_at,
             const CoordInt cellx,
             const CoordInt celly,
             TimerFunc func):
    trigger_at(trigger_at),
    x(cellx),
    y(celly),
    func(func)
{
    // std::cout << "+1 [con]" << std::endl;
}

Timer::Timer(Timer &&ref):
    trigger_at(ref.trigger_at),
    x(ref.x),
    y(ref.y),
    func(std::move(ref.func))
{
    // std::cout << " 0 [movecon]" << std::endl;
}

Timer &Timer::operator=(Timer &&ref)
{
    // std::cout << " 0 [moveassign]" << std::endl;
    trigger_at = ref.trigger_at;
    x = ref.x;
    y = ref.y;
    func = std::move(ref.func);
    return *this;
}

Timer::~Timer()
{
//     std::cout << "-1 [dest]" << std::endl;
}

/* LevelCell */

LevelCell::LevelCell():
    here(nullptr),
    reserved_by(nullptr)
{

}


/* Level */

#define WALL_CENTER_X 45

Level::Level(CoordInt width, CoordInt height, bool mp):
    m_width(width),
    m_height(height),
    m_cells(m_width*m_height),
    m_physics(
        m_width*subdivision_count,
        m_height*subdivision_count,
        SimulationConfig{1.0, 1.0, 0.0}
    ),
    m_objects(),
    m_player(nullptr),
    m_physics_particles(*this),
    m_ticks(0)
{

}

void Level::add_explosion(const CoordInt x,
                          const CoordInt y)
{
    LevelCell *const cell = get_cell(x, y);
    if (cell->here && !cell->here->info.is_destructible) {
        return;
    }

    m_timers.emplace(
        m_ticks + EXPLOSION_TRIGGER_TIMEOUT,
        x, y,
        [x, y](Level &level, LevelCell *cell) {
            if (cell->here) {
                cell->here->explosion_touch();
            }
            if (!cell->here) {
                level.place_object(
                    std::make_unique<ExplosionObject>(&level),
                    x, y, 1.0);
            }

        });

    m_physics_particles.spawn_generator(
        6,
        [x, y](PhysicsParticle *part) {
            part->type = ParticleType::FIRE;
            const float offsx = ((float)rand() / RAND_MAX)*0.5-0.25;
            const float offsy = ((float)rand() / RAND_MAX)*0.5-0.25;
            part->x = x + 0.5 + offsx;
            part->y = y + 0.5 + offsy;
            part->vx = offsx / 2;
            part->vy = offsy / 2;
            part->ax = 0;
            part->ay = 0;
            part->phi = ((float)rand() / RAND_MAX)*2*3.14159;
            part->vphi = (((float)rand() / RAND_MAX)-0.5)*3.14159/5.0;
            part->aphi = 0;
            part->lifetime = (EXPLOSION_BLOCK_LIFETIME +
                              EXPLOSION_TRIGGER_TIMEOUT) / 100.;
        });

}

void Level::add_large_explosion(const CoordInt x0,
                                const CoordInt y0,
                                const CoordInt xradius,
                                const CoordInt yradius)
{
    const CoordInt minx = x0 > xradius-1 ? x0 - xradius : x0;
    const CoordInt miny = y0 > yradius-1 ? y0 - yradius : y0;
    const CoordInt maxx = x0 < m_width - xradius ? x0 + xradius : x0;
    const CoordInt maxy = y0 < m_height - yradius ? y0 + yradius : y0;

    for (CoordInt x = minx; x <= maxx; x++) {
        for (CoordInt y = miny; y <= maxy; y++) {
            add_explosion(x, y);
        }
    }

}

void Level::cleanup_cell(LevelCell *cell)
{
    std::unique_ptr<GameObject> &obj = cell->here;
    if (!obj) {
        return;
    }

    if (obj.get() == m_player) {
        m_on_player_death(this, obj.get());
        m_player = nullptr;
    }

    m_physics.clear_cells(
                obj->phy.x, obj->phy.y,
                obj->info.stamp);

    cell->here = nullptr;
}

void Level::debug_test_stamp(const double x, const double y)
{
    static CellInfo info_arr[cell_stamp_length];
    CellInfo *info = &info_arr[0];
    for (CoordInt x = 0; x < subdivision_count; x++) {
        for (CoordInt y = 0; y < subdivision_count; y++) {
            info->offs = CoordPair(x, y);
            info->meta.blocked = false;
            info->meta.obj = nullptr;
            info->phys.air_pressure = 1.0;
            info->phys.fog_density = 10.;
            info->phys.heat_energy = 1.0;
            info->phys.flow[0] = 0;
            info->phys.flow[1] = 0;
            info++;
        }
    }

    CoordPair coord = get_physics_coords(x, y);
    m_physics.wait_for_frame();
    m_physics.place_stamp(coord.x, coord.y, &info_arr[0], cell_stamp_length);
}

void Level::debug_output(const double x, const double y)
{
    static const CoordInt offs[5][2] = {
        {0, -1}, {-1, 0}, {0, 0}, {1, 0}, {0, 1}
    };

    m_physics.wait_for_frame();

    const CoordPair phys = get_physics_coords(x, y);
    std::cout << "DEBUG: center at x = " << phys.x << "; y = " << phys.y << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "offs: " << offs[i][0] << ", " << offs[i][1] << std::endl;

        const CoordInt cx = phys.x + offs[i][0];
        const CoordInt cy = phys.y + offs[i][1];

        const LabCell *cell = m_physics.safe_front_cell_at(cx, cy);
        if (!cell) {
            std::cout << "  out of range" << std::endl;
            continue;
        }
        LabCellMeta &meta = m_physics.meta_at(cx, cy);

        const double tc = (meta.blocked
                           ? meta.obj->info.temp_coefficient
                           : airtempcoeff_per_pressure * cell->air_pressure);

        if (meta.blocked) {
            std::cout << "  blocked with " << meta.obj << std::endl;
        }
        std::cout << "  p     = " << cell->air_pressure << std::endl;
        std::cout << "  U     = " << cell->heat_energy << std::endl;
        std::cout << "  T     = " << cell->heat_energy / tc << std::endl;
        std::cout << "  f     = " << cell->fog_density << std::endl;
        std::cout << "  f[-x] = " << cell->flow[0] << std::endl;
        std::cout << "  f[-y] = " << cell->flow[1] << std::endl;
    }
}

void Level::get_fall_channel(
    const CoordInt x,
    const CoordInt y,
    LevelCell *&aside,
    LevelCell *&asidebelow)
{
    aside = &m_cells[x+y*m_width];
    if (aside->here || aside->reserved_by)
    {
        aside = nullptr;
        asidebelow = nullptr;
        return;
    } else {
        asidebelow = &m_cells[x+(y+1)*m_width];
    }

    if (asidebelow->here || asidebelow->reserved_by)
    {
        aside = nullptr;
        asidebelow = nullptr;
    }
}

CoordPair Level::get_physics_coords(const double x, const double y)
{
    CoordPair result;
    result.x = round(x * subdivision_count);// - subdivision_count / 2;
    result.y = round(y * subdivision_count);// - subdivision_count / 2;
    return result;
}

void Level::physics_to_gl_texture(bool thread_regions)
{
    m_physics.to_gl_texture(0.0, 2.0, thread_regions);
}

void Level::place_object(
        std::unique_ptr<GameObject> obj,
        const CoordInt x,
        const CoordInt y,
        const SimFloat initial_temperature)
{
    m_physics.wait_for_frame();

    LevelCell *dest = &m_cells[x+y*m_width];
    if (dest->reserved_by) {
        GameObject *reserver = dest->reserved_by;
        const CoordPair oldpos = reserver->phy;
        reserver->movement->skip();
        const CoordPair newpos = get_physics_coords(
            reserver->x,
            reserver->y);
        m_physics.move_stamp(
            oldpos.x, oldpos.y,
            newpos.x, newpos.y,
            reserver->info.stamp);
    }
    cleanup_cell(dest);

    obj->x = x;
    obj->y = y;
    obj->cell = CoordPair{x, y};
    obj->phy = get_physics_coords(x, y);
    m_physics.place_object(
        obj->phy.x, obj->phy.y,
        obj.get(), initial_temperature);

    dest->here = std::move(obj);
}

void Level::place_player(
        std::unique_ptr<GameObject> player,
        const CoordInt x,
        const CoordInt y)
{
    if (m_player) {
        return;
    }

    m_player = player.get();
    place_object(std::move(player), x, y, 1.0);
}

void Level::update()
{
    m_ticks += 1;

    m_physics.wait_for_frame();

    while (!m_timers.empty() && m_timers.top().trigger_at <= m_ticks)
    {
        const Timer &timer = m_timers.top();
        LevelCell *const cell = (timer.x >= 0 && timer.y >= 0)
            ? get_cell(timer.x, timer.y)
            : nullptr;

        timer.func(*this, cell);
        m_timers.pop();
    }

    for (CoordInt y = m_height-1; y >= 0; y--)
    {
        LevelCell *cell = get_cell(0, y);
        for (CoordInt x = 0; x < m_width; x++)
        {
            GameObject *obj = cell->here.get();
            if (!obj) {
                ++cell;
                continue;
            }
            obj->update();
            ++cell;
        }
    }

    m_physics_particles.update(time_slice);

    //debug_output(31.5, 21.5);

    m_physics.start_frame();

}
