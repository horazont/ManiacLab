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

const CoordInt subdivisionCount = 5;
const CoordInt cellStampLength = subdivisionCount*subdivisionCount;
const double airTempCoeffPerPressure = 1.0;

struct SimulationConfig {
    double flowFriction;
    double flowDamping;
    double convectionFriction;
    double heatFlowFriction;

    SimulationConfig(
            const double flowFriction,
            const double flowDamping,
            const double convectionFriction,
            const double heatFlowFriction):
        flowFriction(flowFriction),
        flowDamping(flowDamping),
        convectionFriction(convectionFriction),
        heatFlowFriction(heatFlowFriction)
    {

    }

    SimulationConfig(const SimulationConfig &ref):
        flowFriction(ref.flowFriction),
        flowDamping(ref.flowDamping),
        convectionFriction(ref.convectionFriction),
        heatFlowFriction(ref.heatFlowFriction)
    {

    }

    inline SimulationConfig& operator =(const SimulationConfig &ref)
    {
        flowFriction = ref.flowFriction;
        flowDamping = ref.flowDamping;
        convectionFriction = ref.convectionFriction;
        heatFlowFriction = ref.heatFlowFriction;
        return *this;
    }
};

#endif
