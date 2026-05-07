# ESP-NOW Mesh Protocol

Todos los nodos deben configurarse en el canal del gateway 

Arquitectura de red mesh para sensores sin conexión WiFi a internet.

## Arquitectura

```
Sensor (sin WiFi) ─ESP-NOW→ Gateway (WiFi) ─HTTP→ Grafana
Sensor (sin WiFi) ─┘
Sensor (sin WiFi) ─┘
```

**Dos roles:**
1. **Gateway:** ESP32 conectado a WiFi con acceso a Grafana
2. **Sensor:** ESP32 sin WiFi, solo transmite vía ESP-NOW

## Auto-detección de Rol

Al boot, el sistema determina rol automáticamente:

```
1. WiFi conectado?
   ├─ NO → Sensor mode
   └─ SÍ → 2. Test Grafana ping
            ├─ OK → Gateway mode
            └─ FAIL → Sensor mode
```

**Grafana ping test:**
- URL configurable: `grafana_ping_url` en config.json
- Default: `http://192.168.1.1/ping`
- HTTP GET, expect cualquier 2xx response
- Timeout: 5s

**Force mode:**
- Config: `espnow_force_mode` = "gateway" | "sensor"
- Skip auto-detection si se configura
- Útil para testing o setups específicos

## Protocolo de Mensajes

### Tipos de Mensaje

```c
enum MessageType {
  MSG_BEACON = 0,       // Gateway broadcast
  MSG_PAIR_REQUEST = 1, // Sensor→Gateway pairing
  MSG_PAIR_ACK = 2,     // Gateway→Sensor ACK
  MSG_DATA = 3          // Sensor→Gateway data
};
```

### MSG_BEACON (0)

Gateway broadcasts cada 2s (default, configurable).

**Estructura:**
```c
{
  uint8_t msgType;      // 0
  uint8_t deviceId;     // MAC LSB
  uint8_t macAddr[6];   // Gateway MAC
  uint8_t channel;      // WiFi channel
  int8_t rssi;          // Gateway RSSI (self, ~0)
  uint32_t timestamp;   // Millis since boot
}
```

**Comportamiento:**
- Broadcast a `FF:FF:FF:FF:FF:FF`
- Intervalo: `beacon_interval_ms` (default 2000ms)
- Canal: Current WiFi channel (si conectado) o configured channel

### MSG_PAIR_REQUEST (1)

Sensor solicita pairing con gateway.

**Estructura:**
```c
{
  uint8_t msgType;      // 1
  uint8_t deviceId;     // Sensor MAC LSB
  uint8_t macAddr[6];   // Sensor MAC
  uint8_t channel;      // Sensor WiFi channel
  int8_t rssi;          // Beacon RSSI recibido
  uint32_t timestamp;   // Millis since boot
}
```

**Comportamiento:**
- Sensor envía al recibir BEACON
- Broadcast (gateway puede no estar en peer list aún)
- Solo si not paired o gateway cambió

### MSG_PAIR_ACK (2)

Gateway confirma pairing.

**Estructura:**
```c
{
  uint8_t msgType;      // 2
  uint8_t deviceId;     // Gateway MAC LSB
  uint8_t macAddr[6];   // Gateway MAC
  uint8_t channel;      // Gateway WiFi channel
  int8_t rssi;          // 0 (no usado)
  uint32_t timestamp;   // Millis since boot
}
```

**Comportamiento:**
- Gateway agrega sensor a peer list
- Envía ACK broadcast (sensor puede no estar en peer list del gateway aún para transmisión unicast)
- Sensor marca como paired al recibir

### MSG_DATA (3)

Sensor envía datos de sensores.

**Estructura:**
```c
{
  uint8_t msgType;      // 3
  uint8_t sensorId;     // Sensor MAC LSB
  float temperature;    // °C
  float humidity;       // %
  float co2;            // ppm
  uint32_t sequence;    // Incrementing counter
}
```

**Comportamiento:**
- Sensor envía cada `send_interval_ms` (default 30000ms)
- Unicast al gateway pareado
- Secuencia para detectar pérdida de paquetes

## Flujo de Discovery/Pairing

### Gateway side:

```
1. Init ESP-NOW
2. Add broadcast peer (FF:FF:FF:FF:FF:FF)
3. Start beacon broadcast (every 2s)
4. On MSG_PAIR_REQUEST received:
   - Add sensor MAC to peer list
   - Send MSG_PAIR_ACK
5. On MSG_DATA received:
   - Buffer data (ring buffer, size 10)
   - Main loop forwards to Grafana
```

