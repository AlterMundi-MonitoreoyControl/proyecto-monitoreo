# Configuration Reference

config.json structure completa y parámetros del sistema.

**Ubicación:** `/config.json` en SPIFFS
**Formato:** JSON
**Editable:** Vía web (`/config`) o API POST

## config.json Completo

```json
{
  "ssid": "WiFi_Network",
  "passwd": "WiFi_Password",
  "incubator_name": "moni-AABBCCDDEE",
  "hash": "AABBCCDDEE",
  "min_temperature": 37.3,
  "max_temperature": 37.7,
  "min_hum": 55,
  "max_hum": 65,
  "rotation_duration": 50000,
  "rotation_period": 3600000,
  "incubation_period": 18,
  "tray_one_date": 0,
  "tray_two_date": 0,
  "tray_three_date": 0,
  "rs485_enabled": false,
  "rs485_rx": 16,
  "rs485_tx": 17,
  "rs485_baud": 9600,
  "espnow_enabled": false,
  "espnow_force_mode": "",
  "espnow_channel": 1,
  "beacon_interval_ms": 2000,
  "discovery_timeout_ms": 15000,
  "send_interval_ms": 30000,
  "grafana_ping_url": "http://192.168.1.1/ping",
  "sensors": [
    {
      "type": "scd30",
      "enabled": true,
      "config": {}
    },
    {
      "type": "bme280",
      "enabled": false,
      "config": {}
    },
    {
      "type": "capacitive",
      "enabled": true,
      "config": {
        "pin": 34,
        "name": "Soil1"
      }
    },
    {
      "type": "onewire",
      "enabled": false,
      "config": {
        "pin": 4,
        "scan": true
      }
    }
  ]
}
```

## Parámetros

### WiFi

#### `ssid` (string)
**Descripción:** SSID de red WiFi
**Default:** `"ToChange"`
**Restart:** No (hot-reload con validation)
**Notas:**
- Cambio activa `WiFiManager::onChange()`
- Intenta conexión con nuevas credenciales
- Rollback automático si falla (30s timeout)

#### `passwd` (string)
**Descripción:** Password de red WiFi
**Default:** `"ToChange"`
**Restart:** No (hot-reload con validation)
**Notas:** Mismo comportamiento que `ssid`

---

### Device Identity

#### `incubator_name` (string)
**Descripción:** Nombre del dispositivo para Grafana
**Default:** `"moni-{MAC}"`
**Restart:** No
**Formato:** `moni-AABBCCDDEE` (MAC sin colons)
**Uso:** Tag `device` en InfluxDB line protocol
**Ejemplo:**
```
medicionesCO2,device=moni-80F3DAAD temp=25.3,...
```

#### `hash` (string)
**Descripción:** MAC address corta (12 hex chars)
**Default:** Auto-generado de MAC
**Restart:** No
**Uso:** Interno, para generación de AP name

---

### Temperature/Humidity Ranges (Legacy)

#### `min_temperature`, `max_temperature` (float)
**Descripción:** Rangos de temperatura (°C)
**Default:** `37.3`, `37.7`
**Restart:** No
**Uso:** Legacy (incubator project), no usado actualmente
**Notas:** Mantenido por compatibilidad

#### `min_hum`, `max_hum` (int)
**Descripción:** Rangos de humedad (%)
**Default:** `55`, `65`
**Restart:** No
**Uso:** Legacy, no usado

---

### Rotation/Incubation (Legacy)

#### `rotation_duration` (int, ms)
**Default:** `50000`
**Uso:** No usado

#### `rotation_period` (int, ms)
**Default:** `3600000`
**Uso:** No usado

#### `incubation_period` (int, días)
**Default:** `18`
**Uso:** No usado

#### `tray_one_date`, `tray_two_date`, `tray_three_date` (int, unix timestamp)
**Default:** `0`
**Uso:** No usado

**Nota:** Estos parámetros son heredados de proyecto incubator original. No afectan funcionalidad actual de monitoreo.

---

### RS485

#### `rs485_enabled` (bool)
**Descripción:** Habilita salida RS485
**Default:** `false`
**Restart:** Sí
**Requires:** Build flag `ENABLE_RS485`
**Notas:** Compilar con env `esp32dev_rs485` o `esp32dev_multi`

#### `rs485_rx` (int, GPIO pin)
**Descripción:** Pin RX de RS485
**Default:** `16`
**Restart:** Sí
**Valid:** 0-39 (GPIO válidos ESP32)

#### `rs485_tx` (int, GPIO pin)
**Descripción:** Pin TX de RS485
**Default:** `17`
**Restart:** Sí
**Valid:** 0-39

#### `rs485_baud` (int, baud rate)
**Descripción:** Velocidad serial RS485
**Default:** `9600`
**Restart:** Sí
**Valid:** 9600, 19200, 38400, 57600, 115200
**Formato:** 8N1 (hardcoded)

