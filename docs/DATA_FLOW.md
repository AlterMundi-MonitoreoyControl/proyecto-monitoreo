# Data Flow

Flujo de datos desde sensores hasta visualización en Grafana.

## Overview

```
┌─────────┐      ┌────────┐      ┌─────────┐      ┌─────────┐
│ Sensors │ ───> │ ESP32  │ ───> │ Network │ ───> │ Grafana │
└─────────┘      └────────┘      └─────────┘      └─────────┘
                     │
                     ├────> RS485 (optional)
                     │
                     └────> ESP-NOW mesh (optional)
```

## Local Sensor Flow

Sensor conectado directamente al ESP32.

### 1. Lectura de Sensores

**Frecuencia:** Cada 10s (hardcoded en main loop)

**Código:**
```cpp
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSendTime >= 10000) {
    #ifdef SENSOR_MULTI
      sensorMgr.readAll();  // Multi-sensor
    #else
      if (sensor->dataReady()) {
        sensor->read();  // Single sensor
      }
    #endif
    lastSendTime = currentMillis;
  }
}
```

**Proceso (multi-sensor):**
```
1. sensorMgr.readAll()
2. For each enabled sensor:
   - sensor->dataReady()  // Check if ready
   - sensor->read()       // Read values
   - sendDataGrafana()    // Transmit
   - sendDataRS485()      // If enabled
   - sendDataESPNOW()     // If sensor mode
```

### 2. Creación de Mensaje Grafana

**Función:** `create_grafana_message()` en `src/createGrafanaMessage.cpp`

**Input:**
```cpp
float temperature;
float humidity;
float co2;
const char* sensorType;  // Device name
```

**Output (InfluxDB Line Protocol):**
```
medicionesCO2,device={sensorType} temp={temp},hum={hum},co2={co2} {timestamp}
```

**Ejemplo:**
```
medicionesCO2,device=moni-80F3DAAD temp=25.30,hum=60.50,co2=450 1700000000000000000
```

**Timestamp:**
- Unix epoch en nanoseconds
- Calculado: `millis() * 1000000`
- No usa NTP time (solo para drift reference)

### 3. Transmisión HTTP

**Función:** `sendDataGrafana()` en `src/sendDataGrafana.cpp`

**HTTP Request:**
```
POST {URL from constants_private.h}
Authorization: Basic {TOKEN_GRAFANA}
Content-Type: text/plain

medicionesCO2,device=moni-80F3DAAD temp=25.30,hum=60.50,co2=450 1700000000000000000
```

**Detalles:**
- Client: `HTTPClient`
- Timeout: 5000ms (5s)
- Expected response: 204 No Content
- Error logging si != 204

**Código:**
```cpp
void sendDataGrafana(float temp, float hum, float co2, const char* sensorType) {
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Authorization", TOKEN_GRAFANA);
  http.addHeader("Content-Type", "text/plain");

  String data = create_grafana_message(temp, hum, co2, sensorType);

  int httpResponseCode = http.POST(data);

  if (httpResponseCode == 204) {
    Serial.println("✓ Datos enviados correctamente");
  } else {
    Serial.printf("✗ Error en el envío: %d\n", httpResponseCode);
  }

  http.end();
}
```

**Limitaciones:**
- Llamada bloqueante (pausa main loop ~100-500ms)
- Sin retry logic (pérdida de datos si falla)
- Sin buffering offline
- Sin rate limiting

---

## RS485 Flow

Salida serial paralela a Grafana.

### 1. Formato de Mensaje

**Código:** `sendDataRS485()` en código (inline)

**Output:**
```
{SensorType} - Temp: {temp}°C Humedad: {hum}% CO2: {co2}ppm\r\n
```

**Ejemplo:**
```
SCD30 - Temp: 25.3°C Humedad: 60.5% CO2: 450ppm\r\n
```

### 2. Transmisión Serial

**Config:**
- Baud: `rs485_baud` (default 9600)
- Format: 8N1 (hardcoded)
- DE/RE control: Auto-detect puenteado mode

**Código (puenteado mode):**
```cpp
void sendDataRS485(float temp, float hum, float co2, const char* type) {
  String message = String(type) + " - Temp: " + String(temp, 1) +
                   "°C Humedad: " + String(hum, 1) +
                   "% CO2: " + String(co2, 0) + "ppm\r\n";

  Serial2.println(message);
  Serial2.flush();

  Serial.printf("[RS485 TX] %s\n", message.c_str());
}
```

