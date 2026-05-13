#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "debug.h"
#include "sensors/HD38Sensor.h"
#include "sensors/ISensor.h"
#include "sensors/SensorBME280.h"
#include "sensors/SensorCapacitive.h"
#include "sensors/SensorOneWire.h"
#include "sensors/SensorSCD30.h"
#include "sensors/SensorSimulated.h"
#include "sensors/InternalTempSensor.h"
#include "core/ControlMediator.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <vector>

#ifdef ENABLE_RS485
#include "sensors/ModbusSoil7in1Sensor.h"
#include "sensors/ModbusTHSensor.h"
#endif

class SensorManager {
private:
  std::vector<ISensor *> sensors;
  std::vector<DallasTemperature *> dallasInstances; // Para cleanup
  uint16_t modbusDelayMs =
      50; // Delay entre lecturas de sensores (configurable)

public:
  SensorManager() {}

  void setModbusDelay(uint16_t delayMs) {
    modbusDelayMs = delayMs;
    DBG_INFO("Modbus delay set to %dms\n", modbusDelayMs);
  }

  uint16_t getModbusDelay() const { return modbusDelayMs; }

  ~SensorManager() {
    // Cleanup
    for (auto *sensor : sensors) {
      delete sensor;
    }
    for (auto *dallas : dallasInstances) {
      delete dallas;
    }
  }

  void loadFromConfig(JsonDocument &config) {
    if (!config["sensors"].is<JsonArray>()) {
      DBG_INFO("No sensors config, using default capacitive\n");
      auto* s = new SensorCapacitive();
      if (s->init()) {
        sensors.push_back(s);
      }
      return;
    }

    JsonArray sensorsArray = config["sensors"];

    for (JsonObject sensorCfg : sensorsArray) {
      bool enabled = sensorCfg["enabled"] | true;
      if (!enabled)
        continue;

      const char *type = sensorCfg["type"];
      JsonObject cfg = sensorCfg["config"];

      if (strcmp(type, "capacitive") == 0) {
        int pin = cfg["pin"] | 34;
        int dry = cfg["dry"] | 4095;
        int wet = cfg["wet"] | 0;
        SensorCapacitive *s = new SensorCapacitive(pin, dry, wet);
        if (s->init()) {
          sensors.push_back(s);
          DBG_INFO("Capacitive sensor pin %d cal=%d/%d added\n", pin, dry, wet);
        }

      } else if (strcmp(type, "scd30") == 0) {
        ISensor *s = new SensorSCD30();
        if (s->init()) {
          sensors.push_back(s);
          DBG_INFO("SCD30 sensor added\n");
        }

      } else if (strcmp(type, "internal_temp") == 0) {
        InternalTempSensor *s = new InternalTempSensor();
        if (s->init()) {
          sensors.push_back(s);
          DBG_INFO("InternalTemp sensor added\n");
        } else {
          delete s;
          DBG_ERROR("InternalTemp init failed\n");
        }

      } else if (strcmp(type, "bme280") == 0) {
        ISensor *s = new SensorBME280();
        if (s->init()) {
          sensors.push_back(s);
          DBG_INFO("BME280 sensor added\n");
        }

      } else if (strcmp(type, "simulated") == 0) {
        ISensor *s = new SensorSimulated();
        if (s->init()) {
          sensors.push_back(s);
          DBG_INFO("Simulated sensor added\n");
        }

      } else if (strcmp(type, "onewire") == 0) {
        int pin = cfg["pin"] | 4;
        bool scan = cfg["scan"] | true;
        if (scan) {
          int count = scanOneWire(pin);
          DBG_INFO("OneWire: %d sensors on pin %d\n", count, pin);
        }

      }
#ifdef ENABLE_RS485
      else if (strcmp(type, "modbus_th") == 0) {
        // Modbus RTU Temperature/Humidity sensor (TH-MB-04S)
        // Uses global RS485 bus configuration from config["rs485"]
        // Sensor config only specifies Modbus addresses

        // Check for addresses array or single address
        std::vector<uint8_t> addrList;

        if (cfg["addresses"].is<JsonArray>()) {
          // Multiple addresses: [1, 45, 3]
          for (JsonVariant addr : cfg["addresses"].as<JsonArray>()) {
            addrList.push_back(addr.as<uint8_t>());
          }
        } else {
          // Single address (backwards compatible)
          addrList.push_back(cfg["address"] | 1);
        }

        // Get RS485 bus config from global config (passed via ModbusManager
        // singleton) ModbusManager must be initialized before sensors in
        // main.cpp Sensor just needs its Modbus address, bus is already
        // configured
        for (uint8_t addr : addrList) {
          ISensor *s = new ModbusTHSensor(addr);
          if (s->init()) {
            sensors.push_back(s);
            DBG_INFO("ModbusTH addr=%d added\n", addr);
          } else {
            delete s;
            DBG_ERROR("ModbusTH addr=%d init failed\n", addr);
          }
        }
      } else if (strcmp(type, "modbus_soil_7in1") == 0) {
        // Modbus RTU 7-in-1 Soil Sensor (ZTS-3001-TR-ECTGNPKPH-N01)
        // Measures: moisture, temperature, EC, pH, N, P, K
        // Uses global RS485 bus configuration from config["rs485"]
        // Default baud rate for this sensor is 4800

        std::vector<uint8_t> addrList;

        if (cfg["addresses"].is<JsonArray>()) {
          for (JsonVariant addr : cfg["addresses"].as<JsonArray>()) {
            addrList.push_back(addr.as<uint8_t>());
          }
        } else {
          addrList.push_back(cfg["address"] | 1);
        }

        for (uint8_t addr : addrList) {
          ISensor *s = new ModbusSoil7in1Sensor(addr);
          if (s->init()) {
            sensors.push_back(s);
            DBG_INFO("ModbusSoil7in1 addr=%d added\n", addr);
          } else {
            delete s;
            DBG_ERROR("ModbusSoil7in1 addr=%d init failed\n", addr);
          }
        }
      }
#endif
      else if (strcmp(type, "hd38") == 0) {
        // HD-38 Soil Moisture sensor
        // Support both 'analog_pins' array and legacy 'analog_pin' single value
        // Per-pin calibration via dry_values[] / wet_values[] parallel arrays
        bool divider = cfg["voltage_divider"] | true;
        bool invert = cfg["invert_logic"] | false;

        std::vector<int> pinList;

        if (cfg["analog_pins"].is<JsonArray>()) {
          for (JsonVariant pin : cfg["analog_pins"].as<JsonArray>()) {
            pinList.push_back(pin.as<int>());
          }
        } else if (cfg["pin"].is<int>()) {
          // Unified interface: same 'pin' field as capacitive
          pinList.push_back(cfg["pin"].as<int>());
        } else {
          pinList.push_back(cfg["analog_pin"] | 35);
        }

        // Read optional per-pin calibration arrays
        bool hasDryArr = cfg["dry_values"].is<JsonArray>();
        bool hasWetArr = cfg["wet_values"].is<JsonArray>();
        // Also support single sensor flat dry/wet
        int flatDry = cfg["dry"] | 4095;
        int flatWet = cfg["wet"] | 0;

        for (size_t i = 0; i < pinList.size(); i++) {
          int aPin = pinList[i];
          int dry = hasDryArr ? (cfg["dry_values"][i] | flatDry) : flatDry;
          int wet = hasWetArr ? (cfg["wet_values"][i] | flatWet) : flatWet;

          char sensorName[16];
          snprintf(sensorName, sizeof(sensorName), "%d", aPin);

          HD38Sensor *s = new HD38Sensor(aPin, -1, divider, invert, sensorName);
          s->setCalibration(dry, wet);
          if (s->init()) {
            sensors.push_back(s);
            DBG_INFO("HD38 '%s' pin %d dry=%d wet=%d added\n", sensorName, aPin, dry, wet);
          } else {
            delete s;
            DBG_ERROR("HD38 pin %d init failed\n", aPin);
          }
        }
      }
    }

    DBG_INFO("Total sensors: %d\n", sensors.size());
  }

