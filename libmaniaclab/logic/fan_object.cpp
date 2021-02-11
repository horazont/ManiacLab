#include "logic/fan_object.hpp"

#include "level.hpp"

static const CellStamp horiz_fan_stamp(
        {
            false, true, true, true, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, false, false, false, false,
            false, true, true, true, false,
        }
);

static const CellStamp vert_fan_stamp(
        {
            false, false, false, false, false,
            true, false, false, false, true,
            true, false, false, false, true,
            true, false, false, false, true,
            false, false, false, false, false,
        }
);

static const CellStamp horiz_fan_effect_stamp_raw(
        {
            false, false, false, false, false,
            false, false, true, false, false,
            false, false, true, false, false,
            false, false, true, false, false,
            false, false, false, false, false,
        }
);

static Stamp horiz_fan_effect_stamp(horiz_fan_effect_stamp_raw);

static const CellStamp vert_fan_effect_stamp_raw(
        {
            false, false, false, false, false,
            false, false, false, false, false,
            false, true, true, true, false,
            false, false, false, false, false,
            false, false, false, false, false,
        }
);

static Stamp vert_fan_effect_stamp(vert_fan_effect_stamp_raw);

static ObjectInfo horiz_fan_info(
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    0.0,
    horiz_fan_stamp);

static ObjectInfo vert_fan_info(
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    0.0,
    vert_fan_stamp);

FanObject::FanObject(const ObjectInfo &info,
                     Level &level,
                     const Stamp &effect_stamp,
                     float intensity,
                     float turbulence_magnitude,
                     float turbulence_offset):
    GameObject(info, level, 2.f),
    m_effect_stamp(effect_stamp),
    m_turbulence_distribution(-turbulence_magnitude * M_PI_4f32, turbulence_magnitude * M_PI_4f32),
    m_intensity(intensity),
    m_enable_turbulence(turbulence_magnitude > 1e-6f),
    m_turbulence_offset(turbulence_offset)

{

}

void FanObject::update()
{
    GameObject::update();

    const float deviation = (m_enable_turbulence ? m_turbulence_distribution(m_rnge) : 0.f) + m_turbulence_offset;
    Vector2f dir(m_intensity * std::cos(deviation),
                 m_intensity * std::sin(deviation));

    level.physics().apply_flow_stamp(
                phy.x, phy.y,
                m_effect_stamp,
                dir,
                0.2f);
}


HorizFanObject::HorizFanObject(Level &level, float intensity, float turbulence):
    FanObject(horiz_fan_info, level, horiz_fan_effect_stamp, intensity, turbulence, 0.f)
{

}

VertFanObject::VertFanObject(Level &level, float intensity, float turbulence):
    FanObject(vert_fan_info, level, vert_fan_effect_stamp, intensity, turbulence, M_PI_2f32)
{

}
