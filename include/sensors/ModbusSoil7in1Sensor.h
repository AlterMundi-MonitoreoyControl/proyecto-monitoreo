#ifndef MODBUS_SOIL_7IN1_SENSOR_H
#define MODBUS_SOIL_7IN1_SENSOR_H

#include "ModbusSensorBase.h"
#include "ITemperatureSensor.h"
#include "IMoistureSensor.h"
#include "ISoilSensor.h"
#include "SensorBase.h"
#include "../debug.h"

/**
 * Sensor ZTS-3001-TR-ECTGNPKPH-N01 - 7-in-1 Soil Sensor via Modbus RTU
 *
 * Protocol: Modbus RTU over RS485
 * Default baud rate: 4800
 * Registers:
 *   - 0 (0x00): Soil moisture (int16 * 10, e.g., 658 = 65.8%)
 *   - 1 (0x01): Temperature (int16 * 10, signed, e.g., -101 = -10.1°C)
 *   - 2 (0x02): EC conductivity (int16, direct in uS/cm)
 *   - 3 (0x03): pH (int16 * 10, e.g., 56 = 5.6)
 *   - 4 (0x04): Nitrogen N (int16, direct in mg/kg)
 *   - 5 (0x05): Phosphorus P (int16, direct in mg/kg)
 *   - 6 (0x06): Potassium K (int16, direct in mg/kg)
 */
class ModbusSoil7in1Sensor : public SensorBase,
                              public ModbusSensorBase<7>,
                              public ITemperatureSensor,
                              public IMoistureSensor,
                              public ISoilSensor {
private:
    float moisture = 0;
    float temperature = 999;
    float ec = 0;
    float ph = 0;
    uint16_t nitrogen = 0;
    uint16_t phosphorus = 0;
    uint16_t potassium = 0;

protected:
    const char* getLogPrefix() const override {
        return "Soil7in1";
    }

    void parseRegisters() override {
        moisture = registerBuffer[0] / 10.0;
        int16_t rawTemp = (int16_t)registerBuffer[1];
        temperature = rawTemp / 10.0;
        ec = registerBuffer[2];
        ph = registerBuffer[3] / 10.0;
        nitrogen = registerBuffer[4];
        phosphorus = registerBuffer[5];
        potassium = registerBuffer[6];

        DBG_VERBOSE("[Soil7in1] %d: T=%.1f M=%.1f EC=%.0f pH=%.1f N=%d P=%d K=%d\n",
                     modbusAddress, temperature, moisture, ec, ph, nitrogen, phosphorus, potassium);
    }

    void setInvalidValues() override {
        moisture = 0;
        temperature = 999;
        ec = 0;
        ph = 0;
        nitrogen = 0;
        phosphorus = 0;
        potassium = 0;
    }

public:
    ModbusSoil7in1Sensor(uint8_t address = 1)
        : SensorBase(SensorClass::RS485_MODBUS, address),          // Modbus address = stable sensorId
          ModbusSensorBase<7>(address),
          moisture(0), temperature(999), ec(0), ph(0),
          nitrogen(0), phosphorus(0), potassium(0) {}

    // ITemperatureSensor
    float getTemperature() override { return temperature; }

    // IMoistureSensor
    float getMoisture() override { return moisture; }

    // ISoilSensor
    float getEC() override { return ec; }
    float getPH() override { return ph; }
    int getNitrogen() override { return nitrogen; }
    int getPhosphorus() override { return phosphorus; }
    int getPotassium() override { return potassium; }

    const char* getSensorType() override {
        static char sensorName[32];
        snprintf(sensorName, sizeof(sensorName), "modbus_soil7in1_%d", modbusAddress);
        return sensorName;
    }

    const char* getSensorID() override {
        static char sensorID[32];
        snprintf(sensorID, sizeof(sensorID), "soil7-mod-%d", modbusAddress);
        return sensorID;
    }

    const char* getMeasurementsString() override {
        static char measString[128];
        snprintf(measString, sizeof(measString),
                 "temp=%.1f,moisture=%.1f,ec=%.0f,ph=%.1f,nitrogen=%d,phosphorus=%d,potassium=%d",
                 temperature, moisture, ec, ph, nitrogen, phosphorus, potassium);
        return measString;
    }

    // ── Mediator interface ────────────────────────────────────────────────
    SensorKey getKey() const override { return SensorBase::getKey(); }
    void notifyMediator(ControlMediator& mediator) override {
        if (temperature != 999) _notify(mediator, SensorVariable::TEMPERATURE, temperature);
        if (moisture != 0)      _notify(mediator, SensorVariable::MOISTURE, moisture);
        if (ec != 0)            _notify(mediator, SensorVariable::EC, ec);
        if (ph != 0)            _notify(mediator, SensorVariable::PH, ph);
        if (nitrogen != 0)      _notify(mediator, SensorVariable::NITROGEN, nitrogen);
        if (phosphorus != 0)    _notify(mediator, SensorVariable::PHOSPHORUS, phosphorus);
        if (potassium != 0)     _notify(mediator, SensorVariable::POTASSIUM, potassium);
    }
};

#endif // MODBUS_SOIL_7IN1_SENSOR_H
