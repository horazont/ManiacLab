#ifndef _ML_PHYSICS_CONFIG_H
#define _ML_PHYSICS_CONFIG_H

#include "logic/types.hpp"
const CoordInt subdivision_count = 5;
const CoordInt half_offset = 2;
const CoordInt cell_stamp_length = subdivision_count*subdivision_count;
const double airtempcoeff_per_pressure = 1.0;
const CoordInt level_width = 50;
const CoordInt level_height = 50;

static constexpr TickCounter EXPLOSION_TRIGGER_TIMEOUT = 50;
static constexpr TickCounter EXPLOSION_BLOCK_LIFETIME = 150;

static constexpr float FIRE_PARTICLE_TEMPERATURE_RISE = 0.01;


#endif
