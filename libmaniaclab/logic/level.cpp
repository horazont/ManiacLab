#define QT_NO_EMIT

#include "level.hpp"

#include <iostream>

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "ffengine/io/log.hpp"

#include "logic/explosion_object.hpp"
#include "logic/tileset.hpp"


static io::Logger &level_log = io::logging().get_logger("maniaclab.level");

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

Timer::Timer(Timer &&ref) noexcept:
    trigger_at(ref.trigger_at),
    x(ref.x),
    y(ref.y),
    func(std::move(ref.func))
{
    // std::cout << " 0 [movecon]" << std::endl;
}

Timer &Timer::operator=(Timer &&ref) noexcept
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

Level::Level(CoordInt width, CoordInt height):
    m_rnge(std::random_device()()),
    m_width(width),
    m_height(height),
    m_cells(coord_int_to_unsigned(m_width)*coord_int_to_unsigned(m_height)),
    m_physics(
        m_width*subdivision_count,
        m_height*subdivision_count,
        SimulationConfig{1.0, default_temperature, 0.0}
    ),
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
                    std::make_unique<ExplosionObject>(level),
                    x, y, 1.0);
            }

        });

    auto &rnge = m_rnge;

    std::uniform_real_distribution<float> phi_dist(0.f, M_2_PIf32);
    std::uniform_real_distribution<float> vphi_dist(-M_2_PIf32 / 10.f, M_2_PIf32 / 10.f);
    std::uniform_real_distribution<float> offs_dist(-0.2f, 0.2f);

    m_physics_particles.spawn_generator(
        6,
        [x, y, &rnge, &phi_dist, &vphi_dist, &offs_dist](std::size_t, PhysicsParticle *part) {
            part->type = ParticleType::FIRE;
            const float offsx = offs_dist(rnge);
            const float offsy = offs_dist(rnge);
            part->x = x + 0.5f + offsx;
            part->y = y + 0.5f + offsy;
            part->vx = offsx / 2;
            part->vy = offsy / 2;
            part->ax = 0;
            part->ay = 0;
            part->phi = phi_dist(rnge);
            part->vphi = vphi_dist(rnge);
            part->aphi = 0;
            part->lifetime = (EXPLOSION_BLOCK_LIFETIME +
                              EXPLOSION_TRIGGER_TIMEOUT) * static_cast<float>(time_slice);
        });

}

void Level::add_large_explosion(const CoordInt x0, const CoordInt y0,
                                const CoordInt xradius, const CoordInt yradius)
{
    const CoordInt minx = x0 > xradius-1 ? x0 - xradius : x0;
    const CoordInt miny = y0 > yradius-1 ? y0 - yradius : y0;
    const CoordInt maxx = x0 < m_width - xradius ? x0 + xradius : x0;
    const CoordInt maxy = y0 < m_height - yradius ? y0 + yradius : y0;

    for (CoordInt y = miny; y <= maxy; y++) {
        LevelCell *cell = get_cell(minx, y);
        for (CoordInt x = minx; x <= maxx; x++) {
            add_explosion(x, y);
            ++cell;
        }
    }
}

static const CoordPair PARTICLE_SPAWN_MAP[8] = {
    {1, 1},
    {1, 0},
    {1, -1},
    {0, -1},
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, 1},
};

