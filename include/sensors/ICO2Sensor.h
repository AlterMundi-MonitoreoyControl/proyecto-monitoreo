#ifndef ICO2_SENSOR_H
#define ICO2_SENSOR_H

#include "ISensor.h"

/**
 * Interface for sensors that measure CO2 concentration.
 * Use dynamic_cast<ICO2Sensor*>(sensor) to check capability.
 */
class ICO2Sensor : public virtual ISensor {
public:
    virtual ~ICO2Sensor() = default;

    // CO2 concentration in ppm
    virtual float getCO2() = 0;
};

#endif // ICO2_SENSOR_H
