#pragma once
#include "ISensor.h"
#include "ITemperatureSensor.h"
#include "SensorBase.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

/**
 * InternalTempSensor — Lee la temperatura interna del núcleo (core) del ESP32.
 */
class InternalTempSensor : public ITemperatureSensor, public SensorBase {
public:
    InternalTempSensor() : SensorBase(SensorClass::SYSTEM, 0), _temp(0.0f) {}

    bool init() override { return true; }
    bool dataReady() override { return true; }
    
    bool read() override {
#ifdef ARDUINO
        _temp = temperatureRead(); // Función nativa del ESP32
#else
        _temp = 45.0f; // Mock para tests en PC (native_test)
#endif
        return true;
    }
    
    bool isActive() override { return true; }
    
    const char* getSensorType() override { return "InternalTemp"; }
    const char* getSensorID() override { return "sys-temp"; }
    
    const char* getMeasurementsString() override {
        static char buf[32];
        snprintf(buf, sizeof(buf), "temp=%.1f", _temp);
        return buf;
    }
    
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (!isActive()) return;
        _notify(mediator, SensorVariable::TEMPERATURE, _temp);
    }
    float getTemperature() override { return _temp; }

private:
    float _temp;
};