void Level::add_large_particle_explosion(const CoordInt x0,
                                         const CoordInt y0,
                                         const CoordInt xradius,
                                         const CoordInt yradius)
{
    auto &rnge = m_rnge;

    std::uniform_real_distribution<float> phi_dist(0.f, M_2_PIf32);
    std::uniform_real_distribution<float> vphi_dist(-M_2_PIf32 / 10.f, M_2_PIf32 / 10.f);

    const CoordInt minx = x0 > xradius-1 ? x0 - xradius : x0;
    const CoordInt miny = y0 > yradius-1 ? y0 - yradius : y0;
    const CoordInt maxx = x0 < m_width - xradius ? x0 + xradius : x0;
    const CoordInt maxy = y0 < m_height - yradius ? y0 + yradius : y0;

    for (CoordInt x = minx; x <= maxx; x++) {
        const float dx = static_cast<float>(x - x0) / (xradius+1);
        for (CoordInt y = miny; y <= maxy; y++) {
            const float dy = static_cast<float>(y - y0) / (yradius+1);
            m_physics_particles.spawn_generator(
                8,
                [&rnge, &phi_dist, &vphi_dist, x0, y0, dx, dy, xradius, yradius](std::size_t i, PhysicsParticle *part) {

                    part->type = ParticleType::FIRE;
                    /*const float offsx = dx / 2;
                    const float offsy = dy / 2;*/
                    const std::size_t i_mod = i % 8;
                    const float offsx = dx / 2 + PARTICLE_SPAWN_MAP[i_mod].x / 4.f;
                    const float offsy = dy / 2 + PARTICLE_SPAWN_MAP[i_mod].y / 4.f;
                    //const float offsx = ((float)rand() / RAND_MAX)*0.5-0.25;
                    //const float offsy = ((float)rand() / RAND_MAX)*0.5-0.25;
                    part->x = x0 + 0.5f + offsx;
                    part->y = y0 + 0.5f + offsy;
                    part->vx = dx * (xradius+1) + offsx;
                    part->vy = dy * (yradius+1) + offsy;
                    part->ax = 0;
                    part->ay = 0;
                    part->phi = phi_dist(rnge);
                    part->vphi = vphi_dist(rnge);
                    part->aphi = 0;
                    part->lifetime = (EXPLOSION_BLOCK_LIFETIME +
                                      EXPLOSION_TRIGGER_TIMEOUT) / 100.;
                });

            LevelCell *const cell = get_cell(x, y);
            if (cell->here && !cell->here->info.is_destructible) {
                continue;
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
                            std::make_unique<ExplosionObject>(level),
                            x, y, 1.0);
                    }
                });
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
        m_on_player_death(*this, obj.get());
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

        const float tc = cell->heat_capacity_cache;

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

CoordPair Level::get_physics_coords(const double x, const double y) const
{
    CoordPair result;
    result.x = round(x * subdivision_count);// - subdivision_count / 2;
    result.y = round(y * subdivision_count);// - subdivision_count / 2;
    return result;
}

SimFloat Level::measure_object_avg(
        const GameObject &obj,
        const std::function<SimFloat(const LabCell &)> &sensor) const
{
    assert(&obj.level == this);
    return measure_stamp_avg(obj.phy.x, obj.phy.y, obj.info.stamp,
                             sensor);
}

SimFloat Level::measure_stamp_avg(
        const CoordInt x,
        const CoordInt y,
        const Stamp &stamp,
        const std::function<SimFloat (const LabCell &)> &sensor) const
{
    uintptr_t stamp_len = 0;
    const CoordPair *stamp_coords = stamp.get_map_coords(&stamp_len);
    return m_physics.measure_stamp_avg(x, y,
                                       stamp_coords,
                                       stamp_len,
                                       sensor,
                                       false);
}

SimFloat Level::measure_border_avg(
        const double x,
        const double y,
        const std::function<SimFloat (const LabCell &)> &sensor,
        bool exclude_blocked) const
{
    const LevelCell *dest = &m_cells[x+y*m_width];
    if (!dest->here) {
        return 0;
    }
    CoordPair physics_coords = get_physics_coords(x, y);
    uintptr_t stamp_len = 0;
    const CoordPair *stamp_coords = dest->here->info.stamp.get_border(&stamp_len);
    return m_physics.measure_stamp_avg(physics_coords.x,
                                       physics_coords.y,
                                       stamp_coords,
                                       stamp_len,
                                       sensor,
                                       exclude_blocked);
}

