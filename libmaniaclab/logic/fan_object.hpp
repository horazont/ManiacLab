#ifndef _ML_FAN_OBJECT_HPP
#define _ML_FAN_OBJECT_HPP

#include <random>

#include "logic/game_object.hpp"


class FanObject: public GameObject
{
protected:
    FanObject(const ObjectInfo &info, Level &level,
              const Stamp &effect_stamp,
              float intensity,
              float turbulence_magnitude,
              float turbulence_offset);

protected:
    const Stamp &m_effect_stamp;

private:
    std::mt19937 m_rnge;
    std::uniform_real_distribution<float> m_turbulence_distribution;
    float m_intensity;
    bool m_enable_turbulence;
    float m_turbulence_offset;

public:
    void update() override;
};


class HorizFanObject: public FanObject
{
public:
    HorizFanObject(Level &level, float intensity, float turbulence);

};


class VertFanObject: public FanObject
{
public:
    VertFanObject(Level &level, float intensity, float turbulence);

};


#endif
