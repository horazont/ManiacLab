#ifndef _ML_FOG_OBJECT_HPP
#define _ML_FOG_OBJECT_HPP

#include "logic/game_object.hpp"


class FogObject: public GameObject
{
public:
    FogObject(Level &level,
              SimFloat intensity,
              SimFloat temperature);

private:
    SimFloat m_intensity;
    SimFloat m_temperature;
    Stamp m_effect_stamp;
    Stamp m_temperature_stamp;

public:
    void update() override;

};

#endif
