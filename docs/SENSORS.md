# Sensors Reference

Detalles de implementación de cada sensor soportado.

## Interfaz ISensor

Todos los sensores implementan `include/sensors/ISensor.h`:

```cpp
class ISensor {
public:
  virtual bool init() = 0;
  virtual bool dataReady() = 0;
  virtual bool read() = 0;
  virtual float getTemperature() = 0;
  virtual float getHumidity() = 0;
  virtual float getCO2() = 0;
  virtual const char* getSensorType() = 0;
  virtual bool calibrate(float reference) = 0;
  virtual bool isActive() = 0;
};
```

**Valores de error:** `-1` si métrica no soportada

---

## SCD30 - CO2/Temperature/Humidity

### Hardware

**Sensor:** Sensirion SCD30
**Protocolo:** I2C
**Address:** Default I2C (0x61)
**Pines:**
```
ESP32  →  SCD30
21     →  SDA
22     →  SCL
3.3V   →  VCC
GND    →  GND
```

**Power:** 3.3V @ 19mA (avg), 75mA (peak)

### Especificaciones

| Métrica | Rango | Precisión | Resolución |
|---------|-------|-----------|------------|
| CO2 | 0-40000 ppm | ±(30ppm + 3%) | 1 ppm |
| Temperatura | -40 a +70°C | ±0.5°C | 0.01°C |
| Humedad | 0-100% RH | ±3% RH | 0.01% |

**Interval de medición:** 2s (configurable)
**Warm-up time:** ~2s

### Implementación

**Archivo:** `include/sensors/SCD30Sensor.h`
**Librería:** Adafruit_SCD30

**Inicialización:**
```cpp
bool init() {
  if (!scd30.begin()) return false;
  scd30.setMeasurementInterval(2); // 2s
  return true;
}
```

**Lectura:**
```cpp
bool dataReady() {
  return scd30.dataReady();
}

bool read() {
  if (!scd30.read()) return false;
  temperature = scd30.temperature;
  humidity = scd30.relative_humidity;
  co2 = scd30.CO2;
  return true;
}
```

**Calibración:**
```cpp
bool calibrate(float reference) {
  return scd30.setForcedRecalibrationFactor(reference);
}
```

- Endpoint: `GET /calibrate-scd30`
- Reference: 400ppm (aire exterior típico)
- Requiere ambiente estable

### Troubleshooting

**"No se pudo inicializar el sensor SCD30!"**
- Check I2C wiring (SDA/SCL)
- Verify 3.3V power (no 5V)
- I2C pull-ups (generalmente internos OK)
- Test: `i2cdetect` en otros devices

**Lecturas erráticas:**
- Warm-up: esperar 30s después de power-on
- Calibración: ejecutar `/calibrate-scd30` en exterior
- Interferencia: alejar de fuentes EMI

**CO2 siempre 0 o 999999:**
- Sensor no inicializado (ver logs)
- Hardware failure
- Check connections

---

## BME280 - Temperature/Humidity/Pressure

### Hardware

**Sensor:** Bosch BME280
**Protocolo:** I2C
**Address:** 0x76 o 0x77 (auto-detect)
**Pines:**
```
ESP32  →  BME280
21     →  SDA
22     →  SCL
3.3V   →  VCC (3.3V)
GND    →  GND
```

**Power:** 3.3V @ 2.8μA-3.6mA

### Especificaciones

| Métrica | Rango | Precisión | Resolución |
|---------|-------|-----------|------------|
| Temperatura | -40 a +85°C | ±1.0°C | 0.01°C |
| Humedad | 0-100% RH | ±3% RH | 0.008% |
| Presión | 300-1100 hPa | ±1 hPa | 0.18 Pa |

**Response time:** <1s (temp), <1s (humidity), <1s (pressure)

### Implementación

**Archivo:** `include/sensors/BME280Sensor.h`
**Librería:** Adafruit_BME280, Adafruit_Sensor

**Inicialización:**
```cpp
bool init() {
  // Try both addresses
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    return false;
  }

  // Suggested settings
  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1,  // temp
    Adafruit_BME280::SAMPLING_X1,  // pressure
    Adafruit_BME280::SAMPLING_X1,  // humidity
    Adafruit_BME280::FILTER_OFF
  );
  return true;
}
```

**Lectura:**
```cpp
bool read() {
  bme.takeForcedMeasurement();
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0; // Pa → hPa

  // Check for NaN (read error)
  if (isnan(temperature) || isnan(humidity)) {
    return false;
  }
  return true;
}
```

**Métricas:**
- `getTemperature()`: °C
- `getHumidity()`: % RH
- `getCO2()`: -1 (no soportado)