**Timing:**
- Mismo intervalo que Grafana (cada 10s)
- Flush asegura transmisión completa
- Sin ACK (one-way broadcast)

---

## ESP-NOW Mesh Flow

Transmisión desde sensores remotos a gateway via ESP-NOW.

### Sensor Mode Flow

```
1. Boot → Auto-detect role
   - WiFi not connected → Sensor mode
   - Or espnow_force_mode = "sensor"

2. Listen for gateway beacons
   - Broadcast on configured channel
   - Receive MSG_BEACON

3. Select best gateway
   - Compare RSSI (if multiple gateways)
   - 10dBm hysteresis

4. Pair with gateway
   - Send MSG_PAIR_REQUEST
   - Wait for MSG_PAIR_ACK

5. Read sensors (every send_interval_ms)
   - Same as local: sensor->read()

6. Send via ESP-NOW
   - Create MSG_DATA packet
   - Unicast to gateway MAC
   - Sequence number++
```

**MSG_DATA packet:**
```c
{
  msgType: 3,
  sensorId: sensor_MAC_LSB,
  temperature: 25.3,
  humidity: 60.5,
  co2: 450.0,
  sequence: 123
}
```

### Gateway Mode Flow

```
1. Boot → Auto-detect role
   - WiFi connected + Grafana ping OK → Gateway mode

2. Broadcast beacons
   - Every beacon_interval_ms (default 2s)
   - Advertise channel, MAC, timestamp

3. On MSG_PAIR_REQUEST
   - Add sensor to peer list
   - Send MSG_PAIR_ACK

4. On MSG_DATA (WiFi interrupt context)
   - Buffer data in ring buffer
   - DO NOT call HTTP (crash)
   - Return from interrupt ASAP

5. Main loop processing
   - Check buffer (meshBufferTail != meshBufferHead)
   - For each buffered entry:
     - Extract sensor data
     - Create Grafana message
     - HTTP POST (safe in main loop)
     - Mark buffer entry processed
```

**Ring buffer:**
```c
struct MeshDataBuffer {
  uint8_t senderMAC[6];
  float temp;
  float hum;
  float co2;
  uint32_t seq;
  bool valid;
};

MeshDataBuffer meshBuffer[10];  // Size 10
```

**Gateway forwarding:**
```cpp
void loop() {
  // ... other code ...

  while (meshBufferTail != meshBufferHead) {
    MeshDataBuffer* data = &meshBuffer[meshBufferTail];

    if (data->valid) {
      char sensorId[32];
      snprintf(sensorId, sizeof(sensorId), "mesh_%02X%02X%02X",
               data->senderMAC[3], data->senderMAC[4], data->senderMAC[5]);

      sendDataGrafana(data->temp, data->hum, data->co2, sensorId);

      data->valid = false;
    }

    meshBufferTail = (meshBufferTail + 1) % 10;
  }
}
```

**Device naming:**
- Local sensor: `moni-{full_MAC}`
- Mesh sensor: `mesh_{last3bytes_MAC}`

**Example:**
```
Local:  moni-80F3DAAD4512
Mesh:   mesh_4AA1B2
```

---

## Complete Multi-Hop Example

Sensor remoto → Gateway → Grafana

```
Time  Event
────  ─────────────────────────────────────────────────────────

0s    Sensor boots
      - Detect role: sensor (no WiFi)
      - Force channel 6
      - Listen for beacons

2s    Gateway broadcasts beacon
      - Channel 6, MAC AA:BB:CC:DD:EE:FF

2.1s  Sensor receives beacon
      - RSSI: -45dBm
      - Send pair request

2.2s  Gateway receives pair request
      - Add sensor to peer list
      - Send pair ACK

2.3s  Sensor receives ACK
      - Mark as paired
      - Start data transmission

30s   Sensor reads SCD30
      - temp=25.3, hum=60.5, co2=450

30.1s Sensor sends MSG_DATA via ESP-NOW
      - Unicast to gateway
      - Sequence #1

30.1s Gateway receives MSG_DATA (interrupt)
      - Buffer data (meshBuffer[0])
      - Return from interrupt

30.2s Gateway main loop
      - Process buffer
      - Extract data
      - Create message: "mesh_112233 temp=25.3,..."
      - HTTP POST to Grafana

30.3s Grafana receives data
      - Parse InfluxDB line protocol
      - Store in database
      - Available for querying

60s   Repeat (sensor sends again)
```

