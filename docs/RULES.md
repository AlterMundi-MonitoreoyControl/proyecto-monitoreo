# RULES.md — Esquema de Reglas de Automatización

## Ubicación

`/data/rules.json` en SPIFFS. Se carga al arrancar y se puede recargar via `POST /rules/reload`.

---

## Esquema JSON

```json
{
  "rules": [
    {
      "expr":        <Expresión>,
      "actuator":    <uint8>,
      "state":       <bool>,
      "priority":    <1|2|3>,
      "duration_ms": <uint32>
    }
  ]
}
```

### Campos de una regla

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `expr` | object | Expresión booleana (ver abajo) |
| `actuator` | uint8 | ID del actuador a controlar |
| `state` | bool | Estado a aplicar cuando la expresión es `true` |
| `priority` | 1–3 | 1=timer, 2=regla (default), 3=manual |
| `duration_ms` | uint32 | Tiempo activo en ms; `0` = mantener mientras la condición sea true |

---

## Expresiones

### LEAF (condición simple)

```json
{
  "sensor": { "device": 0, "id": 1 },
  "cond": "LT",
  "value": 30.0
}
```

| Campo | Descripción |
|-------|-------------|
| `sensor.device` | ID de dispositivo (0 = local, N = último byte de MAC remota) |
| `sensor.id` | ID de sensor (índice asignado por SensorManager) |
| `cond` | Operador: `GT`, `LT`, `EQ`, `GTE`, `LTE` |
| `value` | Umbral de comparación (float) |

### AND

```json
{
  "op": "AND",
  "left":  { "sensor": {"device":0,"id":1}, "cond":"LT", "value":30 },
  "right": { "sensor": {"device":0,"id":2}, "cond":"GT", "value":50 }
}
```

### OR

```json
{
  "op": "OR",
  "left":  { "sensor": {"device":0,"id":0}, "cond":"GT", "value":35 },
  "right": { "sensor": {"device":1,"id":0}, "cond":"GT", "value":35 }
}
```

> Las expresiones AND/OR pueden anidarse. El árbol se limita al tamaño del pool (`RULE_EXPR_POOL = 32`).

---

## IDs de Sensores locales

Los sensorId son asignados en orden de registro por `SensorManager`. El orden depende del array `"sensors"` en `config.json`.

Ejemplo con la config por defecto:
| sensorId | Sensor |
|----------|--------|
| 0 | ModbusTH addr=45 |
| 1 | ModbusTH addr=5 |
| 2 | ModbusTH addr=10 |
| 3 | OneWire #0 |
| 4 | HD38 pin=35 |
| 5 | HD38 pin=32 |

---

## IDs de Actuadores

| ID | Actuador |
|----|----------|
| `(addr<<4)\|ch` | RelayModule2CH: addr=Modbus addr, ch=canal (0 o 1) |
| Asignado en código | GpioActuator: ID fijo en el constructor |

Para un relay Modbus addr=1, canal 0: `id = (1<<4)|0 = 16`

---

## Ejemplo completo

```json
{
  "rules": [
    {
      "expr": {
        "op": "AND",
        "left":  { "sensor": {"device":0,"id":0}, "cond":"LT", "value":25.0 },
        "right": { "sensor": {"device":0,"id":4}, "cond":"LT", "value":40.0 }
      },
      "actuator": 16,
      "state": true,
      "priority": 2,
      "duration_ms": 30000
    },
    {
      "expr": { "sensor": {"device":1,"id":0}, "cond":"GT", "value":35.0 },
      "actuator": 0,
      "state": true,
      "priority": 2,
      "duration_ms": 0
    }
  ]
}
```

---

## API REST

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| `GET` | `/rules` | Obtiene `rules.json` crudo |
| `POST` | `/rules/save` | Guarda reglas y recarga el mediador |
| `POST` | `/rules/reload` | Recarga desde SPIFFS sin modificar el archivo |
| `GET` | `/rules-editor` | WebView de edición de reglas |
| `GET` | `/actuator/status` | Array JSON con estado de actuadores |
| `POST` | `/actuator/command` | Enviar comando manual `{"id":1,"state":true,"duration":5000}` |

---

## Prioridades

| Valor | Fuente | Comportamiento |
|-------|--------|----------------|
| 1 | Timer | Menor prioridad. Una regla o comando manual lo puede anular. |
| 2 | Regla | Prioridad media. Comandos manuales lo anulan. |
| 3 | Manual | Mayor prioridad. Siempre gana. `durationMs=0` persiste hasta el próximo manual. |
