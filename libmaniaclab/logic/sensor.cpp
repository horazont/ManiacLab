#include "sensor.hpp"

#include "game_object.hpp"

#include "level.hpp"

/* Trigger */

Trigger::Trigger():
    inverted(false),
    low_threshold(NAN),
    high_threshold(NAN),
    m_is_firing(false)
{

}


void Trigger::update(const SimFloat value)
{
    const bool raw_now_firing = (!(low_threshold > value) && !(high_threshold < value));
    const bool now_firing = (inverted ? !raw_now_firing : raw_now_firing);
    if (now_firing && !m_is_firing) {
        if (rising_edge) {
            rising_edge();
        }
    } else if (!now_firing && m_is_firing) {
        if (falling_edge) {
            falling_edge();
        }
    }
    if (now_firing) {
        if (firing) {
            firing();
        }
    }
    m_is_firing = now_firing;
}


/* AbstractMeasurementSensor */

void AbstractMeasurementSensor::process(const SimFloat current_value)
{
    for (auto &trigger: m_triggers) {
        trigger->update(current_value);
    }
}

void AbstractMeasurementSensor::update()
{
    process(measure());
}

Trigger &AbstractMeasurementSensor::new_trigger()
{
    auto trigger = std::make_unique<Trigger>();
    Trigger &result = *trigger;
    m_triggers.emplace_back(std::move(trigger));
    return result;
}

void AbstractMeasurementSensor::remove_trigger(Trigger &trigger)
{
    for (auto iter = m_triggers.begin();
         iter != m_triggers.end();
         ++iter)
    {
        auto &trigger_ptr = *iter;
        if (&trigger == trigger_ptr.get()) {
            m_triggers.erase(iter);
            return;
        }
    }
}



/* MeasurementSensor */

MeasurementSensor::MeasurementSensor(
        GameObject &object,
        MeasurementSensor::MeasurementFunc &&sensor):
    m_object(object),
    m_func(sensor)
{

}

SimFloat MeasurementSensor::measure() const
{
    return m_object.level.measure_object_avg(
                m_object,
                m_func);
}
