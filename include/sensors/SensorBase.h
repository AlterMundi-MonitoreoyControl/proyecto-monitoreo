#pragma once
#include "ISensor.h"
#include <Arduino.h>   // ESP.getEfuseMac()

/**
 * SensorBase — mixin that provides stable SensorKey identity and a
 * per-sensor monotonic read counter for all concrete sensor classes.
 *
 * Key design decisions:
 *   - deviceId  = last byte of EFuse MAC → unique per board, no config needed.
 *   - sensorId  = stable physical address passed at construction:
 *                   pin number       for ADC/GPIO sensors (Capacitive, HD38)
 *                   I2C address      for I2C sensors (SCD30=0x61, BME280=0x76/0x77)
 *                   Modbus address   for RS485 sensors (ModbusTH, ModbusSoil)
 *                   last addr byte   for OneWire sensors
 *   - counter   = incremented every time _fillReading() is called.
 *                 ControlMediator rejects readings with counter ≤ stored.
 *
 * setSensorId() is intentionally absent — IDs must be stable at construction.
 */
enum class SensorClass : uint8_t {
    SYSTEM       = 0x00,
    ANALOG_ADC   = 0x10,
    I2C_BUS      = 0x20,
    RS485_MODBUS = 0x30,
    ONE_WIRE     = 0x40,
    VIRTUAL      = 0x50
};

class SensorBase {
public:
    explicit SensorBase(SensorClass sclass, uint8_t physicalId)
        : _key({_localDeviceId(), (uint16_t)(((uint16_t)sclass << 8) | physicalId), 0}), _counter(0) {}

    SensorKey getKey() const { return _key; }

    void updatePhysicalId(SensorClass sclass, uint8_t physicalId) {
        _key.sensorId = (uint16_t)(((uint16_t)sclass << 8) | physicalId);
    }

protected:
    SensorKey _key;
    uint32_t  _counter;

    /**
     * Fill a SensorReading with the current counter and value.
     * Increments counter before filling — always call after a successful read().
     * (Legacy for single-value sensors, varId defaults to 0)
     */
    bool _fillReading(SensorReading& out, float value) {
        ++_counter;
        out.key     = _key;
        out.value   = value;
        out.counter = _counter;
        return true;
    }

    /**
     * Helper to directly notify the mediator of a specific variable reading.
     * Use this for multi-variable sensors.
     */
    void _notify(ControlMediator& mediator, SensorVariable var, float value) {
        ++_counter;
        SensorReading r;
        r.key         = _key;
        r.key.varId   = (uint8_t)var;
        r.value       = value;
        r.counter     = _counter;
        mediator.onSensorReading(r);
    }

    /**
     * Returns the last byte of this device's EFuse MAC.
     * Used once at construction; result is stable for the lifetime of the board.
     */
    static uint8_t _localDeviceId() {
        return (uint8_t)(ESP.getEfuseMac() & 0xFF);
    }
};
