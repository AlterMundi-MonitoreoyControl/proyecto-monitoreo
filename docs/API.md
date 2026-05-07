# API Reference

**Base URL:** `http://{IP_DEL_ESP32}` · **Puerto:** 80 · **CORS:** `*`

**Auth:** HTTP Basic Auth opt-in (`admin:<password>`). Sin contraseña configurada, todos los endpoints son libres. Los endpoints 🔒 la requieren una vez que se establece via `POST /api/admin/password`.

---

## Índice

| Método | Ruta | Auth | Descripción |
|--------|------|------|-------------|
| GET | [/actual](#get-actual) | | Lecturas de sensores y actuadores |
| GET | [/api/status](#get-apistatus) | | Estado general del sistema |
| GET | [/config](#get-config) | | Configuración completa |
| POST | [/config](#post-config) | 🔒 | Actualizar configuración |
| POST | [/config/reset](#post-configreset) | 🔒 | Reset a valores de fábrica |
| GET | [/calibrate-scd30](#get-calibrate-scd30) | 🔒 | Calibrar sensor CO2 |
| POST | [/actuator/command](#post-actuatorcommand) | 🔒 | Comando a actuador |
| GET | [/actuator/status](#get-actuatorstatus) | | Estado de actuadores |
| GET | [/api/relays](#get-apirelays) | | Lista de relés |
| POST | [/api/relay/toggle](#post-apirelaytoggle) | 🔒 | Toggle de canal de relé |
| GET | [/rules](#get-rules) | | Reglas de automatización |
| POST | [/rules/save](#post-rulessave) | 🔒 | Guardar y aplicar reglas |
| POST | [/rules/reload](#post-rulesreload) | 🔒 | Recargar reglas sin modificar |
| GET | [/rules-editor](#get-rules-editor) | | Editor visual de reglas |
| GET | [/api/admin/info](#get-apiadmininfo) | | Estado de contraseña admin |
| POST | [/api/admin/password](#post-apiadminpassword) | 🔒 | Cambiar contraseña admin |
| GET | [/wifi](#get-wifi) | | Escanear redes WiFi |
| POST | [/save](#post-save) | | Guardar credenciales WiFi |
| GET | [/espnow/status](#get-espnowstatus) | | Estado ESP-NOW |
| POST | [/restart](#post-restart) | 🔒 | Reiniciar dispositivo |

---

## Sensores

### GET /actual

Lecturas de todos los sensores activos, estado de actuadores y uptime.

```json
{
  "sensors": [
    {
      "type": "SCD30",
      "id": "scd30_0",
      "icon": "🌬️",
      "active": true,
      "error": false,
      "readings": [
        { "label": "Temp",    "value": "23.4", "unit": "°C",  "status": "ok", "key": 16843009, "key_device": 1, "key_sensor": 1, "key_var": 0 },
        { "label": "Humedad", "value": "58.2", "unit": "%",   "status": "ok", "key": 16843010, "key_device": 1, "key_sensor": 1, "key_var": 1 },
        { "label": "CO2",     "value": "620",  "unit": "ppm", "status": "ok", "key": 16843011, "key_device": 1, "key_sensor": 1, "key_var": 2 }
      ]
    },
    {
      "type": "Entradas Digitales",
      "id": "modbus_relay_1",
      "icon": "🔌",
      "error": false,
      "readings": [
        { "label": "IN 1", "value": "1", "unit": "", "status": "ok", "key_device": 212, "key_sensor": 1, "key_var": 10 },
        { "label": "IN 2", "value": "0", "unit": "", "status": "ok", "key_device": 212, "key_sensor": 1, "key_var": 11 }
      ]
    }
  ],
  "wifi_status": "connected",
  "uptime": "2h 15m 30s",
  "actuators": [
    { "id": 1, "name": "Ventilador", "state": false, "status": "ok", "key_actuator": 1 }
  ]
}
```

**`readings[].status`:** `"ok"` / `"warn"` (CO2 > 1000 ppm) / `"bad"` (CO2 > 1500 ppm)

**`key_var`:** 0=TEMP · 1=HUMEDAD · 2=CO2 · 3=HUMEDAD_SUELO · 10=DIN1 · 11=DIN2

Los sensores capacitivos y HD38 incluyen además `diagnostics` y `calibration` en el objeto del sensor. Las entradas digitales de los relés Modbus aparecen como sensores de tipo `"Entradas Digitales"`.

---

### GET /calibrate-scd30 🔒

Calibra el sensor SCD30 a 400 ppm. Ejecutar en exterior o ambiente bien ventilado; esperar 2-3 min para estabilización.

```json
// 200 — éxito
{ "status": "success", "sensor_type": "SCD30", "sensor_detected": true, "calibration_performed": true, "target_co2": 400, "note": "Allow 2-3 minutes for sensor to stabilize after calibration" }

// 503 — SCD30 no encontrado
{ "status": "error", "message": "SCD30 sensor not found or not active", "sensor_detected": false, "calibration_performed": false }

// 500 — sensor encontrado pero calibración fallida
{ "status": "error", "message": "Calibration not supported or failed for SCD30", "sensor_detected": true, "calibration_performed": false }
```

---

## Sistema

### GET /api/status

```json
{
  "device_name": "moni-80F3DAACC7D4",
  "device_id": "80F3DAACC7D4",
  "firmware_version": "0.2.12",
  "wifi_connected": true,
  "wifi_ssid": "MiRed",
  "wifi_ip": "192.168.1.100",
  "wifi_rssi": -55,
  "signal_class": "signal-good",
  "active_sensors": 2,
  "total_sensors": 3,
  "total_actuators": 2,
  "total_active_actuators": 1,
  "uptime": "2h 15m"
}
```

`signal_class`: `signal-excellent` (> −50) · `signal-good` (> −60) · `signal-fair` (> −70) · `signal-weak`

---

### POST /restart 🔒

`200 text/plain` → `Restarting ESP32...`

El dispositivo tarda ~5 s en volver a estar disponible.

---

## Configuración

### GET /config

Devuelve el `config.json` completo del dispositivo.

```json
{
  "incubator_name": "moni-80F3DAACC7D4",
  "hash": "80F3DAACC7D4",
  "ssid": "MiRed",
  "passwd": "ToChange",
  "min_temperature": 37.3,
  "max_temperature": 37.7,
  "min_hum": 55,
  "max_hum": 65,
  "sensors": [
    { "type": "scd30",       "enabled": true,  "config": {} },
    { "type": "bme280",      "enabled": false, "config": {} },
    { "type": "onewire",     "enabled": false, "config": { "pin": 14, "scan": true } },
    { "type": "capacitive",  "enabled": false, "config": { "pin": 34, "name": "Soil1" } },
    { "type": "hd38",        "enabled": false, "config": { "analog_pins": [35, 32] } },
    { "type": "modbus_th",   "enabled": false, "config": { "addresses": [2,3,4,5] } },
    { "type": "modbus_7in1", "enabled": false, "config": { "address": 1 } }
  ],
  "relays": [
    { "type": "relay_2ch", "enabled": false, "config": { "alias": "Relay 01", "address": 1 } }
  ],
  "rs485": { "enabled": false, "rx_pin": 16, "tx_pin": 17, "de_pin": 18, "baudrate": 9600, "raw_send_enabled": false },
  "espnow_enabled": false,
  "espnow_force_mode": "",
  "espnow_channel": 1,
  "beacon_interval_ms": 2000,
  "discovery_timeout_ms": 15000,
  "send_interval_ms": 30000,
  "grafana_ping_url": "",
  "current_wifi_channel": 6
}
```

`current_wifi_channel` es de solo lectura (calculado en runtime).

**Códigos:** `200` · `500` (error leyendo archivo)

---

### POST /config 🔒

Merge parcial: enviar solo los campos a modificar.

```
Content-Type: application/json
{ "ssid": "NuevaRed", "passwd": "password123" }
```

- Cambiar `ssid` inicia reconexión con rollback automático si las credenciales fallan
- `passwd`: 8-63 caracteres (WPA2), o vacío para no modificarla
- Cambios en sensores, ESP-NOW o pines RS485 requieren `POST /restart`

`200 text/plain` → `Configuration updated successfully. Some changes require restart.`

**Códigos:** `200` · `400` (JSON inválido, password fuera de rango) · `401` · `500`

---

### POST /config/reset 🔒

Restaura valores de fábrica y reinicia el dispositivo.

```json
// 200
{ "success": true, "message": "Configuration reset to defaults. Restarting..." }
```

---

## Actuadores

### POST /actuator/command 🔒

```json
// Request
{ "id": 1, "state": true, "duration": 5000, "priority": 3 }

// Response 200
{ "ok": true, "id": 1, "state": true }
```

| Campo | Tipo | Default | Descripción |
|-------|------|---------|-------------|
| `id` | int | — | ID del actuador (ver `/actuator/status`) |
| `state` | bool | — | `true`=ON, `false`=OFF |
| `duration` | int (ms) | 0 | 0 = permanente; si > 0 revierte al estado anterior |
| `priority` | int 1–3 | 3 | 3 = manual (máxima) |

**Códigos:** `200` · `400` · `401`

---

### GET /actuator/status

```json
[
  { "id": 1, "name": "Ventilador", "state": false, "status": true },
  { "id": 2, "name": "Bomba",      "state": true,  "status": true }
]
```

---

## Relés

### GET /api/relays

Devuelve relés Modbus y GPIO. El campo `type` determina la estructura.

```json
[
  {
    "type": "relay_2ch",
    "address": 1, "alias": "Ventilador",
    "active": true,
    "state": [true, false],
    "input_state": [false, true]
  },
  {
    "type": "gpio",
    "address": 25, "alias": "Luz",
    "state": [false]
  }
]
```

- `relay_2ch`: `state[2]` (salidas CH1/CH2) + `input_state[2]` (entradas IN1/IN2)
- `gpio`: `state[1]` (un canal)

---

### POST /api/relay/toggle 🔒

**Query params:** `addr` (dirección Modbus o pin GPIO) · `ch` (0 ó 1; siempre 0 para GPIO)

```bash
POST /api/relay/toggle?addr=1&ch=0
```

`200` → `OK` · `400` → params faltantes · `401` · `404` → relé no encontrado

---

## Reglas

Ver [RULES.md](RULES.md) para el schema completo de reglas.

### GET /rules

Devuelve `rules.json`. Si no existe: `{ "rules": [] }`

**Códigos:** `200` · `500`

---

### POST /rules/save 🔒

Guarda el JSON recibido como `rules.json` y aplica las reglas inmediatamente.

```json
// Request
{ "rules": [...] }

// Response 200
{ "ok": true, "rules_loaded": 3 }
```

**Códigos:** `200` · `400` · `401` · `500`

---

### POST /rules/reload 🔒

Recarga las reglas desde `rules.json` sin modificar el archivo.

```json
// 200
{ "ok": true, "rules_loaded": 3 }
// 500
{ "ok": false, "error": "Cannot open rules.json" }
```

---

### GET /rules-editor

SPA para editar reglas visualmente. `200 text/html`

---

## Administración

### GET /api/admin/info

```json
{ "configured": false }
```

---

### POST /api/admin/password 🔒

```json
// Request
{ "current": "passwordActual", "new": "miNuevaPassword123" }

// Response 200 text/plain
"Admin password updated"
```

- `new`: 8-64 caracteres, diferente a la actual
- `current`: requerido solo si ya hay contraseña configurada

**Códigos:** `200` · `400` (longitud inválida, sin cambios) · `401` · `500`

---

## WiFi

### GET /wifi

Escaneo asíncrono: hacer polling hasta obtener `200`.

| Código | Significado |
|--------|-------------|
| `202` | Scan en progreso — reintentar |
| `200` | Scan completo |
| `408` | Timeout (~15 s) |
| `503` | WiFi no disponible |

```json
// 200
{ "message": "success", "networks": [{ "ssid": "MiRed", "rssi": -45 }] }
// 202
{ "message": "scan in progress", "status": "scanning" }
```

---

### POST /save

Captive portal. Guarda credenciales e intenta conectar.

```
Content-Type: application/x-www-form-urlencoded
ssid=MiRed&password=mipassword
```

**Códigos:** `200 text/html` · `400` (SSID vacío)

---

## ESP-NOW

### GET /espnow/status

Solo disponible en builds con `ENABLE_ESPNOW`.

```json
{
  "enabled": true,
  "mode": "gateway",
  "forced_mode": "",
  "mac_address": "80:F3:DA:AC:C7:D4",
  "channel": 6,
  "paired": true,
  "peer_count": 3
}
```

- `mode`: `"gateway"` recibe datos · `"sensor"` los envía
- `forced_mode`: vacío = auto-detección; `"gateway"` o `"sensor"` para forzar
- `peer_count`: cantidad de nodos sensor (solo modo gateway)
- `paired`: si el nodo sensor se emparejó con un gateway (solo modo sensor)

---

## Páginas HTML

| Ruta | Descripción |
|------|-------------|
| `GET /` | Página principal (→ `/wifi-setup` si no hay WiFi) |
| `GET /data` | Dashboard de sensores |
| `GET /settings` | Formulario de configuración |
| `GET /rules-editor` | Editor de reglas |
| `GET /favicon.svg` | Ícono SVG |
| `GET /style.css` | Estilos |
| `GET /config.js` | JS de configuración |

Rutas no reconocidas → `302` a `/`.

---

## Ejemplos

```bash
# Lecturas actuales
curl http://192.168.4.1/actual | jq '.sensors[].readings'

# Estado del sistema
curl http://192.168.4.1/api/status | jq

# Cambiar red WiFi
curl -X POST http://192.168.4.1/config \
  -H "Content-Type: application/json" \
  -d '{"ssid":"NuevaRed","passwd":"password123"}'

# Habilitar sensor OneWire + restart
curl -X POST http://192.168.4.1/config \
  -H "Content-Type: application/json" \
  -d '{"sensors":[{"type":"onewire","enabled":true,"config":{"pin":4,"scan":true}}]}'
curl -X POST http://192.168.4.1/restart

# Toggle relé Modbus canal 0
curl -X POST "http://192.168.4.1/api/relay/toggle?addr=1&ch=0"

# Comando a actuador (ON por 10 s)
curl -X POST http://192.168.4.1/actuator/command \
  -H "Content-Type: application/json" \
  -d '{"id":1,"state":true,"duration":10000,"priority":3}'

# Guardar reglas
curl -X POST http://192.168.4.1/rules/save \
  -H "Content-Type: application/json" \
  -d '{"rules":[{"id":1,"name":"Ventilador","condition":{"key":16843009,"operator":">","value":30},"action":{"actuator_id":1,"state":true,"priority":1}}]}'

# Configurar contraseña de admin (primer uso)
curl -X POST http://192.168.4.1/api/admin/password \
  -H "Content-Type: application/json" \
  -d '{"new":"miPassword123"}'

# Endpoint protegido con contraseña
curl -u admin:miPassword123 -X POST http://192.168.4.1/restart

# Calibrar CO2
curl http://192.168.4.1/calibrate-scd30 | jq

# Factory reset
curl -X POST http://192.168.4.1/config/reset
```

---

## Errores frecuentes

| Síntoma | Causa | Fix |
|---------|-------|-----|
| Cambio de config sin efecto | Requiere restart | `POST /restart` tras cambiar sensores / ESP-NOW / RS485 |
| Timeout (~5 s) | Dispositivo ocupado | Reintentar |
| Pérdida de conexión tras cambiar SSID | Credenciales incorrectas | Esperar 30 s (rollback automático) o reconectar al AP |
| `401` inesperado | Contraseña admin activa | Usar `-u admin:<password>` · verificar con `GET /api/admin/info` |
