/**
 * test/testActuatorTiming.cpp
 *
 * Verifica la lógica de protección de hardware implementada en ActuatorBase:
 * 1. Max ON Time: Trunca comandos con duración infinita o mayor a la permitida.
 * 2. Min OFF Time: Ignora y bloquea encendidos durante el período de enfriamiento.
 */

#include <unity.h>
#include "core/ActuatorBase.h"

#ifndef ARDUINO
#include <ArduinoFake.h>
using namespace fakeit;
#else
#include <Arduino.h>
#endif

/** Actuador falso para interceptar la lógica de ActuatorBase */
struct TimingActuator : public ActuatorBase {
    uint8_t id;
    bool state;
    uint32_t lastEffDuration;
    int turnOnCalls;
    uint32_t startTime;
    uint32_t durationMs;

    TimingActuator(uint8_t i) : id(i), state(false), lastEffDuration(0), turnOnCalls(0), startTime(0), durationMs(0) {}

    uint8_t getId() const override { return id; }
    const char* getName() const override { return "timing_act"; }
    bool begin() override { return true; }
    bool getState() const override { return state; }
    
    void tick() override {
        if (durationMs > 0 && (millis() - startTime >= durationMs)) {
            durationMs = 0;
            _turnOff();
        }
    }

    void _turnOn(uint32_t effDuration) override {
        state = true;
        lastEffDuration = effDuration;
        durationMs = effDuration;
        if (effDuration > 0) startTime = millis();
        turnOnCalls++;
    }

    void _turnOff() override {
        if (state) {
            _recordTurnOff(); // Fundamental para registrar el timestamp de apagado
        }
        state = false;
    }
};

void test_max_on_time_truncates_infinite_duration() {
    TimingActuator act(1);
    act.configure(5000, 0); // Max ON = 5s
    
    ActuatorCommand cmd{1, true, 0, 3}; // Comando sin límite (infinito)
    act.execute(cmd);
    
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT32(5000, act.lastEffDuration);
}

void test_max_on_time_truncates_large_duration() {
    TimingActuator act(1);
    act.configure(5000, 0); // Max ON = 5s
    
#ifndef ARDUINO
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000);
#endif

    ActuatorCommand cmd{1, true, 10000, 3}; // Comando pidiendo 10s
    act.execute(cmd);
    
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT32(5000, act.lastEffDuration);

    // Simular el paso del tiempo (avanzar 5 segundos exactos)
#ifndef ARDUINO
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000 + 5000);
#else
    delay(5005); // Se añade un extra para tolerar el jitter de hardware
#endif

    // El tick del sistema detectará el tiempo vencido y forzará el apagado
    act.tick();
    TEST_ASSERT_FALSE_MESSAGE(act.state, "El actuador debio apagarse despues de los 5s (Max ON)");
}

void test_min_off_time_blocks_restarts() {
    TimingActuator act(1);
    act.configure(0, 500); // Min OFF = 500ms
    
#ifndef ARDUINO
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000);
#endif

    act.execute({1, true, 0, 3}); // Encendemos inicialmente
    TEST_ASSERT_TRUE(act.state);
    
    act.execute({1, false, 0, 3}); // Apagamos (inicia el contador de _lastOffTime)
    TEST_ASSERT_FALSE(act.state);
    
    act.turnOnCalls = 0;
    act.execute({1, true, 0, 3}); // Intentamos encender inmediatamente
    TEST_ASSERT_FALSE_MESSAGE(act.state, "El actuador debio ser bloqueado por Min OFF time");
    TEST_ASSERT_EQUAL_INT(0, act.turnOnCalls);

    // Simular el paso del periodo de enfriamiento
#ifndef ARDUINO
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000 + 505);
#else
    delay(505);
#endif

    // Intentamos nuevamente; esta vez el periodo de enfriamiento ha finalizado
    act.execute({1, true, 0, 3}); 
    TEST_ASSERT_TRUE_MESSAGE(act.state, "El actuador debio encender luego de transcurrido el tiempo Min OFF");
    TEST_ASSERT_EQUAL_INT(1, act.turnOnCalls);
}

// ── Entry point ───────────────────────────────────────────────────────────────

void runActuatorTimingTests() {
    RUN_TEST(test_max_on_time_truncates_infinite_duration);
    RUN_TEST(test_max_on_time_truncates_large_duration);
    RUN_TEST(test_min_off_time_blocks_restarts);
}