**Nota:** Presión leída pero no transmitida a Grafana (código actual)

### Troubleshooting

**"No se pudo inicializar el sensor BME280!"**
- Check address (0x76 vs 0x77)
  - Some modules: SDO pin controls address
  - SDO→GND: 0x76, SDO→VCC: 0x77
- Verify I2C bus (shared con SCD30)
- Check 3.3V (no 5V, daña sensor)

**Lecturas NaN:**
- Forced measurement timeout
- I2C bus error
- Check serial logs para error details

**Humedad siempre 100%:**
- Condensación en sensor
- Fake BME280 (common en clones baratos)
- Verify BME280 vs BMP280 (sin humidity)

---

## Capacitive Soil Moisture

### Hardware

**Sensor:** Capacitive Soil Moisture Sensor v1.2/v2.0
**Protocolo:** Analog ADC
**Pin:** GPIO34 (default, configurable)
**Pines:**
```
ESP32  →  Sensor
34     →  AOUT (analog out)
3.3V   →  VCC
GND    →  GND
```

**Power:** 3.3V @ 5mA
**Output:** 0-3.3V analog (3.3V=dry, 0V=wet)

### Especificaciones

| Métrica | Rango | Resolución |
|---------|-------|------------|
| Humedad soil | 0-100% | ~0.02% (12-bit ADC) |

**Response time:** <1s
**Accuracy:** Depende de calibración en medio específico

### Implementación

**Archivo:** `include/sensors/CapacitiveSensor.h`
**Librería:** Ninguna (ADC nativo ESP32)

**Inicialización:**
```cpp
bool init() {
  pinMode(pin, INPUT);
  analogReadResolution(12); // 0-4095
  return true;
}
```

**Lectura:**
```cpp
bool read() {
  int raw = analogRead(pin);
  // Invert: 4095=dry (0%), 0=wet (100%)
  humidity = 100.0 - (raw / 4095.0 * 100.0);

  // Constrain 0-100
  humidity = constrain(humidity, 0, 100);
  return true;
}
```

**Métricas:**
- `getTemperature()`: -1
- `getHumidity()`: 0-100%
- `getCO2()`: -1

**Calibración (no automatizada):**
- Dry: Sensor en aire → nota valor ADC
- Wet: Sensor en agua → nota valor ADC
- Ajustar mapeo en código si needed

### Config

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

**name:** Identificador del sensor (usado en logs, futuro multi-capacitive)

### Troubleshooting

**Lecturas estables en 0% o 100%:**
- Check connections (AOUT debe estar conectado)
- Verify 3.3V power
- Sensor defectuoso

**Lecturas erráticas:**
- Noise en ADC: agregar capacitor 0.1μF entre AOUT y GND
- Cable largo: max 30cm recomendado
- Interferencia WiFi: inherente, averaging puede ayudar

**Humedad no sigue realidad:**
- Sensor requiere calibración por tipo de suelo
- Corrosión: versiones v1.2 sufren corrosión en months
- Usar v2.0 (coated) para durabilidad

---

## OneWire (DS18B20) - Temperature

### Hardware

**Sensor:** Dallas DS18B20
**Protocolo:** 1-Wire
**Pin:** GPIO4 (default, configurable)
**Pines:**
```
ESP32  →  DS18B20(s)
4      →  DQ (data)
3.3V   →  VDD
GND    →  GND

Pull-up: 4.7kΩ entre DQ y VDD
```

**Power:** 3.3V @ 1.5mA (active), 1μA (idle)
**Topology:** Múltiples sensores en paralelo (bus compartido)

### Especificaciones

| Métrica | Rango | Precisión | Resolución |
|---------|-------|-----------|------------|
| Temperatura | -55 a +125°C | ±0.5°C (-10 a +85°C) | 0.0625°C (12-bit) |

**Conversion time:** 750ms (12-bit default)
**Max devices per bus:** ~10-20 (limitado por RAM/timing)

### Implementación

**Archivo:** `include/sensors/OneWireSensor.h`
**Librería:** DallasTemperature, OneWire

**Inicialización (scan mode):**
```cpp
bool init() {
  oneWire = new OneWire(pin);
  sensors = new DallasTemperature(oneWire);

  sensors->begin();
  deviceCount = sensors->getDeviceCount();

  if (deviceCount == 0) return false;

  // Set resolution to 12-bit
  sensors->setResolution(12);
  return true;
}
```

