/**
 * test/testRuleConditions.cpp
 *
 * Unit tests for the ControlMediator rule engine:
 *   - 1 condition  (LEAF)
 *   - 2 conditions (AND / OR)
 *   - 3 conditions (flat array, left-deep tree)
 *   - 4 conditions (flat array, left-deep tree)
 *
 * Runs on native platform — no hardware, no Arduino SDK.
 * Mocks: millis(), ESP.getEfuseMac() (needed by SensorBase)
 */

#include <unity.h>
#include <cstdint>
#include <cstring>
#include <cmath>

// ── Minimal Arduino / ESP32 stubs ────────────────────────────────────────────

#ifndef ARDUINO
#include "MockESP.h"
#else
#include <Arduino.h>
#endif

// ── Headers under test ───────────────────────────────────────────────────────
#include "core/SensorKey.h"
#include "core/MediatorConfig.h"
#include "core/Rule.h"
#include "core/ControlMediator.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

// Minimal stub actuator that records the last command received
struct StubActuator : public IActuator {
    uint8_t  id;
    bool     state;
    uint32_t lastDuration;
    uint8_t  commandCount;

    StubActuator(uint8_t i) : id(i), state(false), lastDuration(0), commandCount(0) {}

    uint8_t     getId()   const override { return id; }
    const char* getName() const override { return "stub"; }
    bool        begin()         override { return true; }
    bool        getState() const override { return state; }

    void execute(const ActuatorCommand& cmd) override {
        state        = cmd.state;
        lastDuration = cmd.durationMs;
        commandCount++;
    }
};

/** Build a LEAF expression and add it to the mediator pool. Returns exprIdx. */
static uint8_t addLeaf(ControlMediator& m,
                        uint8_t devId, uint16_t sensorId,
                        CondOp op, float threshold) {
    RuleExpr e;
    e.type = ExprType::LEAF;
    e.cond.key.deviceId = devId;
    e.cond.key.varId = 0;
    e.cond.key.sensorId = sensorId;
    e.cond.op   = op;
    e.cond.threshold = threshold;
    e.leftIdx = e.rightIdx = 0;
    return m.addExpr(e);
}

/** Feed a reading directly into the mediator. */
static void feed(ControlMediator& m, uint8_t dev, uint16_t sid,
                 float value, uint32_t counter = 1) {
    SensorReading r;
    r.key.deviceId = dev;
    r.key.varId = 0;
    r.key.sensorId = sid;
    r.value   = value;
    r.counter = counter;
    m.onSensorReading(r);
}

// ────────────────────────────────────────────────────────────────────────────
// TEST 1 — Single LEAF condition
// Rule: sensor[0:1] > 30  → actuator ON
// ────────────────────────────────────────────────────────────────────────────

void test_single_condition_fires_when_true() {
    ControlMediator m;
    StubActuator    act(1);
    m.registerActuator(&act);

    uint8_t leafIdx = addLeaf(m, 0, 1, CondOp::GT, 30.0f);

    Rule r;
    r.rootExprIdx  = leafIdx;
    r.actuatorId   = 1;
    r.triggerState = true;
    r.priority     = 2;
    r.durationMs   = 0;
    m.addRule(r);

    // Below threshold — should NOT fire
    feed(m, 0, 1, 25.0f, 1);
    TEST_ASSERT_FALSE(act.state);
    TEST_ASSERT_EQUAL_UINT8(0, act.commandCount);

    // Above threshold — SHOULD fire
    feed(m, 0, 1, 35.0f, 2);
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT8(1, act.commandCount);
}

void test_single_condition_does_not_fire_when_false() {
    ControlMediator m;
    StubActuator    act(2);
    m.registerActuator(&act);

    uint8_t leafIdx = addLeaf(m, 0, 2, CondOp::LT, 20.0f);

    Rule r;
    r.rootExprIdx  = leafIdx;
    r.actuatorId   = 2;
    r.triggerState = true;
    r.priority     = 2;
    r.durationMs   = 0;
    m.addRule(r);

    feed(m, 0, 2, 25.0f, 1);
    TEST_ASSERT_FALSE(act.state);
    TEST_ASSERT_EQUAL_UINT8(0, act.commandCount);
}

void test_single_condition_stale_counter_ignored() {
    ControlMediator m;
    StubActuator    act(3);
    m.registerActuator(&act);

    uint8_t leafIdx = addLeaf(m, 0, 3, CondOp::GT, 30.0f);
    Rule r; r.rootExprIdx = leafIdx; r.actuatorId = 3;
    r.triggerState = true; r.priority = 2; r.durationMs = 0;
    m.addRule(r);

    // First reading — valid
    feed(m, 0, 3, 35.0f, 5);
    TEST_ASSERT_TRUE(act.state);

    // Send an OFF update with a LOWER counter — must be ignored
    act.state = false;  // manually reset to detect if it fires
    feed(m, 0, 3, 10.0f, 3);  // counter 3 < 5 → stale
    // state store should not have updated, so rule still sees 35 > 30
    // but the key question is that the second call did NOT trigger a new eval
    // (commandCount should still be 1)
    TEST_ASSERT_EQUAL_UINT8(1, act.commandCount);
}

