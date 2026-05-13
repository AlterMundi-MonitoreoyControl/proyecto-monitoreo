#pragma once
#include "IActuator.h"
#include "MediatorConfig.h"
#include "Rule.h"
#include "SensorKey.h"
#include <math.h>
#include "../debug.h"

/**
 * ControlMediator — central control arbiter.
 *
 * State-store: flat float array indexed by (key.toU32() % STATE_STORE_SIZE).
 * Each slot also tracks the last seen counter to reject stale readings.
 *
 * onSensorReading(): only updates state if reading.counter > stored counter
 *                    — guarantees origin consistency without wall-clock time.
 *
 * Priority arbitration: higher priority always wins; equal priority = last writer.
 */
class ControlMediator {
public:
    ControlMediator() { _clear(); }

    // ── Registration ──────────────────────────────────────────────────────
    void registerActuator(IActuator* a) {
        if (!a) return;
        if (_actuatorCount >= ACTUATOR_MAX) {
            DBG_ERROR("[Mediator] Cannot register actuator. ACTUATOR_MAX (%d) reached.\n", ACTUATOR_MAX);
            return;
        }
        _actuators[_actuatorCount++] = a;
    }

    uint8_t addExpr(const RuleExpr& expr) {
        if (_exprCount >= RULE_EXPR_POOL) return 0xFF;
        _exprPool[_exprCount] = expr;
        return _exprCount++;
    }

    void addRule(const Rule& rule) {
        if (_ruleCount >= RULE_MAX) return;
        _rules[_ruleCount++] = rule;
    }

    void clearRules() { _exprCount = 0; _ruleCount = 0; }

    // ── Inputs ────────────────────────────────────────────────────────────

    /**
     * Receive a sensor reading (local or remote via ESP-NOW).
     * Rejected if reading.counter <= the stored counter for this slot
     * (prevents processing stale or out-of-order readings).
     */
    void onSensorReading(const SensorReading& r) {
        DBG_VERBOSE("[Mediator] onSensorReading: key=%08lX val=%.2f cnt=%lu\n", 
                    (unsigned long)r.key.toU32(), r.value, (unsigned long)r.counter);
        uint8_t idx = _storeIndex(r.key);
        if (_state[idx].valid && _state[idx].keyU32 == r.key.toU32() && r.counter <= _state[idx].counter) {
            DBG_VERBOSE("[Mediator] Discarded stale reading (stored cnt=%lu)\n", (unsigned long)_state[idx].counter);
            return;  // stale — discard
        }
        _state[idx].keyU32  = r.key.toU32();
        _state[idx].value   = r.value;
        _state[idx].counter = r.counter;
        _state[idx].valid   = true;
        evaluateAllRules();
    }

    /** Manual command (priority=3 by convention). */
    void onManualCommand(const ActuatorCommand& cmd) { dispatch(cmd); }

    /** Timer-sourced command (priority=1 by convention). */
    void onTimerTick(uint8_t actuatorId, bool state, uint32_t durationMs) {
        ActuatorCommand cmd{actuatorId, state, durationMs, 1};
        dispatch(cmd);
    }

    // ── Loop ──────────────────────────────────────────────────────────────
    /**
     * Call from loop().  Clears mediator priority tracking when
     * the actuator's hardware timer has already fired its auto-off
     * (GpioActuator uses esp_timer internally and does not need tick()).
     */
    void tick() {
        uint32_t now = millis();
        for (uint8_t i = 0; i < _actuatorCount; i++) {
            // delegated tick (RelayModule2CH or other non-esp_timer actuators)
            if (_actuators[i]) _actuators[i]->tick();

            // clear mediator priority slot when duration has elapsed
            if (_activeDuration[i] > 0 && (now - _activeStartTime[i] >= _activeDuration[i])) {
                _activeDuration[i]  = 0;
                _active[i].priority = 0;   // reset so next rule can fire
            }
        }
    }

    // ── Queries ───────────────────────────────────────────────────────────
    uint8_t    getActuatorCount() const { return _actuatorCount; }
    IActuator* getActuator(uint8_t idx) const {
        return idx < _actuatorCount ? _actuators[idx] : nullptr;
    }
    // active actuators
    uint8_t getActiveActuatorCount() const {
        uint8_t count = 0;
        for (uint8_t i = 0; i < _actuatorCount; i++) {
            if (_actuators[i] && _actuators[i]->getStatus()) count++;
        }
        return count;
    }   

    uint8_t getRuleCount() const { return _ruleCount; }

private:
    struct StateEntry {
        uint32_t keyU32;
        float    value;
        uint32_t counter;
        bool     valid;
    };
    StateEntry _state[STATE_STORE_SIZE];

    RuleExpr _exprPool[RULE_EXPR_POOL];
    uint8_t  _exprCount;

    Rule    _rules[RULE_MAX];
    uint8_t _ruleCount;

    IActuator*      _actuators[ACTUATOR_MAX];
    uint8_t         _actuatorCount;
    ActuatorCommand _active[ACTUATOR_MAX];
    uint32_t        _activeStartTime[ACTUATOR_MAX];
    uint32_t        _activeDuration[ACTUATOR_MAX];
    bool            _dispatched[ACTUATOR_MAX];

    void _clear() {
        for (uint16_t i = 0; i < STATE_STORE_SIZE; i++) {
            _state[i] = {0, 0.0f, 0, false};
        }
        _exprCount = _ruleCount = _actuatorCount = 0;
        for (uint8_t i = 0; i < ACTUATOR_MAX; i++) {
            _actuators[i]   = nullptr;
            _activeStartTime[i] = 0;
            _activeDuration[i]  = 0;
            _active[i]          = {0, false, 0, 0};
            _dispatched[i]      = false;
        }
    }