**Latency breakdown:**
- Sensor read: ~100ms
- ESP-NOW transmission: <10ms
- Gateway buffer processing: <50ms
- HTTP POST: 50-500ms (network-dependent)
- **Total:** ~200-700ms end-to-end

---

## Data Formats Summary

### InfluxDB Line Protocol

**General format:**
```
<measurement>,<tag_set> <field_set> <timestamp>
```

**Our format:**
```
medicionesCO2,device={device_id} temp={temp},hum={hum},co2={co2} {timestamp_ns}
```

**Example:**
```
medicionesCO2,device=moni-80F3DAAD temp=25.30,hum=60.50,co2=450.00 1700000000000000000
```

**Tags:**
- `device`: Device identifier (moni-{MAC} o mesh-{MAC3bytes})

**Fields:**
- `temp`: Temperature (float, °C)
- `hum`: Humidity (float, %)
- `co2`: CO2 (float, ppm)

**Timestamp:**
- Unix epoch nanoseconds
- Calculated from `millis()`

### RS485 Format

Human-readable ASCII.

**Format:**
```
{SensorType} - Temp: {temp}°C Humedad: {hum}% CO2: {co2}ppm\r\n
```

**Example:**
```
SCD30 - Temp: 25.3°C Humedad: 60.5% CO2: 450ppm\r\n
```

**Encoding:** ASCII
**Line ending:** CRLF (`\r\n`)
**Baud:** Configurable (default 9600)

### ESP-NOW Format

Binary protocol.

**MSG_DATA structure:**
```c
struct {
  uint8_t msgType;      // 3
  uint8_t sensorId;     // MAC LSB
  float temperature;    // IEEE 754
  float humidity;       // IEEE 754
  float co2;            // IEEE 754
  uint32_t sequence;    // Counter
} __attribute__((packed));
```

**Size:** 22 bytes
**Endianness:** Little-endian (ESP32 native)

---

## Grafana Integration

### InfluxDB Database

**Database:** `cto` (o configurado)
**Measurement:** `medicionesCO2`
**Retention:** Default (InfluxDB config)

### Query Examples

**Latest reading per device:**
```sql
SELECT last("temp"), last("hum"), last("co2")
FROM "medicionesCO2"
WHERE time > now() - 1h
GROUP BY "device"
```

**Temperature over time:**
```sql
SELECT mean("temp")
FROM "medicionesCO2"
WHERE time > now() - 24h
GROUP BY time(5m), "device"
fill(linear)
```

**All mesh sensors:**
```sql
SELECT *
FROM "medicionesCO2"
WHERE "device" =~ /^mesh_/
AND time > now() - 1h
```

### Grafana Dashboard

**Variables:**
- `$device`: Dropdown de devices activos
- `$sensor`: Filter por tipo (SCD30, mesh_, etc)

**Panels típicos:**
- Time series: Temperatura por device
- Gauge: CO2 actual con thresholds
- Table: Latest values por device
- Stat: Device count

Ver `DASHBOARD_GRAFANA.md` para config completa.

---

## Error Handling

### Sensor Read Failure

```cpp
if (!sensor->read()) {
  Serial.println("Error reading sensor");
  // Skip transmission
  // Try again next cycle (10s)
  return;
}
```

**Behavior:** Datos no enviados, retry automático en próximo ciclo

### HTTP POST Failure

```cpp
if (httpResponseCode != 204) {
  Serial.printf("✗ Error: %d\n", httpResponseCode);
  Serial.println(http.getString());
  // Data lost, no retry
}
```

**Behavior:** Log error, pérdida de datos

**Common errors:**
- Timeout (5s): Network slow o Grafana down
- 404: URL incorrecta en constants_private.h
- 401: TOKEN_GRAFANA inválido
- 500: Grafana internal error

### ESP-NOW Transmission Failure

```cpp
esp_err_t result = esp_now_send(gatewayMAC, data, sizeof(data));

if (result != ESP_OK) {
  Serial.println("ESP-NOW send failed");
  // Mark as unpaired, retry discovery
}
```

**Behavior:** Re-discovery, re-pairing automático

### Buffer Overflow (Gateway)

```cpp
int nextHead = (meshBufferHead + 1) % 10;

if (nextHead == meshBufferTail) {
  Serial.println("[MESH] ✗ Buffer full, dropping data");
  return;  // Data lost
}
```

