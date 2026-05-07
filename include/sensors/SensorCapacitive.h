#ifndef SENSOR_CAPACITIVE_H
#define SENSOR_CAPACITIVE_H

#include "ISensor.h"
#include "IMoistureSensor.h"
#include "SensorBase.h"
#include <Arduino.h>
#include "../debug.h"

#define CAPACITIVE_PIN 34
#define ADC_MAX 4095
#define ADC_MIN 0

class SensorCapacitive : public SensorBase, public IMoistureSensor {
private:
    int pin;
    float moisture;
    int rawValue;
    bool active;
    int dryValue;
    int wetValue;

public:
    SensorCapacitive(int adcPin = CAPACITIVE_PIN, int dry = ADC_MAX, int wet = ADC_MIN)
        : SensorBase(SensorClass::ANALOG_ADC, (uint8_t)(adcPin >= 0 ? adcPin : 0xFF)), pin(adcPin), moisture(0), rawValue(0), active(false), dryValue(dry), wetValue(wet) {}

    bool init() override {
        pinMode(pin, INPUT);
        analogReadResolution(12);              // 12-bit → 0-4095
        analogSetAttenuation(ADC_ATTENDB_MAX); // full 0-3.3V range
        active = true;
        DBG_INFO("[Capacitive] pin %d, 12-bit, 3.3V range OK\n", pin);
        return true;
    }

    bool dataReady() override {
        return active;
    }

    bool read() override {
        if (!active) return false;

        rawValue = analogRead(pin);
        moisture = map(rawValue, dryValue, wetValue, 0, 100);
        moisture = constrain(moisture, 0, 100);

        DBG_VERBOSE("[Capacitive] Raw=%d M=%.1f%% \tkey=0x%llx \n", rawValue, moisture, (unsigned long long)this->getKey().toU32());
        return true;
    }

    int getRawValue() const { return rawValue; }
    int getDryValue() const { return dryValue; }
    int getWetValue() const { return wetValue; }
    int getPin()      const { return pin; }

    // IMoistureSensor
    float getMoisture() override { return moisture; }

    const char* getSensorType() override { return "Capacitive"; }

    const char* getSensorID() override {
        static char idString[16];
        snprintf(idString, sizeof(idString), "m-adc-%d", pin);
        return idString;
    }

    const char* getMeasurementsString() override {
        static char measString[32];
        snprintf(measString, sizeof(measString), "moisture=%.1f", moisture);
        return measString;
    }

    bool isActive() override { return active; }

    void setCalibration(int dry, int wet) {
        dryValue = dry;
        wetValue = wet;
        DBG_INFO("[Capacitive] Cal: dry=%d wet=%d\n", dry, wet);
    }

    // ── Mediator interface ────────────────────────────────────────────────
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (!active) return;
        _notify(mediator, SensorVariable::MOISTURE, moisture);
        _notify(mediator, SensorVariable::RAW_ADC, rawValue);
    }
};

#endif // SENSOR_CAPACITIVE_H
