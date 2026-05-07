#ifndef RS485_MANAGER_H
#define RS485_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "ModbusManager.h"
#include "debug.h"

class RS485Manager {
private:
    HardwareSerial* serial;
    int dePin;         // Driver Enable pin
    int rePin;         // Receiver Enable pin
    int rxPin;
    int txPin;
    int baudRate;
    bool useDERE;      // Whether DE/RE pins are used
    bool rawSendEnabled;  // Whether raw serial sending is enabled

public:
    RS485Manager()
        : serial(nullptr), dePin(-1), rePin(-1), rxPin(16), txPin(17), baudRate(9600), useDERE(false), rawSendEnabled(false) {}

    // Enable/disable raw serial sending
    void setRawSendEnabled(bool enabled) {
        rawSendEnabled = enabled;
        DBG_INFO("[RS485] Raw send %s\n", enabled ? "on" : "off");
    }

    bool isRawSendEnabled() const {
        return rawSendEnabled;
    }

    // Initialize RS485 with optional DE/RE control
    bool init(int rx = 16, int tx = 17, int baud = 9600, int de = -1, int re = -1) {
        // Check if ModbusManager has already initialized the bus
        if (ModbusManager::getInstance().isInitialized()) {
            serial = ModbusManager::getInstance().getSerial();
            dePin = ModbusManager::getInstance().getDEPin();
            rePin = dePin; // Assume shared DE/RE pin logic from Modbus config
            useDERE = (dePin >= 0);
            
            DBG_INFO("[RS485] Reusing ModbusMgr serial\n");
            return true;
        }

        rxPin = rx;
        txPin = tx;
        baudRate = baud;
        dePin = de;
        rePin = re;

        // Use Serial2 (UART2) for RS485
        serial = &Serial2;

        // Initialize UART
        serial->begin(baudRate, SERIAL_8N1, rxPin, txPin);

        // Configure DE/RE pins if provided
        if (dePin >= 0 && rePin >= 0) {
            pinMode(dePin, OUTPUT);
            pinMode(rePin, OUTPUT);
            setReceiveMode();  // Default to receive mode
            useDERE = true;
            DBG_INFO("[RS485] DE/RE pins %d,%d\n", dePin, rePin);
        } else {
            useDERE = false;
            DBG_INFO("[RS485] No DE/RE (bridged)\n");
        }

        DBG_INFO("[RS485] RX=%d TX=%d baud=%d\n", rxPin, txPin, baudRate);
        return true;
    }

    // Switch to transmit mode
    void setTransmitMode() {
        if (useDERE) {
            digitalWrite(dePin, HIGH);  // Enable driver
            digitalWrite(rePin, HIGH);  // Disable receiver
        }
    }

    // Switch to receive mode
    void setReceiveMode() {
        if (useDERE) {
            digitalWrite(dePin, LOW);   // Disable driver
            digitalWrite(rePin, LOW);   // Enable receiver
        }
    }

    // Send string via RS485
    void send(const String& data) {
        if (!serial) return;

        setTransmitMode();
        delayMicroseconds(100);  // Small delay for transceiver switching

        serial->print(data);
        serial->flush();  // Wait for transmission to complete

        delayMicroseconds(100);
        setReceiveMode();
    }

    // Send formatted data (only if raw send is enabled)
    void sendSensorData(float temperature, float humidity, float co2, const char* sensorType) {
        if (!rawSendEnabled) {
            return;  // Raw sending disabled, skip
        }

        String message = String(sensorType) + " - ";

        if (temperature >= 0) {
            message += "Temp: " + String(temperature, 1) + "°C ";
        }

        message += "Humedad: " + String(humidity, 1) + "% ";

        if (co2 >= 0) {
            message += "CO2: " + String(co2, 0) + "ppm";
        }

        send(message + "\r\n");
        DBG_VERBOSE("[RS485 TX] %s\n", message.c_str());
    }

    // Receive data (for bidirectional communication)
    String receive(unsigned long timeout = 1000) {
        if (!serial) return "";

        setReceiveMode();
        String received = "";
        unsigned long startTime = millis();

        while (millis() - startTime < timeout) {
            if (serial->available()) {
                char c = serial->read();
                received += c;
                if (c == '\n') break;  // End of message
            }
        }

        return received;
    }

    // Check if data is available
    bool available() {
        return serial && serial->available();
    }
};

#endif // RS485_MANAGER_H
