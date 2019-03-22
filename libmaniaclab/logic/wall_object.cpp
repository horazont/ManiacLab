#include "wall_object.hpp"

#include <iostream>

#include <ffengine/math/algo.hpp>

static const CellStamp squarewall_object_stamp(
    {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
    }
);

static const CellStamp roundwall_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static const CellStamp heater_stamp_raw(
    {
        false, false, false, false, false,
        false, false, true, false, false,
        false, true, true, true, false,
        false, false, true, false, false,
        false, false, false, false, false,
    }
);

static const Stamp heater_stamp(heater_stamp_raw);

static ObjectInfo safewallsq_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    true,
    0.0,
    squarewall_object_stamp);

static ObjectInfo safewallrd_object_info(
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    0.5,
    roundwall_object_stamp);

/* WallObject */

WallObject::WallObject(ObjectInfo &info, Level &level):
    GameObject(info, level, heat_capacity_stone),
    m_heater_enabled(false),
    m_target_temperature(1.f),
    m_energy_rate(1.f)
{

}

WallObject &WallObject::set_heater_enabled(bool new_value)
{
    m_heater_enabled = new_value;
    return *this;
}

WallObject &WallObject::set_heater_target_temperature(SimFloat new_value)
{
    m_target_temperature = new_value;
    return *this;
}

WallObject &WallObject::set_heater_energy_rate(SimFloat new_value)
{
    m_energy_rate = new_value;
    return *this;
}

void WallObject::update()
{
    GameObject::update();
    if (!m_heater_enabled) {
        return;
    }

    const SimFloat avg_heat_energy = level.measure_stamp_avg(
                phy.x, phy.y,
                heater_stamp,
                [](const LabCell &cell){return cell.heat_energy;});
    const SimFloat target_avg_heat_energy = m_target_temperature * heat_capacity;
    const SimFloat difference = target_avg_heat_energy - avg_heat_energy;
    const SimFloat change = clamp(difference, -m_energy_rate, m_energy_rate);
    const SimFloat new_heat_energy = avg_heat_energy + change;
    const SimFloat new_temperature = new_heat_energy / heat_capacity;
    // üstd::cout << "heater change: " << change << "; avg_U = " << avg_heat_energy << "; target_U = " << target_avg_heat_energy << "; new_U = " << new_heat_energy << "; new_T = " << new_temperature << std::endl;

    level.physics().apply_temperature_stamp(phy.x, phy.y, heater_stamp, new_temperature);
}


/* SafeWallObject */

SafeWallObject::SafeWallObject(Level &level):
    WallObject(safewallsq_object_info, level)
{

}

RoundSafeWallObject::RoundSafeWallObject(Level &level):
    WallObject(safewallrd_object_info, level)
{

}
