#pragma once
#include "ControlMediator.h"
#include "Rule.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

/**
 * RuleLoader — parses /rules.json from SPIFFS and populates ControlMediator.
 *
 * Supported expression formats:
 *
 *   LEAF (single condition):
 *     { "sensor": {"device":0,"id":97}, "cond":"LT", "value":30.0 }
 *
 *   AND / OR with 2 explicit branches (legacy):
 *     { "op":"AND", "left": <expr>, "right": <expr> }
 *
 *   AND / OR with N conditions (flat array, up to 4+):
 *     { "op":"AND", "conditions": [ <leaf>, <leaf>, <leaf>, <leaf> ] }
 *     Builds a left-deep tree: AND(c0, AND(c1, AND(c2, c3)))
 *     Works up to RULE_EXPR_POOL capacity.
 *
 * Both formats can be mixed within the same rule file.
 *
 * Operators: "GT" "LT" "EQ" "GTE" "LTE"
 */
class RuleLoader {
public:
    /**
     * @param mediator   Mediator to populate.
     * @param clearFirst If true, clears existing rules first.
     * @return Number of rules loaded, or -1 on file error.
     */
    static int load(ControlMediator& mediator, bool clearFirst = true) {
        if (!SPIFFS.exists("/rules.json")) return 0;

        File f = SPIFFS.open("/rules.json", "r");
        if (!f) return -1;

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        if (err) return -1;

        if (clearFirst) mediator.clearRules();

        JsonArray rules = doc["rules"].as<JsonArray>();
        if (rules.isNull()) return 0;

        int loaded = 0;
        for (JsonObject ruleObj : rules) {
            if (_loadRule(mediator, ruleObj)) loaded++;
        }
        return loaded;
    }

private:
    // ── Rule ───────────────────────────────────────────────────────────────
    static bool _loadRule(ControlMediator& mediator, JsonObject obj) {
        if (!obj["expr"].is<JsonObject>() && !obj["expr"].is<JsonVariant>()) return false;

        uint8_t rootIdx = _loadExpr(mediator, obj["expr"]);
        if (rootIdx == 0xFF) return false;

        Rule rule;
        rule.rootExprIdx  = rootIdx;
        rule.actuatorId   = obj["actuator"]   | (uint8_t)0;
        rule.triggerState = obj["state"]       | true;
        rule.priority     = obj["priority"]    | (uint8_t)2;
        rule.durationMs   = obj["duration_ms"] | (uint32_t)0;

        mediator.addRule(rule);
        return true;
    }

    // ── Expression dispatcher ──────────────────────────────────────────────
    static uint8_t _loadExpr(ControlMediator& mediator, JsonVariant node) {
        if (node.isNull()) return 0xFF;

        const char* op = node["op"] | "";

        // ── Flat N-condition array ─────────────────────────────────────────
        // { "op":"AND", "conditions":[leaf, leaf, leaf, leaf] }
        if (node["conditions"].is<JsonArray>()) {
            return _loadFlatConditions(mediator, node["conditions"], op);
        }

        // ── Two-branch AND / OR ────────────────────────────────────────────
        if (strcmp(op, "AND") == 0 || strcmp(op, "OR") == 0) {
            uint8_t leftIdx  = _loadExpr(mediator, node["left"]);
            uint8_t rightIdx = _loadExpr(mediator, node["right"]);
            if (leftIdx == 0xFF || rightIdx == 0xFF) return 0xFF;

            RuleExpr expr;
            expr.type     = (strcmp(op, "AND") == 0) ? ExprType::AND : ExprType::OR;
            expr.leftIdx  = leftIdx;
            expr.rightIdx = rightIdx;
            expr.cond     = {{0, 0}, CondOp::GT, 0.0f};
            return mediator.addExpr(expr);
        }

        // ── LEAF ──────────────────────────────────────────────────────────
        return _loadLeaf(mediator, node);
    }

    /**
     * Build a left-deep binary tree from a flat conditions array.
     * Supports any number of conditions (limited by RULE_EXPR_POOL).
     *
     * Example with 4 conditions and op="AND":
     *   AND( AND( AND(c0, c1), c2 ), c3 )
     */
    static uint8_t _loadFlatConditions(ControlMediator& mediator,
                                        JsonArray conds, const char* op) {
        if (conds.size() == 0) return 0xFF;

        ExprType combineType = (strcmp(op, "OR") == 0) ? ExprType::OR : ExprType::AND;

        // Start with the first leaf
        uint8_t result = _loadLeaf(mediator, conds[0]);
        if (result == 0xFF) return 0xFF;

        // Fold remaining conditions left-to-right
        for (size_t i = 1; i < conds.size(); i++) {
            uint8_t rightIdx = _loadLeaf(mediator, conds[i]);
            if (rightIdx == 0xFF) return 0xFF;

            RuleExpr compound;
            compound.type     = combineType;
            compound.leftIdx  = result;
            compound.rightIdx = rightIdx;
            compound.cond     = {{0, 0}, CondOp::GT, 0.0f};

            result = mediator.addExpr(compound);
            if (result == 0xFF) return 0xFF;  // pool exhausted
        }
        return result;
    }

    // ── LEAF builder ──────────────────────────────────────────────────────
    static uint8_t _loadLeaf(ControlMediator& mediator, JsonVariant node) {
        JsonObject sensorObj = node["sensor"];
        if (sensorObj.isNull()) return 0xFF;

        RuleExpr expr;
        expr.type = ExprType::LEAF;
        expr.cond.key.deviceId  = sensorObj["device"] | (uint8_t)0;
        expr.cond.key.sensorId  = sensorObj["id"]     | (uint16_t)0;
        expr.cond.key.varId     = sensorObj["var"]    | (uint8_t)0;
        expr.cond.threshold     = node["value"]       | 0.0f;
        expr.cond.op            = _parseCondOp(node["cond"] | "GT");
        expr.leftIdx = expr.rightIdx = 0;
        return mediator.addExpr(expr);
    }

    static CondOp _parseCondOp(const char* s) {
        if (!s)                   return CondOp::GT;
        if (strcmp(s, "LT")  == 0) return CondOp::LT;
        if (strcmp(s, "EQ")  == 0) return CondOp::EQ;
        if (strcmp(s, "GTE") == 0) return CondOp::GTE;
        if (strcmp(s, "LTE") == 0) return CondOp::LTE;
        return CondOp::GT;
    }
};
