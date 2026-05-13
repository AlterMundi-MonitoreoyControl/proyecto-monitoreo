#ifndef SENSOR_SIMULATED_H
#define SENSOR_SIMULATED_H

#include "ISensor.h"
#include "ITemperatureSensor.h"
#include "IHumiditySensor.h"
#include "ICO2Sensor.h"
#include "SensorBase.h"
#include <Arduino.h>
#include "../debug.h"

class SensorSimulated : public SensorBase, public ITemperatureSensor, public IHumiditySensor, public ICO2Sensor {
private:
    bool active;
    float temperature;
    float humidity;
    float co2;

public:
    SensorSimulated() : SensorBase(SensorClass::VIRTUAL, 0xFE), active(false), temperature(22.5), humidity(50), co2(400) {}

    bool init() override {
        active = true;
        DBG_INFO("[Simulated] OK - random mode\n");
        return true;
    }

    bool dataReady() override {
        return active;
    }

    bool read() override {
        if (!active) return false;

        // Generate random variations
        temperature = 22.5 + random(-100, 100) * 0.01;
        humidity = 50 + random(-500, 500) * 0.01;
        co2 = 400 + random(0, 200);
        DBG_VERBOSE("[Simulated] Read: temp=%.1f hum=%.1f co2=%.0f\n", temperature, humidity, co2);
        return true;
    }

    // ITemperatureSensor
    float getTemperature() override { return temperature; }

    // IHumiditySensor
    float getHumidity() override { return humidity; }

    // ICO2Sensor
    float getCO2() override { return co2; }

    const char* getSensorType() override { return "Simulated"; }
    const char* getSensorID() override { return "sim-001"; }

    const char* getMeasurementsString() override {
        static char measString[64];
        snprintf(measString, sizeof(measString), "temp=%.1f,hum=%.1f,co2=%.0f", temperature, humidity, co2);
        return measString;
    }

    bool isActive() override { return active; }

    // ── Mediator interface ────────────────────────────────────────────────
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (!active) return;
        _notify(mediator, SensorVariable::TEMPERATURE, temperature);
        _notify(mediator, SensorVariable::HUMIDITY, humidity);
        _notify(mediator, SensorVariable::CO2, co2);
    }
};

#endif // SENSOR_SIMULATED_H