    uint8_t _storeIndex(const SensorKey& key) const {
        uint32_t u32 = key.toU32();
        // Better hash to mix high bits
        uint32_t hash = u32 ^ (u32 >> 16) ^ (u32 >> 24);
        uint8_t startIdx = (uint8_t)(hash % STATE_STORE_SIZE);
        uint8_t idx = startIdx;
        
        // Linear probing to resolve collisions
        do {
            if (!_state[idx].valid || _state[idx].keyU32 == u32) {
                return idx;
            }
            idx = (idx + 1) % STATE_STORE_SIZE;
        } while (idx != startIdx);
        
        return startIdx; // Table full; fallback to overwrite (should ideally increase STATE_STORE_SIZE)
    }

    bool evalExpr(uint8_t idx) const {
        if (idx >= _exprCount) return false;
        const RuleExpr& e = _exprPool[idx];
        switch (e.type) {
            case ExprType::LEAF: {
                uint8_t si = _storeIndex(e.cond.key);
                if (!_state[si].valid || _state[si].keyU32 != e.cond.key.toU32()) {
                    DBG_VERBOSE("[Mediator] evalExpr LEAF: key=%08lX state invalid\n", (unsigned long)e.cond.key.toU32());
                    return false;
                }
                float v = _state[si].value;
                bool res = false;
                switch (e.cond.op) {
                    case CondOp::GT:  res = v >  e.cond.threshold; break;
                    case CondOp::LT:  res = v <  e.cond.threshold; break;
                    case CondOp::EQ:  res = fabsf(v - e.cond.threshold) < 1e-4f; break;
                    case CondOp::GTE: res = v >= e.cond.threshold; break;
                    case CondOp::LTE: res = v <= e.cond.threshold; break;
                }
                DBG_VERBOSE("[Mediator] evalExpr LEAF: key=%08lX val=%.2f op=%d thresh=%.2f -> %s\n", 
                            (unsigned long)e.cond.key.toU32(), v, (int)e.cond.op, e.cond.threshold, res ? "TRUE" : "FALSE");
                return res;
            }
            case ExprType::AND: {
                bool res = evalExpr(e.leftIdx) && evalExpr(e.rightIdx);
                DBG_VERBOSE("[Mediator] evalExpr AND: left=%d right=%d -> %s\n", e.leftIdx, e.rightIdx, res ? "TRUE" : "FALSE");
                return res;
            }
            case ExprType::OR: {
                bool res = evalExpr(e.leftIdx) || evalExpr(e.rightIdx);
                DBG_VERBOSE("[Mediator] evalExpr OR: left=%d right=%d -> %s\n", e.leftIdx, e.rightIdx, res ? "TRUE" : "FALSE");
                return res;
            }
        }
        return false;
    }

    void evaluateAllRules() {
        DBG_VERBOSE("[Mediator] Evaluating %d rules...\n", _ruleCount);
        for (uint8_t i = 0; i < _ruleCount; i++) {
            const Rule& r = _rules[i];
            bool result = evalExpr(r.rootExprIdx);
            DBG_VERBOSE("[Mediator]  - Rule %d (actuator %d): %s\n", i, r.actuatorId, result ? "TRUE" : "FALSE");
            if (result) {
                ActuatorCommand cmd{r.actuatorId, r.triggerState, r.durationMs, r.priority};
                dispatch(cmd);
            }
        }
    }

    void dispatch(const ActuatorCommand& cmd) {
        int8_t ai = _findActuatorIndex(cmd.actuatorId);

        if (ai < 0) {
            DBG_ERROR("[Mediator] dispatch: Actuator ID %d not found\n", cmd.actuatorId);
            return;
        }
        DBG_VERBOSE("[Mediator] dispatch: Actuator ID %d state=%s duration=%d priority=%d\n", 
                    cmd.actuatorId, cmd.state ? "ON" : "OFF", cmd.durationMs, cmd.priority);
        // Priority 0 is a special command to reset the actuator to AUTO mode (clear manual override)
        if (cmd.priority == 0) {
            _active[ai].priority = 0;
            _activeStartTime[ai] = 0;
            _activeDuration[ai]  = 0;
            _dispatched[ai] = false; // Next rule evaluation will enforce state if needed
            return;
        }

        if (cmd.priority >= _active[ai].priority) 
        {
            bool physicalState = _actuators[ai] ? _actuators[ai]->getState() : false;
            bool stateChanged = !_dispatched[ai] || 
                                (_active[ai].state != cmd.state) || 
                                (_actuators[ai] && physicalState != cmd.state);

            if (stateChanged || cmd.priority > _active[ai].priority) {
                DBG_VERBOSE("[Mediator] dispatch: Actuator %d (prio %d) applies state=%s (phys=%s, was_prio=%d)\n", 
                          cmd.actuatorId, cmd.priority, cmd.state ? "ON" : "OFF", 
                          physicalState ? "ON" : "OFF", _active[ai].priority);
            }

            _active[ai] = cmd;
            _dispatched[ai] = true;
            _activeStartTime[ai] = millis();
            _activeDuration[ai]  = (cmd.state) ? cmd.durationMs : 0;
            if (stateChanged && _actuators[ai]) {
                _actuators[ai]->execute(cmd);
            }
        }
    }

    int8_t _findActuatorIndex(uint8_t id) const {
        for (uint8_t i = 0; i < _actuatorCount; i++)
            if (_actuators[i] && _actuators[i]->getId() == id) return (int8_t)i;
        return -1;
    }
};
