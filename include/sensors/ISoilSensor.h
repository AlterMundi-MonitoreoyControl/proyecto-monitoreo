#ifndef ISOIL_SENSOR_H
#define ISOIL_SENSOR_H

#include "ISensor.h"

/**
 * Extended interface for soil sensors with EC, pH, and NPK measurements.
 *
 * Sensors implementing this interface provide additional soil-specific
 * measurements beyond basic temperature and humidity.
 *
 * Use dynamic_cast<ISoilSensor*>(sensor) to check if a sensor supports
 * these extended measurements.
 */
class ISoilSensor : public virtual ISensor {
public:
    virtual ~ISoilSensor() = default;

    // Electrical Conductivity in μS/cm
    virtual float getEC() = 0;

    // pH value (0-14 scale)
    virtual float getPH() = 0;

    // Nitrogen content in mg/kg
    virtual int getNitrogen() = 0;

    // Phosphorus content in mg/kg
    virtual int getPhosphorus() = 0;

    // Potassium content in mg/kg
    virtual int getPotassium() = 0;
};

#endif // ISOIL_SENSOR_H
