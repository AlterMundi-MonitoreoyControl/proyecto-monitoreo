#ifndef IMOISTURE_SENSOR_H
#define IMOISTURE_SENSOR_H

#include "ISensor.h"

/**
 * Interface for sensors that measure soil moisture.
 * Use dynamic_cast<IMoistureSensor*>(sensor) to check capability.
 */
class IMoistureSensor : public virtual ISensor {
public:
    virtual ~IMoistureSensor() = default;

    // Soil moisture percentage (0-100%)
    virtual float getMoisture() = 0;
};

#endif // IMOISTURE_SENSOR_H
