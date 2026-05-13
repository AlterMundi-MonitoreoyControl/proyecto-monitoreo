/**
 * test/testSensorIdentity.cpp
 *
 * Unit tests for sensor construction and stable SensorKey ID generation.
 *
 * Verifies:
 *   - deviceId = last byte of EFuse MAC (from stub)
 *   - sensorId set at construction from physical address
 *   - SensorKey.toU32() encoding
 *   - _fillReading() counter increments per call
 *   - Different sensor types produce distinct sensorIds
 *
 * Runs on native platform — no hardware, no Arduino SDK.
 */

#include <unity.h>
#include <cstdint>
#include <cstring>
#include <cmath>

// ── Arduino / ESP32 stubs ─────────────────────────────────────────────────────

#ifndef ARDUINO
#include "MockESP.h"
#define EXPECTED_MAC_BYTE 0xBB
#else
#include <Arduino.h>
#define EXPECTED_MAC_BYTE (ESP.getEfuseMac() & 0xFF)
#endif

// ── Minimal sensor-interface stubs needed by headers ────────────────────────
// (SensorBase only needs SensorKey.h; keep includes minimal)

#include "core/SensorKey.h"
#include "sensors/SensorBase.h"

// ── Concrete minimal sensor for identity tests ────────────────────────────────

/** A minimal ISensor that holds a stable sensorId and exposes _fillReading */
class StubSensor : public SensorBase {
public:
    explicit StubSensor(uint8_t sid) : SensorBase(SensorClass::VIRTUAL, sid), _lastValue(0) {}

    // ISensor interface (minimal stubs)
    bool init()       { return true; }
    bool dataReady()  { return true; }
    bool read()       { return true; }
    bool isActive()   { return true; }
    const char* getSensorType()         { return "stub"; }
    const char* getSensorID()           { return "stub-0"; }
    const char* getMeasurementsString() { return ""; }
    SensorKey   getKey() const          { return SensorBase::getKey(); }
    bool readValue(SensorReading& out)  { return _fillReading(out, _lastValue); }

    void setValue(float v) { _lastValue = v; }

private:
    float _lastValue;
};

// ── Tests ─────────────────────────────────────────────────────────────────────

void test_sensorkey_deviceId_from_mac() {
    // SensorBase derives deviceId from ESP.getEfuseMac() & 0xFF
    // Our stub returns 0x...AABB → last byte = 0xBB
    StubSensor s(0x61);
    TEST_ASSERT_EQUAL_HEX8(EXPECTED_MAC_BYTE, s.getKey().deviceId);
}

void test_sensorkey_sensorId_pin_based() {
    // ADC pin 34 sensor → sensorId = 34 = 0x22
    StubSensor s(34);
    TEST_ASSERT_EQUAL_UINT8(34, s.getKey().sensorId);
}

void test_sensorkey_sensorId_i2c_scd30() {
    // SCD30 I2C address = 0x61
    StubSensor s(0x61);
    TEST_ASSERT_EQUAL_HEX8(0x61, s.getKey().sensorId);
}

void test_sensorkey_sensorId_i2c_bme280() {
    // BME280 default I2C address = 0x76
    StubSensor s(0x76);
    TEST_ASSERT_EQUAL_HEX8(0x76, s.getKey().sensorId);
}

void test_sensorkey_sensorId_modbus_address() {
    // Modbus address 45
    StubSensor s(45);
    TEST_ASSERT_EQUAL_UINT8(45, s.getKey().sensorId);
}

void test_sensorkey_tou32_encoding() {
    // toU32() incorporates deviceId, sensorClass, and sensorId
    // With deviceId=0xBB, sensorClass=VIRTUAL, sensorId=0x61
    StubSensor s(0x61);
    SensorKey k = s.getKey();
    uint32_t expected = ((uint32_t)EXPECTED_MAC_BYTE << 16) | ((uint32_t)SensorClass::VIRTUAL << 8) | 0x61;
    TEST_ASSERT_EQUAL_UINT32(expected, k.toU32());
}