// ────────────────────────────────────────────────────────────────────────────
// TEST 2 — Two conditions (AND / OR)
// ────────────────────────────────────────────────────────────────────────────

void test_two_conditions_AND_both_true() {
    ControlMediator m;
    StubActuator    act(10);
    m.registerActuator(&act);

    uint8_t l = addLeaf(m, 0, 10, CondOp::GT, 30.0f);  // temp > 30
    uint8_t r = addLeaf(m, 0, 11, CondOp::LT, 50.0f);  // hum < 50

    RuleExpr andExpr;
    andExpr.type = ExprType::AND;
    andExpr.leftIdx = l; andExpr.rightIdx = r;
    andExpr.cond = {{0,0}, CondOp::GT, 0};
    uint8_t andIdx = m.addExpr(andExpr);

    Rule rule;
    rule.rootExprIdx = andIdx; rule.actuatorId = 10;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 10, 35.0f, 1);  // temp = 35 > 30 ✓
    feed(m, 0, 11, 40.0f, 1);  // hum  = 40 < 50 ✓ → both true → FIRE
    TEST_ASSERT_TRUE(act.state);
}

void test_two_conditions_AND_one_false() {
    ControlMediator m;
    StubActuator    act(11);
    m.registerActuator(&act);

    uint8_t l = addLeaf(m, 0, 20, CondOp::GT, 30.0f);
    uint8_t r = addLeaf(m, 0, 21, CondOp::LT, 50.0f);

    RuleExpr andExpr;
    andExpr.type = ExprType::AND;
    andExpr.leftIdx = l; andExpr.rightIdx = r;
    andExpr.cond = {{0,0}, CondOp::GT, 0};
    uint8_t andIdx = m.addExpr(andExpr);

    Rule rule;
    rule.rootExprIdx = andIdx; rule.actuatorId = 11;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 20, 35.0f, 1);  // temp = 35 > 30 ✓
    feed(m, 0, 21, 60.0f, 1);  // hum  = 60 < 50 ✗ → AND fails
    TEST_ASSERT_FALSE(act.state);
}

void test_two_conditions_OR_one_true() {
    ControlMediator m;
    StubActuator    act(12);
    m.registerActuator(&act);

    uint8_t l = addLeaf(m, 0, 30, CondOp::GT, 40.0f);  // false (30 < 40 not supplied yet)
    uint8_t r = addLeaf(m, 0, 31, CondOp::LT, 20.0f);  // true (10 < 20)

    RuleExpr orExpr;
    orExpr.type = ExprType::OR;
    orExpr.leftIdx = l; orExpr.rightIdx = r;
    orExpr.cond = {{0,0}, CondOp::GT, 0};
    uint8_t orIdx = m.addExpr(orExpr);

    Rule rule;
    rule.rootExprIdx = orIdx; rule.actuatorId = 12;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 30, 25.0f, 1);  // 25 > 40 → false
    feed(m, 0, 31, 10.0f, 1);  // 10 < 20 → true → OR fires
    TEST_ASSERT_TRUE(act.state);
}

// ────────────────────────────────────────────────────────────────────────────
// TEST 3 — Three conditions (AND of 3 via left-deep tree)
// c0 AND c1 AND c2 → all must be true
// ────────────────────────────────────────────────────────────────────────────

static uint8_t buildLeftDeepAnd(ControlMediator& m, uint8_t* leafIndices, int count) {
    uint8_t result = leafIndices[0];
    for (int i = 1; i < count; i++) {
        RuleExpr e;
        e.type = ExprType::AND;
        e.leftIdx  = result;
        e.rightIdx = leafIndices[i];
        e.cond = {{0,0}, CondOp::GT, 0};
        result = m.addExpr(e);
    }
    return result;
}

void test_three_conditions_all_true() {
    ControlMediator m;
    StubActuator    act(20);
    m.registerActuator(&act);

    uint8_t leaves[3];
    leaves[0] = addLeaf(m, 0, 40, CondOp::GT, 20.0f);  // v > 20
    leaves[1] = addLeaf(m, 0, 41, CondOp::LT, 80.0f);  // v < 80
    leaves[2] = addLeaf(m, 0, 42, CondOp::GTE,10.0f);  // v >= 10

    uint8_t rootIdx = buildLeftDeepAnd(m, leaves, 3);

    Rule rule;
    rule.rootExprIdx = rootIdx; rule.actuatorId = 20;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 40, 30.0f, 1);  // 30 > 20  ✓
    feed(m, 0, 41, 50.0f, 1);  // 50 < 80  ✓
    feed(m, 0, 42, 15.0f, 1);  // 15 >= 10 ✓  → all true → FIRE
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT8(1, act.commandCount);
}