### Sensor side:

```
1. Init ESP-NOW
2. Set WiFi channel to configured value
3. Add broadcast peer
4. Listen for MSG_BEACON
5. On BEACON received:
   - Compare RSSI with current gateway
   - If better (>10dBm hysteresis):
     - Remove old gateway peer
     - Add new gateway peer
     - Send MSG_PAIR_REQUEST
6. On MSG_PAIR_ACK received:
   - Mark as paired
7. Every send_interval_ms:
   - If paired: send MSG_DATA
```

## Multi-Gateway Support

Sensores pueden cambiar de gateway automáticamente.

**Selección de gateway:**
1. Sensor recibe BEACONs de múltiples gateways
2. Compara RSSI (señal)
3. Cambia si nuevo gateway > actual + 10dBm
4. Hysteresis de 10dBm previene ping-ponging

**Ejemplo:**
```
Gateway A: RSSI -50dBm (actual)
Gateway B: RSSI -55dBm → NO cambiar (diff < 10dBm)
Gateway B: RSSI -38dBm → SÍ cambiar (diff > 10dBm)
```

## WiFi Channel Synchronization

**Crítico:** Sensores y gateway deben estar en mismo canal WiFi.

### Gateway:
- Si conectado a WiFi → usa canal de la red
- Si no conectado → usa `espnow_channel` de config
- Canal obteni do vía `esp_wifi_get_channel()`

### Sensor:
- Fuerza canal vía `esp_wifi_set_channel()`
- Usa `espnow_channel` de config
- **Mandatory:** Sin esto, sensor no recibe BEACONs

**Configuración:**
```json
{
  "espnow_channel": 6
}
```

**Válido:** 1-13 (depende región)

**Debug:** Ver logs para "ESP-NOW: Channel set to X"

## Gateway Data Forwarding

**Problema:** ESP-NOW callbacks corren en contexto de interrupción WiFi.

**Solución:** Ring buffer + procesamiento en main loop.

### Ring Buffer (size 10):

```c
struct MeshDataBuffer {
  uint8_t senderMAC[6];
  float temp;
  float hum;
  float co2;
  uint32_t seq;
  bool valid;
};

MeshDataBuffer meshBuffer[10];
volatile int meshBufferHead = 0;
volatile int meshBufferTail = 0;
```

### Flujo:

```
1. ESP-NOW callback (interrupt context):
   - Encolar datos en buffer
   - No hacer HTTP calls (crash)

2. Main loop:
   - Procesar buffer
   - Crear mensaje Grafana
   - HTTP POST (safe en main loop)
```

**Buffer full:**
- Datos se pierden
- Log: "Buffer full, dropping data"
- Aumentar `send_interval_ms` o procesar más rápido

## Limitaciones

### Alcance
- **Línea de vista:** ~100m
- **Interior:** ~30m (depende paredes, interferencia)
- **Penetración:** Peor que 2.4GHz WiFi tradicional

### Capacidad
- **Max peers (gateway):** 20 sensores
- **Max buffer:** 10 mensajes
- **Throughput:** ~1 mensaje/s por sensor recomendado

### Reliability
- **No ACK en data:** ESP-NOW layer tiene ACK, pero no retry de app
- **Pérdida de paquetes:** Posible con señal débil
- **No encryption:** ESP-NOW puede encriptar, no implementado

### Channel restrictions
- **Must match:** Sensor y gateway mismo canal
- **WiFi roaming:** Si gateway roamea, sensores pierden conexión
- **Channel change:** Require sensor restart para detectar nuevo canal

## Configuración Completa

```json
{
  "espnow_enabled": true,
  "espnow_force_mode": "",
  "espnow_channel": 6,
  "beacon_interval_ms": 2000,
  "discovery_timeout_ms": 15000,
  "send_interval_ms": 30000,
  "grafana_ping_url": "http://192.168.1.1/ping"
}
```

**Parámetros:**
- `espnow_enabled`: true/false, habilita ESP-NOW
- `espnow_force_mode`: "", "gateway", o "sensor" (skip auto-detect)
- `espnow_channel`: 1-13, canal WiFi
- `beacon_interval_ms`: Frecuencia de beacons (gateway)
- `discovery_timeout_ms`: Timeout para discovery (sensor)
- `send_interval_ms`: Intervalo de envío de datos (sensor)
- `grafana_ping_url`: URL para test de gateway (auto-detect)

## Debugging