void test_sensorkey_equality() {
    StubSensor a(10);
    StubSensor b(10);
    StubSensor c(20);

    SensorKey ka = a.getKey();
    SensorKey kb = b.getKey();
    SensorKey kc = c.getKey();

    TEST_ASSERT_TRUE(ka == kb);   // same devId+sensorId
    TEST_ASSERT_FALSE(ka == kc);  // different sensorId
    TEST_ASSERT_TRUE(ka != kc);
}

void test_fill_reading_counter_increments() {
    StubSensor s(0x61);
    s.setValue(25.0f);

    SensorReading r1, r2, r3;
    s.readValue(r1);
    s.readValue(r2);
    s.readValue(r3);

    TEST_ASSERT_EQUAL_UINT32(1, r1.counter);
    TEST_ASSERT_EQUAL_UINT32(2, r2.counter);
    TEST_ASSERT_EQUAL_UINT32(3, r3.counter);
}

void test_fill_reading_value_correct() {
    StubSensor s(34);
    s.setValue(42.5f);

    SensorReading r;
    bool ok = s.readValue(r);

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 42.5f, r.value);
}

void test_fill_reading_key_matches_sensor() {
    StubSensor s(0x76);
    SensorReading r;
    s.readValue(r);

    TEST_ASSERT_EQUAL_HEX8(EXPECTED_MAC_BYTE, r.key.deviceId);
    TEST_ASSERT_EQUAL_HEX8(0x76, r.key.sensorId);
}

void test_two_sensors_distinct_ids() {
    // Two sensors with different physical addresses must produce different keys
    StubSensor pin34(34);     // ADC sensor on pin 34
    StubSensor modbus45(45);  // Modbus sensor at address 45

    TEST_ASSERT_FALSE(pin34.getKey() == modbus45.getKey());
    TEST_ASSERT_NOT_EQUAL(pin34.getKey().sensorId, modbus45.getKey().sensorId);
}

void test_onewire_last_byte_as_id() {
    // OneWire uses addr[7] — simulate with stub using that byte directly
    uint8_t owAddr[8] = {0x28, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xC4};
    StubSensor owSensor(owAddr[7]);  // last byte = 0xC4

    TEST_ASSERT_EQUAL_HEX8(0xC4, owSensor.getKey().sensorId);
}

void test_counter_independent_per_sensor() {
    StubSensor s1(10);
    StubSensor s2(20);

    SensorReading r;
    s1.readValue(r); s1.readValue(r); s1.readValue(r);  // counter = 3
    s2.readValue(r);                                      // counter = 1

    SensorReading from1, from2;
    s1.readValue(from1);  // should be 4
    s2.readValue(from2);  // should be 2

    TEST_ASSERT_EQUAL_UINT32(4, from1.counter);
    TEST_ASSERT_EQUAL_UINT32(2, from2.counter);
}

// ── Entry point ───────────────────────────────────────────────────────────────

void runSensorIdentityTests() {
    RUN_TEST(test_sensorkey_deviceId_from_mac);
    RUN_TEST(test_sensorkey_sensorId_pin_based);
    RUN_TEST(test_sensorkey_sensorId_i2c_scd30);
    RUN_TEST(test_sensorkey_sensorId_i2c_bme280);
    RUN_TEST(test_sensorkey_sensorId_modbus_address);
    RUN_TEST(test_sensorkey_tou32_encoding);
    RUN_TEST(test_sensorkey_equality);
    RUN_TEST(test_fill_reading_counter_increments);
    RUN_TEST(test_fill_reading_value_correct);
    RUN_TEST(test_fill_reading_key_matches_sensor);
    RUN_TEST(test_two_sensors_distinct_ids);
    RUN_TEST(test_onewire_last_byte_as_id);
    RUN_TEST(test_counter_independent_per_sensor);
}
