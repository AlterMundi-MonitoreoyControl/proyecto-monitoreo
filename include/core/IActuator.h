#pragma once
#include <stdint.h>

/**
 * ActuatorCommand — instrucción para un actuador.
 *
 * actuatorId : identificador del actuador destino.
 * state      : true = encender / activar; false = apagar / desactivar.
 * durationMs : tiempo activo en ms; 0 = mantener hasta próximo comando.
 * priority   : 1=timer, 2=rule, 3=manual.
 *              El mediador aplica la regla: mayor prioridad siempre gana;
 *              igual prioridad: último en escribir.
 */
struct ActuatorCommand {
    uint8_t  actuatorId;
    bool     state;
    uint32_t durationMs;
    uint8_t  priority;
};

/**
 * IActuator — interfaz abstracta de actuador.
 *
 * Implementaciones concretas:
 *   - GpioActuator     : pin GPIO local con auto-off por millis()
 *   - RelayModule2CH   : módulo de relé RS485 Modbus RTU (existente)
 */
class IActuator {
public:
    virtual ~IActuator() {}

    /** Identificador único del actuador (asignado al construir / registrar). */
    virtual uint8_t getId() const = 0;

    /** Nombre legible para logging y API. */
    virtual const char* getName() const = 0;

    /**
     * Inicializar hardware.
     * @return true si OK, false si falla.
     */
    virtual bool begin() = 0;

    /**
     * Ejecutar un comando.
     * La implementación debe respetar durationMs: si > 0, apagar
     * automáticamente después de ese tiempo (usando millis() internamente
     * o delegando en tick()).
     */
    virtual void execute(const ActuatorCommand& cmd) = 0;

    /** Retorna el estado actual (true = activo). */
    virtual bool getState() const = 0;

    /** Retorna true si el actuador está operativo (opcional). */
    virtual bool getStatus() const { return true; } 

    /**
     * Llamar desde loop() para procesar auto-off y otras tareas periódicas.
     * Implementación por defecto vacía (opcional sobrescribir).
     */
    virtual void tick() {}
};
