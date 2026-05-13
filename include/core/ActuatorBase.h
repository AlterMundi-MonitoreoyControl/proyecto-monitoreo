#pragma once
#include "IActuator.h"
#include <Arduino.h>

/**
 * ActuatorBase — Provee lógica común de temporización para actuadores.
 * Abstrae el manejo de "Max ON time" y "Min OFF time".
 */
class ActuatorBase : public IActuator {
protected:
    uint32_t _maxOnMs;
    uint32_t _minOffMs;
    uint32_t _lastOffTime;

public:
    ActuatorBase() : _maxOnMs(0), _minOffMs(0), _lastOffTime(0) {}
    virtual ~ActuatorBase() = default;

    virtual void configure(uint32_t maxOn, uint32_t minOff) {
        _maxOnMs = maxOn;
        _minOffMs = minOff;
    }

    void execute(const ActuatorCommand& cmd) override {
        if (cmd.state) {
            if (_minOffMs > 0 && (millis() - _lastOffTime < _minOffMs)) {
                return; // Ignora el encendido si está en periodo de enfriamiento
            }
            uint32_t effDuration = cmd.durationMs;
            if (_maxOnMs > 0) {
                if (effDuration == 0 || effDuration > _maxOnMs) effDuration = _maxOnMs;
            }
            _turnOn(effDuration);
        } else {
            _turnOff();
        }
    }

protected:
    virtual void _turnOn(uint32_t effectiveDurationMs) = 0;
    virtual void _turnOff() = 0;

    void _recordTurnOff() {
        _lastOffTime = millis();
    }
};