**Lectura:**
```cpp
bool read() {
  // Request temperatures (async)
  sensors->requestTemperatures();
  delay(100); // Wait for conversion

  // Read all devices
  for (int i = 0; i < deviceCount; i++) {
    float temp = sensors->getTempCByIndex(i);

    // 85.0 = default value (not ready)
    if (temp == 85.0) continue;

    // Send individual reading to Grafana
    sendToGrafana(temp, i);
  }
  return true;
}
```

**Identificación de sensores:**
- ROM address (64-bit unique ID)
- Usado para identificar sensores individuales
- Grafana tag: `device=onewire_{ROM_suffix}`

### Config

```json
{
  "type": "onewire",
  "enabled": true,
  "config": {
    "pin": 4,
    "scan": true
  }
}
```

**scan:** true = auto-detect todos los sensores en bus

### Múltiples Sensores

**Wiring en paralelo:**
```
ESP32 GPIO4 ──┬─── DQ (DS18B20 #1)
               ├─── DQ (DS18B20 #2)
               └─── DQ (DS18B20 #3)

Pull-up 4.7kΩ entre GPIO4 y 3.3V
```

**Data transmission:**
- Cada sensor envía lectura separada a Grafana
- Tag `device` diferente por sensor
- Sequence basada en orden de detección (no determinista)

### Troubleshooting

**"OneWire: 0 sensors detected"**
- **Pull-up missing:** Crítico, 4.7kΩ DQ↔VDD
- Wrong pin en config
- Power: check 3.3V
- Wiring: máximo 30cm (sin pull-up fuerte)

**Lecturas 85.0°C constantes:**
- Conversion time insuficiente (aumentar delay)
- Bus timing issue (cable muy largo)
- Sensor defectuoso

**Solo detecta 1 sensor (múltiples connected):**
- Short circuit: check que DQ no toca VDD/GND
- Sensor defectuoso en cadena (desconectar 1 por 1)
- Parasite power mode (no implementado, requiere wiring diferente)

**Lecturas erráticas:**
- Interference en bus (alejar de WiFi antenna)
- Cable length: max 3m sin repeater
- Capacitance: agregar 0.1μF VDD↔GND cerca de sensores

---

## ModbusTH (TH-MB-04S) - Temperature/Humidity

### Hardware

**Sensor:** TH-MB-04S (YF-TH-V18N-T21)
**Protocolo:** Modbus RTU sobre RS485
**Address:** Configurable (default 1)
**Pines:**
```
ESP32  →  RS485 Module  →  Sensor
17     →  TX/DI         →  D+/A (yellow/green)
16     →  RX/RO         →  D-/B (blue)
GPIO*  →  DE/RE         →  (bridge si auto-flow)
5V     →  VCC           →  Power+ (5-30V DC)
GND    →  GND           →  Power-
```

**RS485 Module:** MAX485 o similar
**Power sensor:** 5-30V DC @ <0.2W
**Baud rate:** 9600 (default), 4800/19200 configurables

### Especificaciones

| Métrica | Rango | Precisión | Resolución |
|---------|-------|-----------|------------|
| Temperatura | -40 a +80°C | ±0.5°C | 0.1°C |
| Humedad | 0-100% RH | ±3% RH | 0.1% |

**Response time:** ~1s
**Protocolo:** Modbus RTU, Function 03 (Read Holding Registers)

### Registros Modbus

| Registro | Contenido | Formato |
|----------|-----------|---------|
| 0 | Humedad | int16 × 10 (556 = 55.6%) |
| 1 | Temperatura | int16 × 10 (259 = 25.9°C) |
| 80 | Offset humedad | int16 × 10 |
| 81 | Offset temperatura | int16 × 10 |

### Implementación

**Archivo:** `include/sensors/ModbusTHSensor.h`
**Librería:** emelianov/modbus-esp8266

**Inicialización:**
```cpp
bool init() {
  serial->begin(baudrate, SERIAL_8N1, rxPin, txPin);
  mb.begin(serial);
  mb.master();

  if (dePin >= 0) {
    pinMode(dePin, OUTPUT);
    digitalWrite(dePin, LOW);  // Receive mode
  }

  return readRegisters();  // Test read
}
```

**Lectura:**
```cpp
bool read() {
  // Read 2 registers starting at address 0
  mb.readHreg(modbusAddress, 0, registerBuffer, 2, callback);

  // Wait for response with timeout
  while (!readComplete && (millis() - start < 1000)) {
    mb.task();
    delay(10);
  }

  humidity = registerBuffer[0] / 10.0;
  temperature = registerBuffer[1] / 10.0;
  return true;
}
```

### Config

```json
{
  "type": "modbus_th",
  "enabled": true,
  "config": {
    "address": 1,
    "rx_pin": 16,
    "tx_pin": 17,
    "de_pin": -1,
    "baudrate": 9600
  }
}
```