  int scanOneWire(int pin) {
    OneWire *oneWire = new OneWire(pin);
    DallasTemperature *dallas = new DallasTemperature(oneWire);
    dallas->begin();

    dallasInstances.push_back(dallas);

    int deviceCount = dallas->getDeviceCount();

    for (int i = 0; i < deviceCount; i++) {
      DeviceAddress addr;
      if (dallas->getAddress(addr, i)) {
        ISensor *s = new SensorOneWire(dallas, addr, i);
        if (s->init()) {
          sensors.push_back(s);
        }
      }
    }

    dallas->requestTemperatures();
    return deviceCount;
  }

  /**
   * Read all active sensors.
   * Legacy method — called from sendDataGrafana loop.
   */
  void readAll() {
    for (auto *dallas : dallasInstances) {
      dallas->requestTemperatures();
    }
    if (!dallasInstances.empty()) delay(100);

    bool isFirst = true;
    for (auto *s : sensors) {
      if (s->isActive() && s->dataReady()) {
        if (!isFirst && modbusDelayMs > 0) delay(modbusDelayMs);
        isFirst = false;
        s->read();
      }
    }
  }

  /**
   * Read all sensors AND notify the ControlMediator with each primary value.
   * Call this every read_interval_ms from loop().
   */
  void readAllAndNotify(ControlMediator& mediator) {
    for (auto *dallas : dallasInstances) {
      dallas->requestTemperatures();
    }
    if (!dallasInstances.empty()) delay(100);

    bool isFirst = true;
    for (auto *s : sensors) {
      if (s->isActive() && s->dataReady()) {
        if (!isFirst && modbusDelayMs > 0) delay(modbusDelayMs);
        isFirst = false;
          if (s->read()) {
            s->notifyMediator(mediator);
          }
      }
    }
  }

  std::vector<ISensor *> &getSensors() { return sensors; }

  int getSensorCount() const { return sensors.size(); }

  // Get sensor identifier for logging
  String getSensorId(ISensor *sensor) const {
    return String(sensor->getSensorID());
  }
};

#endif // SENSOR_MANAGER_H
