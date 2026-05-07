#pragma once
#include <stdint.h>

/**
 * SensorKey — identidad compuesta de una magnitud de sensor.
 *
 * deviceId : last byte of the device's EFuse MAC (ESP.getEfuseMac() & 0xFF).
 *            Unique per physical board; 0x00 is fallback only.
 * sensorId : stable within a device — derived from the sensor's physical
 *            address (pin, I2C addr, Modbus addr, OneWire last byte).
 *            NOT a sequential registration index.
 *
 * toU32()  : encodes key as uint32_t for use as hash index in the
 *            ControlMediator state-store.
 */
enum class SensorVariable : uint8_t {
    TEMPERATURE = 0,
    HUMIDITY    = 1,
    CO2         = 2,
    MOISTURE    = 3,
    PRESSURE    = 4,
    EC          = 5,
    PH          = 6,
    NITROGEN    = 7,
    PHOSPHORUS  = 8,
    POTASSIUM   = 9,
    RAW_ADC     = 10,
    DIGITAL_IN_1= 11,
    DIGITAL_IN_2= 12,
    UNKNOWN     = 255
};

struct SensorKey {
    uint8_t  deviceId;
    uint16_t sensorId;
    uint8_t  varId;

    bool operator==(const SensorKey& o) const {
        return deviceId == o.deviceId && sensorId == o.sensorId && varId == o.varId;
    }
    bool operator!=(const SensorKey& o) const { return !(*this == o); }

    /** [31:24]=varId [23:16]=deviceId [15:0]=sensorId */
    uint32_t toU32() const {
        return ((uint32_t)varId << 24) | ((uint32_t)deviceId << 16) | (uint32_t)sensorId;
    }
};

/**
 * SensorReading — a single sample from one sensor magnitude.
 *
 * key     : which sensor on which device.
 * value   : reading in the sensor's native unit.
 * counter : monotonic read counter incremented by SensorBase::_fillReading().
 *           Does NOT require time-sync. ControlMediator uses it to discard
 *           stale/out-of-order readings.
 *           For remote ESP-NOW nodes, use the packet sequence field.
 */
struct SensorReading {
    SensorKey key;
    float     value;
    uint32_t  counter;
};