**Ejemplo RS485 output:**
```
SCD30 - Temp: 25.3°C Humedad: 60.5% CO2: 450ppm\r\n
```

---

### ESP-NOW

#### `espnow_enabled` (bool)
**Descripción:** Habilita ESP-NOW mesh
**Default:** `false`
**Restart:** Sí
**Requires:** Build flag `ENABLE_ESPNOW`

#### `espnow_force_mode` (string)
**Descripción:** Fuerza modo ESP-NOW
**Default:** `""` (auto-detect)
**Valid:** `""`, `"gateway"`, `"sensor"`
**Restart:** Sí
**Notas:**
- `""`: Auto-detection (WiFi + Grafana ping test)
- `"gateway"`: Force gateway mode
- `"sensor"`: Force sensor mode

#### `espnow_channel` (int)
**Descripción:** Canal WiFi para ESP-NOW
**Default:** `1`
**Valid:** 1-13 (región-dependiente)
**Restart:** Sí
**Crítico:** Gateway y sensores deben usar mismo canal
**Notas:**
- Gateway usa canal de WiFi network si conectado
- Sensor fuerza canal vía `esp_wifi_set_channel()`

#### `beacon_interval_ms` (int, ms)
**Descripción:** Intervalo de beacon broadcast (gateway)
**Default:** `2000`
**Min:** 500 (recomendado)
**Max:** 10000
**Restart:** No
**Uso:** Gateway mode solamente

#### `discovery_timeout_ms` (int, ms)
**Descripción:** Timeout para discovery (sensor)
**Default:** `15000`
**Restart:** No
**Uso:** Sensor mode solamente

#### `send_interval_ms` (int, ms)
**Descripción:** Intervalo de envío de datos (sensor)
**Default:** `30000`
**Min:** 5000 (recomendado)
**Restart:** No
**Uso:** Sensor mode solamente

#### `grafana_ping_url` (string, URL)
**Descripción:** URL para test de gateway (auto-detect)
**Default:** `"http://192.168.1.1/ping"`
**Restart:** No
**Formato:** HTTP URL completa
**Test:** GET, expect 2xx
**Timeout:** 5s

---

### Sensors Array

#### `sensors` (array)
**Descripción:** Lista de sensores habilitados
**Default:** SCD30 enabled, resto disabled
**Restart:** Sí (para cambios en sensores)

**Estructura:**
```json
{
  "type": "sensor_type",
  "enabled": true/false,
  "config": {sensor-specific params}
}
```

#### Sensor: SCD30

```json
{
  "type": "scd30",
  "enabled": true,
  "config": {}
}
```

**Config params:** Ninguno
**Hardware:** I2C (SDA=21, SCL=22)
**Mide:** CO2, temperatura, humedad
**Librería:** Adafruit_SCD30

#### Sensor: BME280

```json
{
  "type": "bme280",
  "enabled": false,
  "config": {}
}
```

**Config params:** Ninguno
**Hardware:** I2C (SDA=21, SCL=22), address 0x76 o 0x77
**Mide:** Temperatura, humedad, presión*
**Librería:** Adafruit_BME280
*Presión leída pero no enviada a Grafana

#### Sensor: Capacitive

```json
{
  "type": "capacitive",
  "enabled": true,
  "config": {
    "pin": 34,
    "name": "Soil1"
  }
}
```

**Config params:**
- `pin` (int): GPIO ADC pin (default 34)
- `name` (string, opcional): Identificador del sensor

**Hardware:** ADC pin (34 recomendado)
**Mide:** Humedad de suelo (0-100%)
**Notas:** Lectura invertida (4095=seco, 0=mojado)

#### Sensor: OneWire (DS18B20)

```json
{
  "type": "onewire",
  "enabled": false,
  "config": {
    "pin": 4,
    "scan": true
  }
}
```

**Config params:**
- `pin` (int): GPIO OneWire data (default 4)
- `scan` (bool): Auto-detect sensores en bus

**Hardware:**
- Data pin + pull-up 4.7kΩ a 3.3V
- Múltiples DS18B20 en paralelo (max ~10)

**Mide:** Temperatura
**Librería:** DallasTemperature, OneWire
**Notas:**
- Scan detecta todos los sensores en bus
- Cada sensor envía dato separado a Grafana
- Device IDs basados en ROM address

---

## Migración Automática

Si config.json no tiene array `sensors`, se crea automáticamente:

```json
{
  "sensors": [
    {
      "type": "capacitive",
      "enabled": true,
      "config": {"pin": 34, "name": "Soil1"}
    },
    {
      "type": "onewire",
      "enabled": false,
      "config": {"pin": 4, "scan": true}
    },
    {
      "type": "scd30",
      "enabled": true,
      "config": {}
    },
    {
      "type": "bme280",
      "enabled": false,
      "config": {}
    }
  ]
}
```