**address:** Dirección Modbus del sensor (1-254)
**rx_pin:** GPIO para RS485 RX (RO del MAX485)
**tx_pin:** GPIO para RS485 TX (DI del MAX485)
**de_pin:** GPIO para DE/RE control (-1 si puenteado para auto-flow)
**baudrate:** Velocidad de comunicación (default 9600)

### Troubleshooting

**"Sensor not responding"**
- Check wiring: D+/A → verde/amarillo, D-/B → azul
- Verify 5-30V power al sensor
- Check Modbus address (usar scanner si desconocido)
- Verify baud rate matches sensor config

**Lecturas siempre 0 o -1:**
- RS485 polaridad invertida (swap D+/D-)
- Timeout muy corto (aumentar en código)
- Bus termination: 120Ω en extremos si cable largo

**Comunicación intermitente:**
- Cable máximo 1200m (RS485 spec)
- Evitar cables paralelos a líneas de potencia
- Agregar resistencias de terminación 120Ω

**Múltiples sensores en bus:**
- Cada sensor debe tener dirección única
- Usar 120Ω terminación solo en extremos del bus
- Max ~32 dispositivos por bus

---

## HD38 - Soil Moisture

### Hardware

**Sensor:** HD-38 Soil Moisture / Rain Sensor
**Chip:** LM393 comparator
**Protocolo:** Analog (ADC) + Digital GPIO
**Pines:**
```
ESP32  →  Sensor
35     →  AOUT (analog)
GPIO*  →  DOUT (digital, optional)
5V     →  VCC
GND    →  GND
```

**Con divisor de voltaje (recomendado para analog):**
```
Sensor AOUT → [10kΩ] → ESP32 ADC
                    ↓
                 [10kΩ]
                    ↓
                  GND
```

**Power:** 3.3-5V DC @ 15mA
**Cable:** 5m con sondas gold-coated
**Output:** 0-5V analog, 0/5V digital

### Especificaciones

| Métrica | Rango | Resolución |
|---------|-------|------------|
| Humedad suelo | 0-100% | ~0.02% (12-bit ADC) |
| Digital state | Wet/Dry | 1 bit |

**Response time:** <1s
**Threshold digital:** Ajustable via potenciómetro en PCB

### Implementación

**Archivo:** `include/sensors/HD38Sensor.h`
**Librería:** Ninguna (ADC nativo ESP32)

**Inicialización:**
```cpp
bool init() {
  if (analogPin >= 0) {
    pinMode(analogPin, INPUT);
    analogReadResolution(12);  // 0-4095
  }
  if (digitalPin >= 0) {
    pinMode(digitalPin, INPUT);
  }
  return (analogPin >= 0 || digitalPin >= 0);
}
```

**Lectura:**
```cpp
bool read() {
  int rawValue = analogRead(analogPin);

  // Scale for voltage divider (5V→2.5V max)
  if (useVoltageDivider) {
    rawValue = constrain(rawValue, 0, 3100);
  }

  // Map to 0-100% (inverted: high ADC = dry)
  humidity = map(rawValue, dryValue, wetValue, 0, 100);
  humidity = constrain(humidity, 0, 100);

  // Digital threshold
  if (digitalPin >= 0) {
    digitalState = digitalRead(digitalPin);
    if (invertLogic) digitalState = !digitalState;
  }

  return true;
}
```

### Config

```json
{
  "type": "hd38",
  "enabled": true,
  "config": {
    "analog_pin": 35,
    "digital_pin": -1,
    "voltage_divider": true,
    "invert_logic": false,
    "name": "Suelo1"
  }
}
```

**analog_pin:** GPIO ADC para lectura analog (default 35)
**digital_pin:** GPIO para lectura digital (-1 = disabled)
**voltage_divider:** true si hay divisor 2:1 para 5V sensor
**invert_logic:** true si seco=HIGH húmedo=LOW
**name:** Identificador del sensor

### Calibración

El sensor requiere calibración manual por tipo de suelo:

1. **Medir en seco:** Sensor en aire → anotar valor ADC (`dryValue`)
2. **Medir en húmedo:** Sensor en agua → anotar valor ADC (`wetValue`)
3. **Configurar:**
```cpp
sensor->setCalibration(dryValue, wetValue);
```

**Valores típicos (con divisor):**
- Dry: ~3000-3100
- Wet: ~1000-1500

### Troubleshooting

**Lecturas siempre 0% o 100%:**
- Check conexión AOUT
- Verify 5V power (3.3V insuficiente para rango completo)
- Sin divisor: ADC satura a 4095

