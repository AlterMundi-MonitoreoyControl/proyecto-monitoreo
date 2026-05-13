#ifndef ITEMPERATURE_SENSOR_H
#define ITEMPERATURE_SENSOR_H

#include "ISensor.h"

/**
 * Interface for sensors that measure temperature.
 * Use dynamic_cast<ITemperatureSensor*>(sensor) to check capability.
 */
class ITemperatureSensor : public virtual ISensor {
public:
    virtual ~ITemperatureSensor() = default;

    // Temperature in Celsius
    virtual float getTemperature() = 0;
};

#endif // ITEMPERATURE_SENSOR_H
