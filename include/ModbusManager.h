#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include <HardwareSerial.h>
#include "debug.h"

class ModbusManager {
public:
    static ModbusManager& getInstance() {
        static ModbusManager instance;
        return instance;
    }

    bool begin(int rx, int tx, int de, uint32_t baud) {
        if (_initialized) {
            // If already initialized, check if config matches (optional warning)
            if (rx != _rx || tx != _tx || de != _de || baud != _baud) {
                DBG_INFO("[ModbusMgr] Config mismatch, using existing\n");
            }
            return true;
        }

        DBG_INFO("[ModbusMgr] RX=%d TX=%d DE=%d baud=%d\n",
                      rx, tx, de, baud);

        _serial = &Serial2;
        _serial->begin(baud, SERIAL_8N1, rx, tx);

        _mb = new ModbusRTU();
        _mb->begin(_serial, de);
        _mb->master();

        _rx = rx; _tx = tx; _de = de; _baud = baud;
        _initialized = true;
        return true;
    }

    ModbusRTU* getModbus() { return _mb; }
    HardwareSerial* getSerial() { return _serial; }
    int getDEPin() const { return _de; }
    bool isInitialized() const { return _initialized; }

private:
    ModbusManager() : _mb(nullptr), _serial(nullptr), _initialized(false), _rx(0), _tx(0), _de(0), _baud(0) {}
    ModbusRTU* _mb;
    HardwareSerial* _serial;
    bool _initialized;
    int _rx, _tx, _de;
    uint32_t _baud;
};

#endif // MODBUS_MANAGER_H