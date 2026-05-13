#ifndef SENSOR_FACTORY_H
#define SENSOR_FACTORY_H

#include "ISensor.h"

// Include all sensor implementations
#if defined(SENSOR_TYPE_SCD30)
  #include "SensorSCD30.h"
#elif defined(SENSOR_TYPE_CAPACITIVE)
  #include "SensorCapacitive.h"
#elif defined(SENSOR_TYPE_BME280)
  #include "SensorBME280.h"
#elif defined(SENSOR_TYPE_MODBUS_TH)
  #include "ModbusTHSensor.h"
#elif defined(MODO_SIMULACION)
  #include "SensorSimulated.h"
#else
  #error "No sensor type defined! Use -DSENSOR_TYPE_SCD30, -DSENSOR_TYPE_CAPACITIVE, -DSENSOR_TYPE_BME280, -DSENSOR_TYPE_MODBUS_TH, or -DMODO_SIMULACION"
#endif

class SensorFactory {
public:
    static ISensor* createSensor() {
        #if defined(SENSOR_TYPE_SCD30)
            return new SensorSCD30();
        #elif defined(SENSOR_TYPE_CAPACITIVE)
            return new SensorCapacitive();
        #elif defined(SENSOR_TYPE_BME280)
            return new SensorBME280();
        #elif defined(SENSOR_TYPE_MODBUS_TH)
            return new ModbusTHSensor();
        #elif defined(MODO_SIMULACION)
            return new SensorSimulated();
        #else
            return nullptr;
        #endif
    }
};

#endif // SENSOR_FACTORY_H
