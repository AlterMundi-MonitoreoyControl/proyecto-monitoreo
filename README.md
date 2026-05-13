# proyecto-monitoreo

Sistema de monitoreo multi-sensor para ESP32 con transmisión a Grafana/InfluxDB.

**Versión:** 0.1.10
**Plataforma:** ESP32 (espressif32)

## Funcionalidad

- Lectura de sensores ambientales (CO2, temperatura, humedad)
- Transmisión HTTP a InfluxDB (formato line protocol)
- Salida serial RS485 (opcional)
- Mesh ESP-NOW para sensores remotos sin WiFi
- Interfaz web de configuración
- OTA updates desde GitHub releases

## Sensores Soportados

| Sensor | Protocolo | Mide | Pines |
|--------|-----------|------|-------|
| SCD30 | I2C | CO2, temp, humedad | SDA=21, SCL=22 |
| BME280 | I2C | Temp, humedad, presión* | SDA=21, SCL=22 |
| Capacitive | ADC | Humedad de suelo | GPIO34 |
| DS18B20 | OneWire | Temperatura | GPIO4 |

*Presión leída pero no transmitida a Grafana

## Instalación

### Hardware

**ESP32 básico:**
```
3.3V → sdc30,capacitivo,resistivo I2C (VCC)
GND  → sdc30 I2C (GND)
21   → sdc30 SDA (I2C) 
22   → sdc30 SCL (I2C) 
34   → Sensor capacitivo (AOUT)
4    → OneWire data (requiere pull-up 4.7kΩ a 3.3V)
```

**RS485 (opcional):**
```
17 → DI (MAX485)
16 → RO (MAX485)
18 → DE//RE
```

**ESP-NOW mesh:** Sólo requiere ESP32, sin hardware adicional.

### Firmware

```bash
# Clonar repo
git clone https://github.com/Pablomonte/proyecto-monitoreo
cd proyecto-monitoreo

# Copiar template de credenciales
cp include/constants_private.h.example include/constants_private.h

# Editar constants_private.h con:
# - URL de InfluxDB
# - Token de autenticación
# - Repo de GitHub para OTA

# Compilar y flashear
pio run -e esp32dev --target upload

# Entornos disponibles:
# esp32dev           - SCD30 solo
# esp32dev_multi     - Todos los sensores + RS485
# esp32dev_espnow    - Multi-sensor + ESP-NOW mesh
```

### Configuración inicial

1. ESP32 crea AP `ESP32-{MAC}`
2. Conectar a ese AP
3. Navegar a http://192.168.16.10/settings
4. Configurar WiFi y sensores
5. Guardar y reiniciar

## Uso

### Interfaz Web

Acceder a `http://{IP_DEL_ESP32}/settings`

**Endpoints disponibles:**
- `/actual` - Lecturas actuales (JSON)
- `/data` - Vista HTML de datos
- `/config` - GET/POST configuración (JSON)
- `/settings` - Formulario web de configuración
- `/restart` - POST para reiniciar ESP32
- `/espnow/status` - Estado ESP-NOW (JSON)

### Configuración (config.json)

El sistema usa `/config.json` en SPIFFS. Editable vía web o API REST.

**Parámetros principales:**
```json
{
  "ssid": "WiFi_SSID",
  "passwd": "WiFi_Password",
  "incubator_name": "moni-AABBCCDDEE",
  "rs485_enabled": false,
  "rs485_rx": 16,
  "rs485_tx": 17,
  "rs485_baud": 9600,
  "espnow_enabled": false,
  "espnow_channel": 1,
  "send_interval_ms": 30000,
  "sensors": [
    {
      "type": "scd30",
      "enabled": true,
      "config": {}
    }
  ]
}
```

Ver [docs/CONFIGURATION.md](docs/CONFIGURATION.md) para detalles completos.

### Transmisión de Datos

**InfluxDB (cada 10s):**
```
POST {URL from constants_private.h}
Authorization: Basic {TOKEN_GRAFANA}
Content-Type: text/plain

medicionesCO2,device=moni-AABBCCDDEE temp=25.3,hum=60.5,co2=450 1700000000000000000
```

