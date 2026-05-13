#ifndef IHUMIDITY_SENSOR_H
#define IHUMIDITY_SENSOR_H

#include "ISensor.h"

/**
 * Interface for sensors that measure air humidity.
 * Use dynamic_cast<IHumiditySensor*>(sensor) to check capability.
 */
class IHumiditySensor : public virtual ISensor {
public:
    virtual ~IHumiditySensor() = default;

    // Air humidity percentage (0-100%)
    virtual float getHumidity() = 0;
};

#endif // IHUMIDITY_SENSOR_H