**Notas:**
- Migración ocurre en `ConfigFile::loadOrCreateDefault()`
- Config se guarda a SPIFFS inmediatamente
- Sin versionado, migración siempre se intenta

---

## Defaults Completos

Cuando se crea config desde cero (factory reset o primer boot):

```json
{
  "ssid": "ToChange",
  "passwd": "ToChange",
  "incubator_name": "moni-{MAC}",
  "hash": "{MAC}",
  "min_temperature": 37.3,
  "max_temperature": 37.7,
  "min_hum": 55,
  "max_hum": 65,
  "rotation_duration": 50000,
  "rotation_period": 3600000,
  "incubation_period": 18,
  "tray_one_date": 0,
  "tray_two_date": 0,
  "tray_three_date": 0,
  "rs485_enabled": false,
  "rs485_rx": 16,
  "rs485_tx": 17,
  "rs485_baud": 9600,
  "espnow_enabled": false,
  "espnow_force_mode": "",
  "espnow_channel": 1,
  "beacon_interval_ms": 2000,
  "discovery_timeout_ms": 15000,
  "send_interval_ms": 30000,
  "grafana_ping_url": "http://192.168.1.1/ping",
  "sensors": [
    {"type": "scd30", "enabled": true, "config": {}},
    {"type": "bme280", "enabled": false, "config": {}},
    {"type": "capacitive", "enabled": true, "config": {"pin": 34}},
    {"type": "onewire", "enabled": false, "config": {"pin": 4, "scan": true}}
  ]
}
```

---

## Hot-Reload vs Restart

### Hot-Reload (no restart needed):
- `ssid`, `passwd` (con validation fallback)
- `beacon_interval_ms`
- `discovery_timeout_ms`
- `send_interval_ms`
- `grafana_ping_url`

### Restart Required:
- `sensors` (cualquier cambio)
- `rs485_enabled`, `rs485_rx`, `rs485_tx`, `rs485_baud`
- `espnow_enabled`, `espnow_force_mode`, `espnow_channel`

**Cómo reiniciar:** POST a `/restart`

---

## Ejemplos de Configuración

### Sensor simple (solo SCD30)
```json
{
  "ssid": "MyWiFi",
  "passwd": "password123",
  "incubator_name": "sensor-cocina",
  "sensors": [
    {"type": "scd30", "enabled": true, "config": {}}
  ]
}
```

### Multi-sensor con RS485
```json
{
  "ssid": "MyWiFi",
  "passwd": "password123",
  "rs485_enabled": true,
  "rs485_baud": 9600,
  "sensors": [
    {"type": "scd30", "enabled": true, "config": {}},
    {"type": "bme280", "enabled": true, "config": {}},
    {"type": "capacitive", "enabled": true, "config": {"pin": 34}}
  ]
}
```

### Gateway ESP-NOW
```json
{
  "ssid": "MyWiFi",
  "passwd": "password123",
  "espnow_enabled": true,
  "espnow_channel": 6,
  "grafana_ping_url": "http://192.168.1.100:8086/ping",
  "sensors": [
    {"type": "scd30", "enabled": true, "config": {}}
  ]
}
```

### Sensor ESP-NOW (sin WiFi)
```json
{
  "ssid": "ToChange",
  "espnow_enabled": true,
  "espnow_force_mode": "sensor",
  "espnow_channel": 6,
  "send_interval_ms": 60000,
  "sensors": [
    {"type": "scd30", "enabled": true, "config": {}}
  ]
}
```

### OneWire chain (múltiples DS18B20)
```json
{
  "ssid": "MyWiFi",
  "passwd": "password123",
  "sensors": [
    {
      "type": "onewire",
      "enabled": true,
      "config": {
        "pin": 4,
        "scan": true
      }
    }
  ]
}
```

---

## Validación

**JSON parsing:**
- Usa ArduinoJson library
- Ignora campos desconocidos
- Valores inválidos → usa defaults

**Pin validation:**
- GPIO válidos: 0-39 (algunos reservados)
- ADC válidos: 32-39 (capacitive)
- Conflictos no detectados (responsabilidad usuario)

**Límites:**
- `espnow_channel`: 1-13
- `rs485_baud`: Cualquier int (no validado)
- Sensor types: "scd30", "bme280", "capacitive", "onewire"

---

## Troubleshooting

### Config no se guarda
**Causa:** SPIFFS error
**Fix:** Ver serial logs, intentar factory reset

### Cambios no tienen efecto
**Causa:** Require restart
**Fix:** POST a `/restart` después de config change

### WiFi fallback no funciona
**Causa:** Timeout muy corto
**Check:** Validation timeout es 30s
**Fix:** Esperar timeout completo antes de reconectar

### Sensor no se habilita
**Causa:** Require restart
**Fix:** Guardar config, luego restart

### ESP-NOW no detecta role
**Causa:** Grafana ping URL incorrecta
**Fix:** Verificar URL accessible desde ESP32
**Test:** `curl {grafana_ping_url}` desde red local
