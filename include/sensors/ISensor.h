#ifndef ISENSOR_H
#define ISENSOR_H

#include "core/SensorKey.h"
#include "core/ControlMediator.h"

/**
 * Base interface for all sensors.
 *
 * SensorKey identity is now STABLE — derived from physical address
 * (pin, I2C addr, Modbus addr, OneWire last byte) at construction time.
 * setSensorId() has been removed; each concrete sensor class inherits
 * SensorBase(stableSensorId) and sets the ID in its own constructor.
 *
 * Measurement sub-interfaces via dynamic_cast:
 *   ITemperatureSensor, IHumiditySensor, IMoistureSensor,
 *   ICO2Sensor, IPressureSensor, ISoilSensor
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    // ── Lifecycle ─────────────────────────────────────────────────────────
    virtual bool init()      = 0;
    virtual bool dataReady() = 0;
    virtual bool read()      = 0;
    virtual bool isActive()  = 0;

    // ── Identification ────────────────────────────────────────────────────
    virtual const char* getSensorType()         = 0;
    virtual const char* getSensorID()           = 0;
    virtual const char* getMeasurementsString() = 0;

    // ── Mediator identity ─────────────────────────────────────────────────
    /**
     * Stable SensorKey derived from physical address + board MAC.
     * Never changes after construction.
     */
    virtual SensorKey getKey() const = 0;

    /** Human-readable name (defaults to getSensorType()). */
    virtual const char* getName() const {
        return const_cast<ISensor*>(this)->getSensorType();
    }

    // ── Mediator reading ──────────────────────────────────────────────────
    /**
     * Notify the mediator with all valid readings from this sensor.
     * Call only after a successful read().
     */
    virtual void notifyMediator(ControlMediator& mediator) = 0;

    // ── Optional calibration ──────────────────────────────────────────────
    virtual bool calibrate(float reference = 0) { return false; }
};

#endif // ISENSOR_H
