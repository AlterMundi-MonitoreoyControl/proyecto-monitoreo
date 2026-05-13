#ifndef IPRESSURE_SENSOR_H
#define IPRESSURE_SENSOR_H

#include "ISensor.h"

/**
 * Interface for sensors that measure atmospheric pressure.
 * Use dynamic_cast<IPressureSensor*>(sensor) to check capability.
 */
class IPressureSensor : public virtual ISensor {
public:
    virtual ~IPressureSensor() = default;

    // Pressure in hPa
    virtual float getPressure() = 0;
};

#endif // IPRESSURE_SENSOR_H
