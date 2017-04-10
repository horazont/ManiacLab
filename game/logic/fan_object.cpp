#include "logic/fan_object.hpp"

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
    1.0,
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
    1.0,
    vert_fan_stamp);

HorizFanObject::HorizFanObject(Level *level, float intensity):
    GameObject(horiz_fan_info, level),
    m_intensity(intensity)
{

}

void HorizFanObject::update()
{
    GameObject::update();
    level->physics().apply_flow_stamp(
                phy.x, phy.y,
                horiz_fan_effect_stamp,
                Vector2f(m_intensity, 0),
                0.2);
}

VertFanObject::VertFanObject(Level *level, float intensity):
    GameObject(vert_fan_info, level),
    m_intensity(intensity)
{

}

void VertFanObject::update()
{
    GameObject::update();
    level->physics().apply_flow_stamp(
                phy.x, phy.y,
                vert_fan_effect_stamp,
                Vector2f(m_intensity, 0),
                0.2);
}
