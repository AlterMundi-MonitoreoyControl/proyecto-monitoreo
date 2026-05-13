/**
 * test/testAutomationFlow.cpp
 *
 * Prueba de integración que simula un flujo de automatización completo:
 * 1. Define un actuador (calentador).
 * 2. Define reglas de automatización (encender con frío, apagar con calor).
 * 3. Simula lecturas de sensor de temperatura.
 * 4. Verifica que el actuador responde correctamente según las reglas.
 *
 * Este test utiliza stubs y no depende de hardware ni de la carga de archivos JSON.
 */

#include <unity.h>
#include "core/ControlMediator.h"
#include "core/Rule.h"

// -- Stubs y Helpers --

/** Actuador simulado que registra el último comando recibido. */
struct StubActuator : public IActuator {
    uint8_t  id;
    bool     state;
    uint32_t lastDuration;
    uint8_t  commandCount;

    StubActuator(uint8_t i) : id(i), state(false), lastDuration(0), commandCount(0) {}
    uint8_t     getId()   const override { return id; }
    const char* getName() const override { return "stub_actuator"; }
    bool        begin()         override { return true; }
    bool        getState() const override { return state; }
    void execute(const ActuatorCommand& cmd) override {
        state        = cmd.state;
        lastDuration = cmd.durationMs;
        commandCount++;
    }
};

/** Helper para crear una expresión de regla simple (LEAF). */
static uint8_t addLeaf(ControlMediator& m, uint8_t devId, uint16_t sensorId, CondOp op, float threshold) {
    RuleExpr e;
    e.type = ExprType::LEAF;
    e.cond.key.deviceId = devId;
    e.cond.key.varId = 0;
    e.cond.key.sensorId = sensorId;
    e.cond.op   = op;
    e.cond.threshold = threshold;
    return m.addExpr(e);
}

/** Helper para inyectar una lectura de sensor en el mediador. */
static void feed(ControlMediator& m, uint8_t dev, uint16_t sid, float value, uint32_t counter = 1) {
    SensorReading r;
    r.key.deviceId = dev;
    r.key.varId = 0;
    r.key.sensorId = sid;
    r.value   = value;
    r.counter = counter;
    m.onSensorReading(r);
}

// -- Tests --

void test_heater_turn_on_when_cold() {
    ControlMediator mediator;
    StubActuator heater(16);
    mediator.registerActuator(&heater);

    uint8_t ruleExprIdx = addLeaf(mediator, 0, 1, CondOp::LT, 20.0f); // Sensor 1 = Temp
    Rule rule = { ruleExprIdx, 16, true, 2, 0 }; // IF temp < 20 THEN heater ON
    mediator.addRule(rule);

    feed(mediator, 0, 1, 22.0f, 1); // Temp normal, no debe encender
    TEST_ASSERT_FALSE_MESSAGE(heater.getState(), "Calentador deberia estar OFF con 22C");

    feed(mediator, 0, 1, 18.0f, 2); // Temp baja, debe encender
    TEST_ASSERT_TRUE_MESSAGE(heater.getState(), "Calentador deberia encender con 18C");
}

void test_heater_turn_off_when_warm() {
    ControlMediator mediator;
    StubActuator heater(16);
    heater.state = true; // Asumimos que el calentador ya está encendido
    mediator.registerActuator(&heater);

    uint8_t ruleExprIdx = addLeaf(mediator, 0, 1, CondOp::GT, 25.0f);
    Rule rule = { ruleExprIdx, 16, false, 2, 0 }; // IF temp > 25 THEN heater OFF
    mediator.addRule(rule);

    feed(mediator, 0, 1, 26.0f, 2); // Temp sube, debe apagar
    TEST_ASSERT_FALSE_MESSAGE(heater.getState(), "Calentador deberia apagar con 26C");
}

void runAutomationFlowTests() {
    RUN_TEST(test_heater_turn_on_when_cold);
    RUN_TEST(test_heater_turn_off_when_warm);
}