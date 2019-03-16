#ifndef _ML_SAFE_WALL_OBJECT_H
#define _ML_SAFE_WALL_OBJECT_H

#include "logic/game_object.hpp"



class WallObject: public GameObject
{
public:
    WallObject(ObjectInfo &info, Level &level);

private:
    bool m_heater_enabled;
    float m_target_temperature;
    float m_energy_rate;

public:
    inline bool heater_enabled() const {
        return m_heater_enabled;
    }

    inline bool heater_target_temperature() const {
        return m_target_temperature;
    }

    inline bool heater_energy_rate() const {
        return m_energy_rate;
    }

    WallObject &set_heater_enabled(bool new_value);
    WallObject &set_heater_target_temperature(float new_value);
    WallObject &set_heater_energy_rate(float new_value);


    // GameObject interface
public:
    void update() override;
};

class SafeWallObject: public WallObject
{
public:
    explicit SafeWallObject(Level &level);

};

class RoundSafeWallObject: public WallObject
{
public:
    explicit RoundSafeWallObject(Level &level);

};


#endif
