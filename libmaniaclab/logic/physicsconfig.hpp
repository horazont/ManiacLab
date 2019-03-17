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

static inline SimFloat air_thermal_capacity(const SimFloat pressure) {
    return pressure * SimFloat(1.);
}

#endif
