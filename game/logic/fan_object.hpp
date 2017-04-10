#ifndef _ML_FAN_OBJECT_HPP
#define _ML_FAN_OBJECT_HPP

#include "logic/game_object.hpp"


class HorizFanObject: public GameObject
{
public:
    HorizFanObject(Level *level, float intensity);

private:
    float m_intensity;

public:
    void update() override;

};


class VertFanObject: public GameObject
{
public:
    VertFanObject(Level *level, float intensity);

private:
    float m_intensity;

public:
    void update() override;

};


#endif
