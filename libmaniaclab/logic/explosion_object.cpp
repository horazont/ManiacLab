#include "explosion_object.hpp"

static constexpr float EXPLOSION_PRESSURE = 1.5f;
static constexpr float EXPLOSION_TEMPERATURE = 1000.f;
static constexpr float EXPLOSION_FLOW_INTENSITY = 10.f;

static const CellStamp explosion_object_stamp(
    {
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
    }
);

static const CellStamp explosion_pressure_spawn_stamp_raw(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static const Stamp explosion_pressure_spawn_stamp(explosion_pressure_spawn_stamp_raw);

static const CellStamp explosion_pressure_flow_left_stamp_raw(
    {
            false, false, false, false, false,
            true, false, false, false, false,
            true, false, false, false, false,
            true, false, false, false, false,
            false, false, false, false, false,
    }
);

static const CellStamp explosion_pressure_flow_right_stamp_raw(
    {
            false, false, false, false, false,
            false, false, false, false, true,
            false, false, false, false, true,
            false, false, false, false, true,
            false, false, false, false, false,
    }
);

static const CellStamp explosion_pressure_flow_top_stamp_raw(
    {
            false, true, true, true, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, false, false, false, false,
    }
);

static const CellStamp explosion_pressure_flow_bottom_stamp_raw(
    {
            false, false, false, false, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, true, true, true, false,
    }
);

static const std::array<std::tuple<Stamp, Vector2f>, 4> explosion_pressure_flow_stamps{{
        std::make_tuple(Stamp(explosion_pressure_flow_top_stamp_raw), Vector2f(0, -1)),
        std::make_tuple(Stamp(explosion_pressure_flow_left_stamp_raw), Vector2f(-1, 0)),
        std::make_tuple(Stamp(explosion_pressure_flow_right_stamp_raw), Vector2f(1, 0)),
        std::make_tuple(Stamp(explosion_pressure_flow_bottom_stamp_raw), Vector2f(0, 1))
}};

static ObjectInfo explosion_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    0.5,
    explosion_object_stamp);


/* ExplosionView */

ExplosionView::ExplosionView():
    ObjectView()
{

}

/* ExplosionObject */

ExplosionObject::ExplosionObject(Level &level):
    GameObject(explosion_object_info, level, 1.f),
    die_at(level.get_ticks() + EXPLOSION_BLOCK_LIFETIME),
    ctr(0)
{

}

void ExplosionObject::update()
{
    GameObject::update();
    const float rel_time = static_cast<float>(ctr) / EXPLOSION_BLOCK_LIFETIME;
    auto &physics = level.physics();
    if (ctr == 0) {
        physics.apply_pressure_stamp(phy.x, phy.y, explosion_pressure_spawn_stamp, EXPLOSION_PRESSURE);
        physics.apply_temperature_stamp(phy.x, phy.y, explosion_pressure_spawn_stamp, EXPLOSION_TEMPERATURE);
    }
    const float cos_bare = std::cos(rel_time * M_PI_2f32);
    const float cos_factor = cos_bare;
    for (auto &info: explosion_pressure_flow_stamps) {
        physics.apply_flow_stamp(phy.x, phy.y, std::get<0>(info), std::get<1>(info) * cos_factor * EXPLOSION_FLOW_INTENSITY);
    }

    ctr += 1;
    if (ticks >= die_at) {
        destruct_self();
    }
}