**Behavior:** Datos más viejos se pierden (FIFO)

**Mitigation:**
- Aumentar buffer size (modificar código)
- Reducir send_interval_ms de sensores
- Optimizar HTTP POST speed

---

## Performance Metrics

### Typical Scenario

**Config:**
- 1 local SCD30
- 3 mesh sensors (SCD30)
- 10s local read interval
- 30s mesh send interval
- Grafana on LAN

**Throughput:**
- Local: 1 reading/10s = 0.1 Hz
- Mesh: 3 readings/30s = 0.1 Hz
- Total: ~0.2 Hz to Grafana

**Latency:**
- Local: <500ms (sensor read + HTTP)
- Mesh: <1s (ESP-NOW + buffer + HTTP)

**Data rate:**
- InfluxDB message: ~120 bytes
- Total: 0.2 Hz × 120 bytes = 24 bytes/s
- Daily: ~2 MB

### Resource Usage

**RAM (local only):**
- Base: ~30KB
- Sensors: ~10KB
- WiFi: ~5KB
- **Total:** ~45-50KB (15%)

**RAM (gateway + mesh):**
- Base: ~45KB
- ESP-NOW: ~2KB
- Mesh buffer: ~500 bytes
- **Total:** ~47-52KB (16%)

**Flash:**
- Firmware: ~1.1MB
- SPIFFS: ~1KB (config.json)
- **Total:** 1.1MB (85%)

**CPU:**
- Sensor read: <1% (10s interval)
- HTTP POST: ~5% durante transmisión
- ESP-NOW: <1%
- **Average:** <5%

---

## Optimization Opportunities

### Reduce Latency

1. **Increase read frequency** (cambiar hardcoded 10s)
2. **Async HTTP** (usar tasks/RTOS)
3. **Reduce HTTP timeout** (5s → 2s)

### Reduce Data Loss

1. **Add retry logic** para HTTP failures
2. **Offline buffering** (SPIFFS queue)
3. **Increase mesh buffer** size
4. **ACK mechanism** en ESP-NOW

### Reduce Power

1. **Deep sleep** entre lecturas
2. **Sensor power down** cuando inactivo
3. **WiFi modem sleep**
4. **ESP-NOW power save mode**

**Nota:** Optimizaciones no implementadas actualmente

---

## Monitoring

### Serial Logs

**Debug output (115200 baud):**
```
[ESP-NOW] Gateway mode
[ESP-NOW] Channel set to 6
SCD30 - Temp: 25.3°C, Hum: 60.5%, CO2: 450ppm
Enviando a Grafana:
medicionesCO2,device=moni-80F3DAAD temp=25.30,hum=60.50,co2=450.00 1700000000000000000
✓ Datos enviados correctamente
[MESH→GRAFANA] mesh_112233: T=25.1 H=60.5 CO2=450 (seq=123)
```

### API Endpoints

**Current readings:**
```bash
curl http://ESP32_IP/actual
```

**ESP-NOW status:**
```bash
curl http://ESP32_IP/espnow/status
```

### Grafana Queries

**Check data arrival:**
```sql
SELECT count(*)
FROM "medicionesCO2"
WHERE time > now() - 5m
GROUP BY "device"
```

**Expected:** 30 readings/device (5min / 10s interval)

---

## Troubleshooting Data Flow

### No data in Grafana

**Check:**
1. Serial logs: "✓ Datos enviados correctamente"?
2. If no: Check URL, TOKEN_GRAFANA, WiFi
3. If yes: Check InfluxDB directly
4. Query: `SELECT * FROM medicionesCO2 LIMIT 10`

**InfluxDB test:**
```bash
curl "http://GRAFANA_URL:8086/query?db=cto&q=SELECT * FROM medicionesCO2 LIMIT 10"
```

### Mesh data not forwarded

**Check:**
1. Gateway in gateway mode? (`/espnow/status`)
2. Sensor paired? (sensor logs)
3. Buffer full warnings? (gateway logs)
4. Channel match? (config.json)

### RS485 no output

**Check:**
1. `rs485_enabled: true` en config?
2. Restart after enable?
3. Baud rate match receiver?
4. Wiring: TX pin connected?

### HTTP timeouts

**Causes:**
- Network congestion
- Grafana slow/down
- DNS resolution delay

**Fix:**
- Check network ping ESP32→Grafana
- Reduce timeout (modify code)
- Static IP (skip DNS)
