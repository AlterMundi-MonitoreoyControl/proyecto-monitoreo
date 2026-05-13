#ifndef SENSOR_BME280_H
#define SENSOR_BME280_H

#include "ISensor.h"
#include "ITemperatureSensor.h"
#include "IHumiditySensor.h"
#include "IPressureSensor.h"
#include "SensorBase.h"
#include <Adafruit_BME280.h>
#include "../debug.h"

class SensorBME280 : public SensorBase, public ITemperatureSensor, public IHumiditySensor, public IPressureSensor {
private:
    Adafruit_BME280 bme;
    bool active;
    float temperature;
    float humidity;
    float pressure;
    uint8_t address = 0x76;

public:
    SensorBME280() : SensorBase(SensorClass::I2C_BUS, 0x76), active(false), temperature(999), humidity(100), pressure(0) {}
    //TODO: error en la clase base... no actualiza el sensor id
    bool init() override {
        active = bme.begin(0x76);
        if (!active) {
            active = bme.begin(0x77);
            if (active) {
                address = 0x77;
                updatePhysicalId(SensorClass::I2C_BUS, 0x77);  // update stable ID to actual I2C addr
            }
        }
        if (!active) {
            DBG_ERROR("[BME280] Init failed\n");
        } else {
            DBG_INFO("[BME280] OK addr=0x%02X\n", address);
            bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                          Adafruit_BME280::SAMPLING_X2,
                          Adafruit_BME280::SAMPLING_X16,
                          Adafruit_BME280::SAMPLING_X1,
                          Adafruit_BME280::FILTER_X16,
                          Adafruit_BME280::STANDBY_MS_500);
        }
        return active;
    }

    bool dataReady() override {
        return active;
    }

    bool read() override {
        if (!active) return false;

        temperature = bme.readTemperature();
        humidity = bme.readHumidity();
        pressure = bme.readPressure() / 100.0F;  // Convert Pa to hPa

        if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
            DBG_ERROR("[BME280] Read error\n");
            return false;
        }
        DBG_VERBOSE("[BME280] Read: temp=%.1f hum=%.1f pres=%.1f\n", temperature, humidity, pressure);
        return true;
    }

    // ITemperatureSensor
    float getTemperature() override { return temperature; }

    // IHumiditySensor
    float getHumidity() override { return humidity; }

    // IPressureSensor
    float getPressure() override { return pressure; }

    const char* getSensorType() override { return "BME280"; }

    const char* getSensorID() override {
        static char idString[16];
        snprintf(idString, sizeof(idString), "thp-i2c-%u", (unsigned)address);
        return idString;
    }

    const char* getMeasurementsString() override {
        static char measString[64];
        snprintf(measString, sizeof(measString), "temp=%.1f,hum=%.1f,press=%.1f", temperature, humidity, pressure);
        return measString;
    }

    bool isActive() override { return active; }

    // ── Mediator interface ────────────────────────────────────────────────
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (!active) return;
        _notify(mediator, SensorVariable::TEMPERATURE, temperature);
        _notify(mediator, SensorVariable::HUMIDITY, humidity);
        _notify(mediator, SensorVariable::PRESSURE, pressure);
    }
};

#endif // SENSOR_BME280_H
