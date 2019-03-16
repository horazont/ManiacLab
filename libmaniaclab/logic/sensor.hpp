#ifndef ML_SENSOR_H
#define ML_SENSOR_H

#include <functional>

#include "physics.hpp"

using SensorFunc = std::function<void(void)>;


struct Trigger
{
    Trigger();
    Trigger(const Trigger &ref) = delete;
    Trigger &operator=(const Trigger &ref) = delete;
    Trigger(Trigger &&ref) = default;
    Trigger &operator=(Trigger &&ref) = default;

    bool inverted;
    SimFloat low_threshold;
    SimFloat high_threshold;
    SensorFunc rising_edge;
    SensorFunc falling_edge;
    SensorFunc firing;

private:
    bool m_is_firing;

public:
    void update(const SimFloat value);

    inline bool is_firing() const
    {
        return m_is_firing;
    }

};


class Sensor
{

public:
    virtual void update() = 0;

};


class AbstractMeasurementSensor: public Sensor
{
private:
    std::vector<std::unique_ptr<Trigger> > m_triggers;

protected:
    virtual void process(const SimFloat current_value);

public:
    virtual SimFloat measure() const = 0;
    void update() override;

public:
    Trigger &new_trigger();
    void remove_trigger(Trigger &trigger);

};


class MeasurementSensor: public AbstractMeasurementSensor
{
public:
    using MeasurementFunc = std::function<SimFloat(const LabCell&)>;

public:
    MeasurementSensor() = delete;
    MeasurementSensor(GameObject &object, MeasurementFunc &&sensor);

private:
    GameObject &m_object;
    MeasurementFunc m_func;

public:
    SimFloat measure() const override;

};

#endif
