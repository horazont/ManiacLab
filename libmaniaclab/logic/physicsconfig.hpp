#ifndef _ML_PHYSICS_CONFIG_H
#define _ML_PHYSICS_CONFIG_H

#include "logic/types.hpp"
const CoordInt subdivision_count = 5;
const CoordInt half_offset = 2;
const CoordInt cell_stamp_length = subdivision_count*subdivision_count;
const float airtempcoeff_per_pressure = 1.0f;
const CoordInt level_width = 52;
const CoordInt level_height = 52;

static constexpr TickCounter EXPLOSION_TRIGGER_TIMEOUT = 50;
static constexpr TickCounter EXPLOSION_BLOCK_LIFETIME = 150;

static constexpr float FIRE_PARTICLE_TEMPERATURE_RISE = 1.f;
static constexpr float KELVIN_TO_CELSIUS = 273.15;

const float default_temperature = KELVIN_TO_CELSIUS + 25;
const float default_pressure = 1.f;

#endif
