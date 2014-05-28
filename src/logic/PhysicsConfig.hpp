/**********************************************************************
File name: PhysicsConfig.hpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#ifndef _ML_PHYSICS_CONFIG_H
#define _ML_PHYSICS_CONFIG_H

#include "Types.hpp"

const CoordInt subdivision_count = 5;
const CoordInt half_offset = 2;
const CoordInt cell_stamp_length = subdivision_count*subdivision_count;
const double airtempcoeff_per_pressure = 1.0;
const CoordInt level_width = 50;
const CoordInt level_height = 50;

static constexpr TickCounter EXPLOSION_TRIGGER_TIMEOUT = 50;
static constexpr TickCounter EXPLOSION_BLOCK_LIFETIME = 150;

static constexpr float FIRE_PARTICLE_TEMPERATURE_RISE = 0.01;

struct SimulationConfig {
    double flow_friction;
    double flow_damping;
    double convection_friction;
    double heat_flow_friction;
    double fog_flow_friction;

    SimulationConfig(
            const double flow_friction,
            const double flow_damping,
            const double convection_friction,
            const double heat_flow_friction,
            const double fog_flow_friction):
        flow_friction(flow_friction),
        flow_damping(flow_damping),
        convection_friction(convection_friction),
        heat_flow_friction(heat_flow_friction),
        fog_flow_friction(fog_flow_friction)
    {

    }

    SimulationConfig(const SimulationConfig &ref):
        flow_friction(ref.flow_friction),
        flow_damping(ref.flow_damping),
        convection_friction(ref.convection_friction),
        heat_flow_friction(ref.heat_flow_friction),
        fog_flow_friction(ref.fog_flow_friction)
    {

    }

    inline SimulationConfig& operator =(const SimulationConfig &ref)
    {
        flow_friction = ref.flow_friction;
        flow_damping = ref.flow_damping;
        convection_friction = ref.convection_friction;
        heat_flow_friction = ref.heat_flow_friction;
        fog_flow_friction = ref.fog_flow_friction;
        return *this;
    }
};

#endif