**Lecturas erráticas:**
- Agregar capacitor 0.1μF entre AOUT y GND
- Cable máximo recomendado: 5m (incluido)
- Interferencia WiFi: promediar múltiples lecturas

**Digital siempre HIGH o LOW:**
- Ajustar potenciómetro en PCB del sensor
- Verificar conexión DOUT
- Check `invert_logic` en config

**Corrosión de sondas:**
- Versión HD-38 tiene gold-coating
- No alimentar constantemente (usar solo al leer)
- En condiciones muy húmedas, verificar periódicamente

---

## Sensor Simulation (Debug)

### Hardware

Ninguno. Sensor virtual para testing.

### Implementación

**Archivo:** `include/sensors/SimulatedSensor.h`
**Librería:** Ninguna

**Lecturas:**
```cpp
bool read() {
  temperature = 25.0 + (random(-10, 10) / 10.0); // 24-26°C
  humidity = 60.0 + (random(-50, 50) / 10.0);    // 55-65%
  co2 = 450 + random(-50, 50);                   // 400-500ppm
  return true;
}
```

**Valores:**
- Temperatura: ~25°C ± 1°C (random)
- Humedad: ~60% ± 5%
- CO2: ~450ppm ± 50ppm

**Uso:**
- Build flag `-DMODO_SIMULACION`
- Testing sin hardware
- CI/CD builds

---

## Comparativa de Sensores

| Feature | SCD30 | BME280 | Capacitive | OneWire | ModbusTH | HD38 |
|---------|-------|--------|------------|---------|----------|------|
| Temperatura | ✓ | ✓ | ✗ | ✓ | ✓ | ✗ |
| Humedad | ✓ | ✓ | ✓ (soil) | ✗ | ✓ | ✓ (soil) |
| CO2 | ✓ | ✗ | ✗ | ✗ | ✗ | ✗ |
| Presión | ✗ | ✓* | ✗ | ✗ | ✗ | ✗ |
| Bus | I2C | I2C | ADC | 1-Wire | RS485 | ADC |
| Multi-sensor | No** | No** | No | Sí | Sí*** | No |
| Calibración | Sí | No | Manual | No | Registro | Manual |
| Power (mA) | 19 | 2.8 | 5 | 1.5 | <40 | 15 |
| Cable max | ~1m | ~1m | 30cm | 3m | 1200m | 5m |

*Leído pero no enviado a Grafana
**Limitado por I2C bus (max 2 por bus)
***Hasta 32 dispositivos por bus RS485 con direcciones únicas

---

## Agregar Sensor Nuevo

### 1. Crear Header

`include/sensors/MyNewSensor.h`:
```cpp
#include "ISensor.h"

class MyNewSensor : public ISensor {
private:
  float temperature;
  bool active;

public:
  bool init() override {
    // Hardware init
    return true;
  }

  bool dataReady() override { return true; }

  bool read() override {
    // Read sensor
    temperature = /* ... */;
    return true;
  }

  float getTemperature() override { return temperature; }
  float getHumidity() override { return -1; }
  float getCO2() override { return -1; }

  const char* getSensorType() override {
    return "mynewsensor";
  }

  bool calibrate(float ref) override { return false; }
  bool isActive() override { return active; }
};
```

### 2. Registrar en SensorManager

`include/SensorManager.h`, método `loadFromConfig()`:
```cpp
if (type == "mynewsensor") {
  ISensor* sensor = new MyNewSensor();
  if (sensor->init()) {
    sensors.push_back(sensor);
  }
}
```

### 3. Update platformio.ini

Agregar librería si needed:
```ini
lib_deps =
  existing/libs
  author/MyNewSensorLib @ ^1.0.0
```

### 4. Documentar

Agregar sección a este documento con:
- Hardware specs
- Wiring
- Config JSON
- Troubleshooting

---

## Best Practices

### Inicialización
- Init en `setup()`, no en `loop()`
- Check return value de `init()`
- Log failure detalladamente
- Mark sensor inactive si init falla

### Lecturas
- Check `dataReady()` antes de `read()`
- Return false si read falla (no partial data)
- Usar timeouts en sensor blocking
- NaN checks para floating point

### Error handling
- Graceful degradation (continuar con otros sensores)
- Log errores pero no crash
- Retry logic si aplicable
- User feedback vía API/web

### Power
- Considerar deep sleep para battery
- Power down sensores si no needed
- I2C pull-ups: 4.7kΩ típico

### Timing
- Async reads donde posible (OneWire)
- Avoid blocking main loop >100ms
- Coordinate multi-sensor reads
