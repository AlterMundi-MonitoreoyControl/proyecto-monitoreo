#pragma once
#include "ActuatorBase.h"
#include <Arduino.h>
#include <esp_timer.h>

/**
 * GpioActuator — controls a single GPIO pin using an ESP32 hardware timer
 * for auto-off (no tick() / polling needed).
 *
 * The esp_timer fires in the timer task context and directly drives the pin LOW/HIGH.
 * GPIO writes from ISR/timer context are safe on ESP32.
 *
 * activeHigh = true  → HIGH = relay ON  (most relay boards)
 * activeHigh = false → LOW  = relay ON  (active-low boards)
 *
 * Alias: RelayActuator = GpioActuator
 */
class GpioActuator : public ActuatorBase {
public:
    GpioActuator(uint8_t id, uint8_t pin, String name = "gpio",
                 bool activeHigh = true)
        : _id(id), _pin(pin), _name(name), _activeHigh(activeHigh),
          _state(false), _timer(nullptr) {}

    ~GpioActuator() {
        if (_timer) {
            esp_timer_stop(_timer);
            esp_timer_delete(_timer);
            _timer = nullptr;
        }
    }

    uint8_t     getId()   const override { return _id; }
    const char* getName() const override { return _name.c_str(); }

    bool begin() override {
        const esp_timer_create_args_t args = {
            .callback        = &GpioActuator::_timerCb,
            .arg             = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name            = "gpio_autooff",
            .skip_unhandled_events = true
        };
        esp_err_t err = esp_timer_create(&args, &_timer);
        if (err != ESP_OK) {
            // timer creation failed — auto-off disabled but GPIO still works
            _timer = nullptr;
        }
        pinMode(_pin, OUTPUT);
        _write(false);
        return true;
    }

    void _turnOn(uint32_t effDuration) override {
        if (_timer) esp_timer_stop(_timer);
        _write(true);
        if (effDuration > 0 && _timer) {
            esp_timer_start_once(_timer, (uint64_t)effDuration * 1000ULL);
        }
    }

    void _turnOff() override {
        if (_timer) esp_timer_stop(_timer);
        _write(false);
    }

    bool getState() const override { return _state; }

    /** tick() is intentionally empty — auto-off is handled by esp_timer. */
    void tick() override {}

private:
    uint8_t            _id;
    uint8_t            _pin;
    String             _name;
    bool               _activeHigh;
    volatile bool      _state;
    esp_timer_handle_t _timer;

    void _write(bool on) {
        if (!on && _state) { // was ON, now OFF
             _recordTurnOff();
        }
        _state = on;
        digitalWrite(_pin, (_activeHigh ? on : !on) ? HIGH : LOW);
    }

    /** Called by esp_timer in timer task context when durationMs elapses. */
    static void _timerCb(void* arg) {
        auto* self = static_cast<GpioActuator*>(arg);
        self->_write(false);
    }
};

using RelayActuator = GpioActuator;
