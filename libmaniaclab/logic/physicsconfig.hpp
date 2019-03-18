#ifndef _ML_PHYSICS_CONFIG_H
#define _ML_PHYSICS_CONFIG_H

#include "logic/types.hpp"

const CoordInt subdivision_count = 5;
const CoordInt half_offset = 2;
const CoordInt cell_stamp_length = subdivision_count*subdivision_count;
const CoordInt level_width = 52;
const CoordInt level_height = 52;

static constexpr TickCounter EXPLOSION_TRIGGER_TIMEOUT = 50;
static constexpr TickCounter EXPLOSION_BLOCK_LIFETIME = 150;

static constexpr SimFloat FIRE_PARTICLE_TEMPERATURE_RISE = 1.;
static constexpr SimFloat KELVIN_TO_CELSIUS = 273.15;

static constexpr SimFloat default_temperature = KELVIN_TO_CELSIUS + 25;
static constexpr SimFloat default_pressure = 1.;

static constexpr SimFloat heat_capacity_stone = 0.75e3; // J/K
static constexpr SimFloat heat_capacity_metal = 0.49e3; // J/K

static inline float air_thermal_conductivity(const SimFloat pressure) {
    // this is a fancy approximation, based on
    // https://www.researchgate.net/figure/Air-thermal-conductivity-as-a-function-of-pressure_fig1_230959207
    // unit is J/(Kms)
    return (1./(1.+std::exp(-pressure/0.07)) - 0.5) * 0.06;
}

/**
 * Approximate the thermal capacity of a non-blocked (air) cell by pressure and
 * temperature.
 *
 * This is an awful hack because the heat capacity depends on the temperature,
 * but since the simulation operates on internal energy, we have no way to
 * calculate the temperature without knowing the thermal capacity.
 *
 * Under ideal gas law, the thermal capacity U/T is given as c_vnR, and we do
 * not know n without knowing P, V (constant) and T.
 *
 * @param pressure Absolute pressure P in bar.
 * @return thermal capacity in Joule per Kelvin
 */
static inline SimFloat air_thermal_capacity(const SimFloat pressure)
{
    return 1e-3f * pressure;

    // this is mostly accurate for low pressures (<10 bar)
    static constexpr SimFloat air_specific_heat = 1.07e-3;  // J/(K kg)
    // we assume a constant temperature; we reserve the second argument for
    // the actual temperature in case we want to go down that rabbit hole in
    // the future
    static constexpr SimFloat air_temperature = default_temperature;  // K
    static constexpr SimFloat air_specific_gas_constant = 287.058;  // J/(K kg)
    static constexpr SimFloat cell_size = 1.e6;  // m^3

    // 1e-5 = Pa/bar
    const float air_density = (pressure * 1e-5) / (air_specific_gas_constant * air_temperature);  // kg/m^3
    const float air_mass = air_density * cell_size;  // kg

    return air_specific_heat * air_mass;  // J/K
}

#endif
