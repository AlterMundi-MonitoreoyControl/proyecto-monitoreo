#pragma once
#include "SensorKey.h"
#include "MediatorConfig.h"
#include <stdint.h>

/**
 * Rule Engine — árbol de expresiones booleanas sobre lecturas de sensores.
 *
 * Soporta:
 *   - LEAF  : condición atómica (sensor op threshold)
 *   - AND   : ambas sub-expresiones deben ser verdaderas
 *   - OR    : al menos una sub-expresión debe ser verdadera
 *
 * Las expresiones se almacenan en un pool plano (RuleExpr[RULE_EXPR_POOL]).
 * leftIdx / rightIdx son índices dentro de ese pool.
 * El evaluador recorre el árbol recursivamente.
 */

/** Operadores de comparación para condiciones de hoja */
enum class CondOp : uint8_t {
    GT  = 0,   ///< sensor > threshold
    LT  = 1,   ///< sensor < threshold
    EQ  = 2,   ///< sensor == threshold  (con tolerancia 1e-4)
    GTE = 3,   ///< sensor >= threshold
    LTE = 4,   ///< sensor <= threshold
};

/** Condición atómica: una magnitud de sensor comparada con un umbral */
struct Condition {
    SensorKey key;
    CondOp    op;
    float     threshold;
};

/** Tipo de nodo en el árbol de expresiones */
enum class ExprType : uint8_t {
    LEAF = 0,
    AND  = 1,
    OR   = 2,
};

/**
 * Nodo de expresión en el pool.
 *
 * Si type == LEAF : se usa el campo `cond`.
 * Si type == AND/OR: se usan leftIdx y rightIdx (índices en el pool).
 */
struct RuleExpr {
    ExprType  type;
    Condition cond;       ///< válido si type == LEAF
    uint8_t   leftIdx;    ///< índice en pool, válido si AND/OR
    uint8_t   rightIdx;   ///< índice en pool, válido si AND/OR
};

/**
 * Regla completa: una expresión booleana → comando a un actuador.
 *
 * rootExprIdx  : índice de la expresión raíz en el pool.
 * actuatorId   : actuador a controlar cuando la regla se dispara.
 * triggerState : estado a aplicar cuando la expresión es true.
 * priority     : prioridad del comando generado (1=timer, 2=rule, 3=manual).
 * durationMs   : duración del comando; 0 = mantener mientras la regla siga true.
 */
struct Rule {
    uint8_t  rootExprIdx;
    uint8_t  actuatorId;
    bool     triggerState;
    uint8_t  priority;
    uint32_t durationMs;
};
