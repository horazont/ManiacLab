#include "fog_object.hpp"

static const CellStamp fog_object_stamp(
    {
        false, false, false, false, false,
        false, false, true, false, false,
        false, true, true, true, false,
        false, false, true, false, false,
        false, false, false, false, false,
    }
);

static const CellStamp fog_effect_stamp(
    {
        false, false, true, false, false,
        false, true, false, true, false,
        true, false, false, false, true,
        false, true, false, true, false,
        false, false, true, false, false,
    }
);

static const CellStamp fog_temperature_stamp(
    {
        false, false, true, false, false,
        false, true, true, true, false,
        true, true, true, true, true,
        false, true, true, true, false,
        false, false, true, false, false,
    }
);

static ObjectInfo fog_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    0.0,
    2.0,
    fog_object_stamp);


FogObject::FogObject(Level &level, SimFloat intensity, SimFloat temperature):
    GameObject(fog_object_info, level),
    m_intensity(intensity),
    m_temperature(temperature),
    m_effect_stamp(fog_effect_stamp),
    m_temperature_stamp(fog_temperature_stamp)
{

}

void FogObject::update()
{
    GameObject::update();
    level.physics().apply_fog_effect_stamp(phy.x, phy.y, m_effect_stamp, m_intensity);
    level.physics().apply_temperature_stamp(phy.x, phy.y, m_temperature_stamp, m_temperature);
}