**RS485 (cada 10s, si habilitado):**
```
SCD30 - Temp: 25.3°C Humedad: 60.5% CO2: 450ppm\r\n
```

**ESP-NOW (configurable, default 30s):**
- Sensores remotos transmiten a gateway vía ESP-NOW
- Gateway reenvía a Grafana via HTTP
- Ver [docs/ESPNOW.md](docs/ESPNOW.md) para arquitectura

## ESP-NOW Mesh

Sistema de mesh para sensores sin conectividad WiFi a internet.

**Roles:**
- **Gateway:** Conectado a WiFi, recibe datos de sensores, envía a Grafana
- **Sensor:** Sin WiFi, transmite a gateway vía ESP-NOW

**Auto-detección de rol:**
1. Si WiFi conectado → test Grafana ping
2. Grafana accesible → modo gateway
3. Sin Grafana → modo sensor

**Limitaciones:**
- Alcance: ~100m línea de vista, ~30m interior
- Max peers por gateway: 20
- Buffer de 10 mensajes en gateway
- Sensores y gateway deben estar en mismo canal WiFi

Ver [docs/ESPNOW.md](docs/ESPNOW.md) para detalles del protocolo.

## Limitaciones Conocidas

### Hardware
- I2C: Max 2 sensores por bus (SDA/SCL compartidos)
- OneWire: Max ~10 sensores DS18B20 en cadena (limitado por RAM)
- Capacitive: Solo 1 sensor (pin ADC específico)

### Software
- Sin buffering de datos (pérdida si sin conexión)
- Llamadas HTTP bloqueantes (main loop pausado durante envío)
- ESP-NOW channel debe coincidir entre gateway y sensores
- OTA sin rollback automático
- Config sin versionado (migraciones best-effort)

### Performance
- Intervalo lectura: 10s (hardcoded)
- RAM usada: ~49KB (15%)
- Flash: ~1.1MB (85%)

## Troubleshooting

### Sensor no detectado
```
# Ver logs seriales (115200 baud)
pio device monitor

# Buscar:
"No se pudo inicializar el sensor {TIPO}!"
```
→ Verificar wiring, voltaje 3.3V, pull-ups si aplica

### Sin datos en Grafana
1. Check IP y conectividad: ping ESP32
2. Test endpoint: `curl http://{IP}/actual`
3. Verificar URL en `constants_private.h`
4. Ver logs serial para errores HTTP

### ESP-NOW no parece
1. Verificar mismo canal WiFi (config.json)
2. Gateway debe estar en modo gateway (ver `/espnow/status`)
3. Sensor debe detectar beacons (ver logs)
4. Alcance: mover dispositivos más cerca

### WiFi no conecta
- Borrar credenciales: POST a `/config/reset`
- Reconectar a AP del ESP32
- Re-configurar WiFi en `/settings`

## Desarrollo

### Agregar sensor nuevo

1. Crear `include/sensors/MySensor.h` implementando `ISensor`
2. Agregar a `SensorManager::loadFromConfig()` switch
3. Update `platformio.ini` con dependencias
4. Documentar en [docs/SENSORS.md](docs/SENSORS.md)

### Testing local

```bash
# Build
pio run -e esp32dev_multi

# Upload
pio run -e esp32dev_multi --target upload

# Monitor serial
pio device monitor -b 115200

# Test RS485 output (Linux)
screen /dev/ttyUSB0 9600
```

## Documentación Adicional

- [API Reference](docs/API.md) - Endpoints HTTP detallados
- [ESP-NOW Protocol](docs/ESPNOW.md) - Mesh networking
- [Configuration](docs/CONFIGURATION.md) - config.json completo
- [Sensors](docs/SENSORS.md) - Detalles de cada sensor
- [Data Flow](docs/DATA_FLOW.md) - Flujo desde sensor a Grafana

## Licencia

Same as upstream [AlterMundi-MonitoreoyControl/proyecto-monitoreo](https://github.com/AlterMundi-MonitoreoyControl/proyecto-monitoreo)
