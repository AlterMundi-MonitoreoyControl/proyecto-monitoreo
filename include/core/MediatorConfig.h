#pragma once

/**
 * MediatorConfig — constantes de tamaño para el ControlMediator.
 *
 * Ajustar según restricciones de RAM del target.
 * ESP32: ~300 KB de SRAM disponible; estos valores consumen ~2 KB estáticos.
 */

/** Máximo de expresiones en el pool del árbol de reglas */
#define RULE_EXPR_POOL   32

/** Máximo de reglas activas */
#define RULE_MAX         16

/** Máximo de actuadores registrados */
#define ACTUATOR_MAX     16

/**
 * Tamaño del state-store plano (array de floats).
 * El índice se calcula como: key.toU32() % STATE_STORE_SIZE
 * Debe ser >= cantidad total de magnitudes de sensores esperadas.
 */
#define STATE_STORE_SIZE 64