### Gateway logs:
```
ESP-NOW: Gateway mode
ESP-NOW: Channel set to 6
[ESP-NOW] Beacon broadcast started (every 2000ms)
[ESP-NOW] Pair request from sensor XX
[ESP-NOW] Sensor added to peer list
[ESP-NOW] Data buffered from sensor XX (seq=123)
[MESH→GRAFANA] mesh_XXYYZZ: T=25.1 H=60.5 CO2=450 (seq=123)
```

### Sensor logs:
```
ESP-NOW: Sensor mode
ESP-NOW: Channel set to 6
[ESP-NOW] Listening for gateway beacons...
[ESP-NOW] Gateway found: AA:BB:CC:DD:EE:FF (RSSI=-45dBm)
[ESP-NOW] Pair request sent
[ESP-NOW] Paired with gateway
[ESP-NOW] Data sent (seq=1)
```

### Common issues:

**Sensor no detecta gateway:**
- Check channel match
- Ver logs: "Channel set to X"
- Gateway debe estar broadcasting (ver log de beacons)
- Alcance: mover más cerca

**Gateway no recibe datos:**
- Check peer list (max 20)
- Ver buffer full warning
- RSSI muy bajo: mover sensores más cerca

**Datos no llegan a Grafana:**
- Gateway debe estar en gateway mode
- Check HTTP POST en logs
- Ver errores de Grafana (timeout, 404, etc)

## Performance

### Típico (30s send interval, 3 sensores):
- **RAM (gateway):** +2KB vs sin ESP-NOW
- **RAM (sensor):** +1KB vs sin ESP-NOW
- **CPU:** Minimal (<1%)
- **Latency:** 30-60s end-to-end (sensor→Grafana)

### Optimización:

**Para baja latency:**
- Reducir `send_interval_ms` (min 5000ms recomendado)
- Reducir `beacon_interval_ms` (min 500ms)

**Para bajo consumo (sensores a batería):**
- Aumentar `send_interval_ms` (60000ms+)
- Deep sleep entre envíos (no implementado)

## Seguridad

**Estado actual:** Sin encriptación.

**Vulnerabilidades:**
- Datos en claro (temperatura, humedad, CO2 visibles)
- MAC spoofing posible
- Pairing sin autenticación

**Mitigación:**
- Red física aislada
- ESP-NOW encryption (no implementado, pero disponible en ESP-IDF)

## Casos de Uso

### 1. Sensor remoto exterior
```
Config (sensor):
{
  "espnow_enabled": true,
  "espnow_channel": 6,
  "send_interval_ms": 60000
}

Config (gateway):
{
  "espnow_enabled": true,
  "espnow_channel": 6
}
```

### 2. Múltiples sensores en malla
```
Gateway: 1 ESP32 con WiFi
Sensores: 5-10 ESP32 sin WiFi
Canal: Todos en canal 6
Alcance: <30m interior
```

### 3. Redundancia multi-gateway
```
Gateway A: Canal 6, zona norte
Gateway B: Canal 6, zona sur
Sensores: Auto-selección por RSSI
```

## Troubleshooting Avanzado

### Packet loss
**Síntoma:** Sequence numbers con gaps
**Causa:** RSSI bajo, interferencia
**Fix:**
- Mejorar alcance (antena externa, relocation)
- Aumentar `send_interval_ms` (menos colisiones)

### Gateway switch frecuente
**Síntoma:** Logs muestran cambios de gateway
**Causa:** RSSIs similares, hysteresis insuficiente
**Fix:**
- Aumentar hysteresis en código (actual 10dBm)
- Ubicar gateways más alejados

### Buffer overflow
**Síntoma:** "Buffer full" en logs de gateway
**Causa:** Main loop bloqueado, datos llegan más rápido
**Fix:**
- Aumentar buffer size (modificar código)
- Reducir `send_interval_ms` de sensores
- Optimizar HTTP POST (reducir timeout)

## Implementación en Código

**Archivos clave:**
- `include/ESPNowManager.h` - Clase principal (singleton)
- `src/main.cpp` - Integración, buffer processing
- `src/endpoints.cpp` - API `/espnow/status`

**Puntos de entrada:**
```cpp
// Init
espnowMgr.begin(role, channel, onDataCallback);

// Update (main loop)
espnowMgr.update();

// Status
ESPNowStatus status = espnowMgr.getStatus();
```

**Callback:**
```cpp
void onMeshDataReceived(
  const uint8_t* mac,
  float temp,
  float hum,
  float co2,
  uint32_t seq
) {
  // Runs in WiFi interrupt context
  // Buffer data, process in main loop
}
```