void test_three_conditions_last_one_false() {
    ControlMediator m;
    StubActuator    act(21);
    m.registerActuator(&act);

    uint8_t leaves[3];
    leaves[0] = addLeaf(m, 0, 50, CondOp::GT, 20.0f);
    leaves[1] = addLeaf(m, 0, 51, CondOp::LT, 80.0f);
    leaves[2] = addLeaf(m, 0, 52, CondOp::GT, 100.0f); // will be false

    uint8_t rootIdx = buildLeftDeepAnd(m, leaves, 3);

    Rule rule;
    rule.rootExprIdx = rootIdx; rule.actuatorId = 21;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 50, 30.0f, 1);
    feed(m, 0, 51, 50.0f, 1);
    feed(m, 0, 52,  5.0f, 1);  // 5 > 100 → false → AND fails
    TEST_ASSERT_FALSE(act.state);
}

// ────────────────────────────────────────────────────────────────────────────
// TEST 4 — Four conditions (AND of 4, mirrors RuleLoader flat array output)
// ────────────────────────────────────────────────────────────────────────────

void test_four_conditions_all_true() {
    ControlMediator m;
    StubActuator    act(30);
    m.registerActuator(&act);

    uint8_t leaves[4];
    leaves[0] = addLeaf(m, 0, 60, CondOp::GT, 10.0f);
    leaves[1] = addLeaf(m, 0, 61, CondOp::GT, 10.0f);
    leaves[2] = addLeaf(m, 0, 62, CondOp::GT, 10.0f);
    leaves[3] = addLeaf(m, 0, 63, CondOp::GT, 10.0f);

    uint8_t rootIdx = buildLeftDeepAnd(m, leaves, 4);

    Rule rule;
    rule.rootExprIdx = rootIdx; rule.actuatorId = 30;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 60, 15.0f, 1);
    feed(m, 0, 61, 20.0f, 1);
    feed(m, 0, 62, 25.0f, 1);
    feed(m, 0, 63, 30.0f, 1);  // all > 10 → FIRE
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT8(1, act.commandCount);
}

void test_four_conditions_third_false_stops_chain() {
    ControlMediator m;
    StubActuator    act(31);
    m.registerActuator(&act);

    uint8_t leaves[4];
    leaves[0] = addLeaf(m, 0, 70, CondOp::GT, 10.0f);
    leaves[1] = addLeaf(m, 0, 71, CondOp::GT, 10.0f);
    leaves[2] = addLeaf(m, 0, 72, CondOp::GT, 50.0f);  // will fail
    leaves[3] = addLeaf(m, 0, 73, CondOp::GT, 10.0f);

    uint8_t rootIdx = buildLeftDeepAnd(m, leaves, 4);

    Rule rule;
    rule.rootExprIdx = rootIdx; rule.actuatorId = 31;
    rule.triggerState = true; rule.priority = 2; rule.durationMs = 0;
    m.addRule(rule);

    feed(m, 0, 70, 15.0f, 1);
    feed(m, 0, 71, 20.0f, 1);
    feed(m, 0, 72,  5.0f, 1);  // 5 > 50 → false
    feed(m, 0, 73, 25.0f, 1);
    TEST_ASSERT_FALSE(act.state);
    TEST_ASSERT_EQUAL_UINT8(0, act.commandCount);
}

void test_four_conditions_with_duration_ms() {
    ControlMediator m;
    StubActuator    act(32);
    m.registerActuator(&act);

    uint8_t leaves[4];
    leaves[0] = addLeaf(m, 0, 80, CondOp::LTE, 25.0f);
    leaves[1] = addLeaf(m, 0, 81, CondOp::LTE, 25.0f);
    leaves[2] = addLeaf(m, 0, 82, CondOp::GTE, 60.0f);
    leaves[3] = addLeaf(m, 0, 83, CondOp::GT,   0.0f);

    uint8_t rootIdx = buildLeftDeepAnd(m, leaves, 4);

    Rule rule;
    rule.rootExprIdx  = rootIdx;
    rule.actuatorId   = 32;
    rule.triggerState = true;
    rule.priority     = 2;
    rule.durationMs   = 5000;   // auto-off after 5 s
    m.addRule(rule);

    feed(m, 0, 80, 20.0f, 1);
    feed(m, 0, 81, 22.0f, 1);
    feed(m, 0, 82, 70.0f, 1);
    feed(m, 0, 83,  1.0f, 1);
    TEST_ASSERT_TRUE(act.state);
    TEST_ASSERT_EQUAL_UINT32(5000, act.lastDuration);
}

// ────────────────────────────────────────────────────────────────────────────
// Test entry points
// ────────────────────────────────────────────────────────────────────────────

void runRuleConditionTests() {
    // Single condition
    RUN_TEST(test_single_condition_fires_when_true);
    RUN_TEST(test_single_condition_does_not_fire_when_false);
    RUN_TEST(test_single_condition_stale_counter_ignored);
    // Two conditions
    RUN_TEST(test_two_conditions_AND_both_true);
    RUN_TEST(test_two_conditions_AND_one_false);
    RUN_TEST(test_two_conditions_OR_one_true);
    // Three conditions
    RUN_TEST(test_three_conditions_all_true);
    RUN_TEST(test_three_conditions_last_one_false);
    // Four conditions
    RUN_TEST(test_four_conditions_all_true);
    RUN_TEST(test_four_conditions_third_false_stops_chain);
    RUN_TEST(test_four_conditions_with_duration_ms);
}
