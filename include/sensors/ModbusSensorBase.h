#ifndef MODBUS_SENSOR_BASE_H
#define MODBUS_SENSOR_BASE_H

#include "ISensor.h"
#include <ModbusRTU.h>
#include <HardwareSerial.h>
#include "../ModbusManager.h"
#include "../debug.h"

/**
 * Base template class for Modbus RTU sensors over RS485
 *
 * Template parameter NUM_REGS specifies the number of registers to read.
 * Each template instantiation gets its own static members (registerBuffer, etc.)
 *
 * Provides common functionality:
 *   - Modbus address management
 *   - Connection state tracking (active, failure counts)
 *   - Async read with callback pattern
 *   - Automatic recovery attempts after failures
 *
 * Subclasses must implement:
 *   - parseRegisters(): convert registerBuffer to sensor values
 *   - setInvalidValues(): set values when read fails
 *   - getLogPrefix(): return log prefix string (e.g., "ModbusTH")
 *   - getSensorType(), getSensorID(), getMeasurementsString()
 *   - Capability interfaces (ITemperatureSensor, IHumiditySensor, etc.)
 */
template<uint8_t NUM_REGS>
class ModbusSensorBase : public virtual ISensor {
protected:
    uint8_t modbusAddress;
    bool active;
    int readFailureCount;
    int inactiveCheckCount;

    // Static members per template instantiation
    static uint16_t registerBuffer[NUM_REGS];
    static bool readComplete;
    static bool readError;

    static bool readCallback(Modbus::ResultCode event, uint16_t transactionId, void* data) {
        readComplete = true;
        if (event == Modbus::EX_SUCCESS) {
            readError = false;
            return true;
        } else {
            readError = true;
            return false;
        }
    }

    // Pure virtual methods - subclasses must implement
    virtual void parseRegisters() = 0;
    virtual void setInvalidValues() = 0;
    virtual const char* getLogPrefix() const = 0;

    bool readRegisters() {
        ModbusRTU* mb = ModbusManager::getInstance().getModbus();
        if (!mb) return false;

        readComplete = false;
        readError = false;

        mb->task();

        if (!mb->readHreg(modbusAddress, 0, registerBuffer, NUM_REGS, readCallback)) {
            DBG_ERROR("[%s] Addr %d: Read init failed\n", getLogPrefix(), modbusAddress);
            mb->task();
            return false;
        }

        unsigned long startTime = millis();
        while (!readComplete && (millis() - startTime < 2000)) {
            mb->task();
            delay(10);
        }

        if (!readComplete) {
            DBG_ERROR("[%s] Addr %d: Timeout\n", getLogPrefix(), modbusAddress);
            return false;
        }

        if (readError) {
            DBG_ERROR("[%s] Addr %d: Read error\n", getLogPrefix(), modbusAddress);
            return false;
        }
        return true;
    }

public:
    ModbusSensorBase(uint8_t address = 1)
        : modbusAddress(address),
          active(false),
          readFailureCount(0),
          inactiveCheckCount(0) {
    }

    virtual ~ModbusSensorBase() = default;

    bool init() override {
        DBG_VERBOSE("[%s] Init addr=%d\n", getLogPrefix(), modbusAddress);

        if (!ModbusManager::getInstance().isInitialized()) {
            DBG_ERROR("[%s] ModbusMgr not init\n", getLogPrefix());
            return false;
        }

        delay(100);
        bool testRead = readRegisters();

        if (testRead) {
            active = true;
            readFailureCount = 0;
            DBG_INFO("[%s] Addr=%d OK\n", getLogPrefix(), modbusAddress);
        } else {
            active = false;
            DBG_ERROR("[%s] Addr=%d no response\n", getLogPrefix(), modbusAddress);
        }

        return active;
    }

    bool dataReady() override {
        return active;
    }

    bool read() override {
        if (!active) return false;

        bool success = readRegisters();

        if (success) {
            readFailureCount = 0;
            parseRegisters();
            return true;
        }

        readFailureCount++;
        if (readFailureCount >= 5) {
            active = false;
            DBG_ERROR("[%s] Addr %d: Disabled (5 fails)\n", getLogPrefix(), modbusAddress);
        }

        setInvalidValues();
        DBG_ERROR("[%s] Addr %d: Read failed\n", getLogPrefix(), modbusAddress);
        return false;
    }

    bool isActive() override {
        if (active) {
            inactiveCheckCount = 0;
            return true;
        }
        inactiveCheckCount++;
        if (inactiveCheckCount >= 10) {
            inactiveCheckCount = 0;
            return init();
        }
        return false;
    }

    uint8_t getAddress() const {
        return modbusAddress;
    }
};

// Static member initialization
template<uint8_t NUM_REGS>
uint16_t ModbusSensorBase<NUM_REGS>::registerBuffer[NUM_REGS] = {};

template<uint8_t NUM_REGS>
bool ModbusSensorBase<NUM_REGS>::readComplete = false;

template<uint8_t NUM_REGS>
bool ModbusSensorBase<NUM_REGS>::readError = false;

#endif // MODBUS_SENSOR_BASE_H