Vector<SimFloat, 2> Level::measure_object_gradient(
        const GameObject &obj,
        const std::function<SimFloat(const LabCell &)> &sensor,
        bool exclude_blocked) const
{
    assert(&obj.level == this);
    uintptr_t stamp_len = 0;
    const CoordPair *stamp_coords = obj.info.stamp.get_border(&stamp_len);
    return m_physics.measure_stamp_gradient(obj.phy.x,
                                            obj.phy.y,
                                            stamp_coords,
                                            stamp_len,
                                            sensor,
                                            exclude_blocked);
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
        assert(!reserver->movement);
        assert(!dest->reserved_by);
        const CoordPair newpos = get_physics_coords(
            reserver->x,
            reserver->y);
        assert(std::round(reserver->x) != x || std::round(reserver->y) != y);
        m_physics.move_stamp(
            oldpos.x, oldpos.y,
            newpos.x, newpos.y,
            reserver->info.stamp);
        // we must update phy here, because update() of GameObject will
        // otherwise attempt the move we just did, which will lead to awful
        // things happening
        reserver->phy = newpos;
    }
    cleanup_cell(dest);

    obj->x = x;
    obj->y = y;
    obj->cell = CoordPair{x, y};
    obj->phy = get_physics_coords(x, y);
    m_physics.place_object(
        obj->phy.x, obj->phy.y,
        obj.get(), initial_temperature, obj->heat_capacity);

    dest->here = std::move(obj);
    m_on_object_spawn.emit(*this, *dest->here);
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

void Level::step_singlebuffered_simulation()
{
    assert(!m_physics.running());
    m_ticks += 1;
    // level_log.logf(io::LOG_DEBUG, "tick %u", m_ticks);

    while (!m_timers.empty() && m_timers.top().trigger_at <= m_ticks)
    {
        Timer timer(m_timers.pop());
        assert(timer.func);
        LevelCell *const cell = (timer.x >= 0 && timer.y >= 0)
            ? get_cell(timer.x, timer.y)
            : nullptr;

        timer.func(*this, cell);
    }

    /* we do the update from bottom to top for better use of graaavity */
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

}

void Level::clear()
{
    assert(!m_physics.running());
    for (auto &cell: m_cells) {
        cleanup_cell(&cell);
    }
    m_physics.reset_unblocked_cells();
}


LevelOperator::~LevelOperator()
{

}

/* FIXME: deduplicate code with editor_level.cpp */

static std::tuple<bool, TileArgValue> deprotobufferize_arg_value(
        const mlio::TileArg &arg)
{
    switch (arg.value_case()) {
    case mlio::TileArg::kSval:
        return std::make_tuple(true, TileArgValue(arg.sval()));
    case mlio::TileArg::kFval:
        return std::make_tuple(true, TileArgValue(arg.fval()));
    case mlio::TileArg::kDval:
        return std::make_tuple(true, TileArgValue(arg.dval()));
    case mlio::TileArg::kIval:
        return std::make_tuple(true, TileArgValue(arg.ival()));
    case mlio::TileArg::kUival:
        return std::make_tuple(true, TileArgValue(arg.uival()));
    case mlio::TileArg::kBval:
        return std::make_tuple(true, TileArgValue(arg.bval()));
    case mlio::TileArg::kBinval:
        return std::make_tuple(true, TileArgValue(arg.binval()));
    case mlio::TileArg::kPval:
    {
        const auto pval = arg.pval();
        return std::make_tuple(
                    true,
                    TileArgValue(CoordPair(pval.x(), pval.y()))
                    );
    }
    default:
        return std::make_tuple(false, TileArgValue(0));
    }
}

bool load_level(Level &dest, const Tileset &tileset, std::istream &in)
{
    mlio::Level level;
    if (!level.ParseFromIstream(&in)) {
        level_log.logf(io::LOG_ERROR, "protobuf failed to parse");
        return false;
    }

    dest.clear();
    TileArgv argv;
    for (int i = 0; i < level.cells_size(); ++i) {
        const CoordInt y = i / level_width;
        const CoordInt x = i % level_width;
        if (y >= level_height || x >= level_width) {
            level_log.logf(io::LOG_ERROR, "level size out of bounds");
            dest.clear();
            return false;
        }

        const auto cell = level.cells(i);
        if (!cell.tile().size()) {
            // empty cell
            continue;
        }

        const QUuid id(cell.tile().c_str());
        argv.clear();
        for (const auto &arg: cell.argv()) {
            auto [ok, value] = deprotobufferize_arg_value(arg);
            if (ok) {
                argv.emplace(arg.type(), std::move(value));
            }
        }
        auto tile = tileset.make_tile(id, dest, argv);
        if (tile == nullptr) {
            level_log.logf(io::LOG_ERROR, "failed to instantiate tile %s from saved file", id.toString().data());
            dest.clear();
            return false;
        }
        // FIXME: adhere to initial temperature from level.
        dest.place_object(std::move(tile), x, y, 1.0f);
    }

    return true;
}
