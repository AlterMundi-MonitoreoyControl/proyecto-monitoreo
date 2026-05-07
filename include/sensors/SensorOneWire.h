#ifndef SENSOR_ONEWIRE_H
#define SENSOR_ONEWIRE_H

#include "ISensor.h"
#include "ITemperatureSensor.h"
#include "SensorBase.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../debug.h"

class SensorOneWire : public SensorBase, public ITemperatureSensor {
private:
    DallasTemperature* dallas;
    DeviceAddress address;
    String addressStr;
    float temperature;
    int deviceIndex;
    bool active;

public:
    SensorOneWire(DallasTemperature* dt, DeviceAddress addr, int idx)
        : SensorBase(SensorClass::ONE_WIRE, addr[7]),   // last byte of 8-byte OneWire address = stable ID
          dallas(dt), deviceIndex(idx), temperature(-127), active(false) {
        memcpy(address, addr, 8);
        addressStr = "";
        for (uint8_t i = 0; i < 8; i++) {
            if (address[i] < 16) addressStr += "0";
            addressStr += String(address[i], HEX);
        }
        addressStr.toUpperCase();
    }

    bool init() override {
        if (dallas) {
            dallas->setResolution(address, 12);
            active = true;
            DBG_INFO("[OneWire] %s OK\n", addressStr.c_str());
            return true;
        }
        return false;
    }

    bool dataReady() override {
        return active;
    }

    bool read() override {
        if (!active || !dallas) return false;

        float temp = dallas->getTempC(address);

        if (temp != DEVICE_DISCONNECTED_C && temp != 85.0) {
            temperature = temp;
            DBG_VERBOSE("[OneWire] %s: %.1f\n", addressStr.c_str(), temperature);
            return true;
        }

        return false;
    }

    // ITemperatureSensor
    float getTemperature() override { return temperature; }

    const char* getSensorType() override { return "OneWire"; }

    const char* getSensorID() override {
        static char sensorId[32];
        size_t len = addressStr.length();
        String last4 = (len > 4) ? addressStr.substring(len - 4) : addressStr;
        snprintf(sensorId, sizeof(sensorId), "t-1w-%s", last4.c_str());
        return sensorId;
    }

    const char* getMeasurementsString() override {
        static char measString[32];
        snprintf(measString, sizeof(measString), "temp=%.1f", temperature);
        return measString;
    }

    bool isActive() override { return active; }

    // ── Mediator interface ────────────────────────────────────────────────
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (!active) return;
        _notify(mediator, SensorVariable::TEMPERATURE, temperature);
    }
};

#endif // SENSOR_ONEWIRE_H
