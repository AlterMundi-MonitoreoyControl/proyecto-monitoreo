#include "actuators/RelayManager.h"
#include "configFile.h"
#include "constants.h"
#include "createGrafanaMessage.h"
#include "debug.h"
#include "endpoints.h"
#include "globals.h"
#include "otaUpdater.h"
#include "sendDataGrafana.h"
#include "version.h"
#include "core/ControlMediator.h"
#include "core/TimerSource.h"
#include "core/RuleLoader.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <esp_ota_ops.h>
#include <time.h>

#ifdef SENSOR_MULTI
#include "SensorManager.h"
#include "sensors/ICO2Sensor.h"
#include "sensors/IHumiditySensor.h"
#include "sensors/IMoistureSensor.h"
#include "sensors/ITemperatureSensor.h"
SensorManager sensorMgr;
// Wrapper function for endpoints.cpp to access sensors without including full header
std::vector<ISensor *> &getSensorList() { return sensorMgr.getSensors(); }
#else
#include "sensors/ICO2Sensor.h"
#include "sensors/IHumiditySensor.h"
#include "sensors/IMoistureSensor.h"
#include "sensors/ITemperatureSensor.h"
#include "sensors/SensorFactory.h"
#endif

#ifdef ENABLE_RS485
#include "RS485Manager.h"
RS485Manager rs485;
#endif

#ifdef ENABLE_ESPNOW
#include "ESPNowManager.h"
ESPNowManager espnowMgr;
#endif

RelayManager relayMgr;

// ── Mediator / Timer / Rules ──────────────────────────────────────────────────
ControlMediator mediator;
TimerSource     timerSource;

// Forward declarations for new endpoints
void handleRelayList();
void handleRelayToggle();
void handleActuatorCommand();
void handleActuatorStatus();
void handleRulesReload();
void handleRulesGet();
void handleRulesSave();
void handleRulesEditor();

unsigned long lastUpdateCheck = 0;
unsigned long lastSendTime    = 0;
unsigned long lastReadTime    = 0;   // ← NEW: independent read timer
uint32_t      sendIntervalMs  = 10000;
uint32_t      readIntervalMs  = 3000;

#ifdef ENABLE_OTA
bool otaInitialized = false;
#endif

#ifdef ENABLE_ESPNOW
// Mesh data buffer structure to avoid HTTP calls from WiFi interrupt context
struct MeshDataBuffer {
  uint8_t senderMAC[6];
  char sensorId[32];
  float temp;
  float hum;
  float co2;
  uint32_t seq;
  bool valid;
};

const int MESH_BUFFER_SIZE = 10;
MeshDataBuffer meshBuffer[MESH_BUFFER_SIZE];
volatile int meshBufferHead = 0;
volatile int meshBufferTail = 0;
#endif

#ifndef UNIT_TEST

#ifdef ENABLE_ESPNOW

// Callback to enqueue mesh data (gateway only)
// IMPORTANT: Runs in WiFi interrupt context - must not call HTTP/blocking
// functions
void onMeshDataReceived(const uint8_t *senderMAC, float temp, float hum,
                        float co2, uint32_t seq, const char *sensorId) {
  int nextHead = (meshBufferHead + 1) % MESH_BUFFER_SIZE;

  if (nextHead == meshBufferTail) {
    DBG_ERROR("[MESH] Buffer full, dropping data\n");
    return;
  }

  memcpy(meshBuffer[meshBufferHead].senderMAC, senderMAC, 6);

  if (sensorId != nullptr) {
    strncpy(meshBuffer[meshBufferHead].sensorId, sensorId,
            sizeof(meshBuffer[meshBufferHead].sensorId) - 1);
    meshBuffer[meshBufferHead]
        .sensorId[sizeof(meshBuffer[meshBufferHead].sensorId) - 1] = '\0';
  } else {
    strcpy(meshBuffer[meshBufferHead].sensorId, "unknown");
  }
  meshBuffer[meshBufferHead].temp = temp;
  meshBuffer[meshBufferHead].hum  = hum;
  meshBuffer[meshBufferHead].co2  = co2;
  meshBuffer[meshBufferHead].seq  = seq;
  meshBuffer[meshBufferHead].valid = true;

  meshBufferHead = nextHead;

  // Feed mediator — use packet sequence as monotonic counter per remote sensor.
  // nodeId = last byte of sender MAC (unique per board, matches SensorBase logic).
  // sensorIds 0/1/2 are conventional for temp/hum/co2 on remote nodes.
  uint8_t nodeId = senderMAC[5];

  if (temp > -200.0f) {
    SensorReading r; r.key = {nodeId, 0}; r.value = temp; r.counter = seq;
    mediator.onSensorReading(r);
  }
  if (hum > -200.0f) {
    SensorReading r; r.key = {nodeId, 1}; r.value = hum;  r.counter = seq;
    mediator.onSensorReading(r);
  }
  if (co2 > -200.0f) {
    SensorReading r; r.key = {nodeId, 2}; r.value = co2;  r.counter = seq;
    mediator.onSensorReading(r);
  }
}

String detectRole(const JsonDocument &config) {
  DBG_INFOLN("\n[INFO] Auto-detecting device role...");

  // 1. Check if WiFi is connected
  if (!wifiManager.isOnline()) {
    DBG_INFOLN("  No WiFi -> SENSOR mode");
    return "sensor";
  }

  DBG_INFOLN("  WiFi connected, checking Grafana...");

  // 2. Test Grafana connectivity
  String grafanaUrl = config["grafana_ping_url"] | "http://192.168.1.1/ping";

  HTTPClient http;
  http.begin(grafanaUrl);
  http.setTimeout(3000); // 3 second timeout

  int httpCode = http.GET();
  http.end();

  if (httpCode > 0) {
    DBG_INFO("  Grafana OK (HTTP %d) -> GATEWAY mode\n", httpCode);
    return "gateway";
  } else {
    DBG_INFO("  Grafana unreachable (%d) -> SENSOR mode\n", httpCode);
    return "sensor";
  }
}
#endif

void printBanner() {
  DBG_INFOLN("\n  ALTERMUNDI - Proyecto Monitoreo");
  DBG_INFOLN("  La pata tecnologica de ese otro mundo posible\n");
}

void setup() {
  DEBUG_BEGIN(115200);
  delay(500);

  IF_INFO(printBanner());

  DBG_INFOLN("[INFO] Starting system...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  DBG_INFOLN("=== SYSTEM INIT ===");

  if (!SPIFFS.begin(true)) {
    DBG_ERRORLN("[ERR] SPIFFS mount failed");
  } else {
    DBG_INFOLN("[OK] SPIFFS mounted");
  }

  secrets.begin();

  createConfigFile();

  // Load configuration ONCE for all modules
  JsonDocument config = loadConfig();

  IF_VERBOSE({
    DBG_INFOLN("\n[INFO] Config loaded:");
    String configOutput;
    serializeJsonPretty(config, configOutput);
    DBG_INFOLN(configOutput.c_str());
  });

#ifdef ENABLE_RS485
  DBG_INFOLN("\n[INFO] Configuring RS485...");
  JsonObject rs485Cfg = config["rs485"];
  bool rs485Enabled = rs485Cfg["enabled"] | false;

  if (!rs485Enabled) {
    DBG_INFOLN("[INFO] RS485/Modbus disabled in config");
  } else {
    int rx = rs485Cfg["rx_pin"] | 16;
    int tx = rs485Cfg["tx_pin"] | 17;
    int de = rs485Cfg["de_pin"] | 18;
    int baud = rs485Cfg["baudrate"] | 9600;
    bool rawSendEnabled = rs485Cfg["raw_send_enabled"] | false;

    DBG_INFO("[INFO] RS485: RX=%d TX=%d DE=%d Baud=%d\n", rx, tx, de, baud);
    DBG_VERBOSE("[INFO] RS485 Raw Send: %s\n",
                rawSendEnabled ? "enabled" : "disabled");

    // 1. Initialize ModbusManager first (Singleton owner of the bus)
    ModbusManager::getInstance().begin(rx, tx, de, baud);

    // 2. Initialize RS485Manager (will reuse ModbusManager's serial if
    // available)
    rs485.init(rx, tx, baud, de, de);
    rs485.setRawSendEnabled(rawSendEnabled);

    DBG_INFOLN("[OK] RS485/Modbus enabled");
    delay(100);
  }
#endif

  DBG_INFOLN("\n[INFO] Initializing sensors...");
#ifdef SENSOR_MULTI
  sensorMgr.loadFromConfig(config);

  uint16_t modbusDelay = config["modbus_delay_ms"] | 50;
  sensorMgr.setModbusDelay(modbusDelay);

  // Read + send intervals from config
  sendIntervalMs = config["send_interval_ms"] | 10000UL;
  readIntervalMs = config["read_interval_ms"]  | 3000UL;
  DBG_INFO("[OK] Timing: read=%lums send=%lums\n", readIntervalMs, sendIntervalMs);

  int sensorCount = sensorMgr.getSensorCount();
  DBG_INFO("[OK] Multi-sensor: %d active\n", sensorCount);

  IF_VERBOSE({
    for (auto *s : sensorMgr.getSensors()) {
      if (s && s->isActive()) {
        DBG_VERBOSE("  - %s\n", sensorMgr.getSensorId(s).c_str());
      }
    }
  });

  // Initialize Relays
  DBG_INFOLN("\n[INFO] Initializing Relays...");
  relayMgr.loadFromConfig(config);
  DBG_INFO("[OK] %d relays configured\n", relayMgr.getRelays().size());
  for (auto *r : relayMgr.getRelays()) {
    if (r) {
      r->init();
      // Register each individual relay channel as an actuator in mediator
      mediator.registerActuator(r->getChannel(0));
      mediator.registerActuator(r->getChannel(1));
    }
  }

  for (auto& g : relayMgr.getGpioRelays()) {
    if (g.actuator) {
      g.actuator->begin();
      mediator.registerActuator(g.actuator);
    }
  }

  // Load automation rules from SPIFFS
  DBG_INFOLN("[INFO] Loading rules...");
  int rulesLoaded = RuleLoader::load(mediator);
  DBG_INFO("[OK] %d rules loaded\n", rulesLoaded);
#else
  sensor = SensorFactory::createSensor();
  if (sensor) {
    if (sensor->init()) {
      DBG_INFO("[OK] Sensor %s initialized\n", sensor->getSensorType());
    } else {
      DBG_ERROR("[ERR] Init failed: %s\n", sensor->getSensorType());
    }
  } else {
    DBG_ERRORLN("[ERR] Could not create sensor");
  }
#endif

  clientSecure.setInsecure();

  server.on("/", HTTP_GET, handleHome);
  server.on("/actual", HTTP_GET, handleMediciones);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/config", HTTP_GET, handleConfiguracion);
  server.on("/config", HTTP_POST, habldePostConfig);
  server.on("/config/reset", HTTP_POST, handleConfigReset);
  server.on("/data", HTTP_GET, handleData);
  server.on("/calibrate-scd30", HTTP_GET, handleSCD30Calibration);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/restart", HTTP_POST, handleRestart);

  // Relay endpoints (legacy)
  server.on("/api/relays", HTTP_GET, handleRelayList);
  server.on("/api/relay/toggle", HTTP_POST, handleRelayToggle);

  // Mediator / actuator endpoints
  server.on("/actuator/command", HTTP_POST, handleActuatorCommand);
  server.on("/actuator/status",  HTTP_GET,  handleActuatorStatus);
  server.on("/rules/reload",     HTTP_POST, handleRulesReload);
  server.on("/rules",            HTTP_GET,  handleRulesGet);
  server.on("/rules/save",       HTTP_POST, handleRulesSave);
  server.on("/rules-editor",     HTTP_GET,  handleRulesEditor);
  server.on("/api/admin/info", HTTP_GET, handleApiAdminInfo);
  server.on("/api/admin/password", HTTP_POST, handleApiAdminPassword);

  server.on("/style.css", HTTP_GET, handleStyle);
  server.on("/config.js", HTTP_GET, handleConfigJs);
  server.on("/favicon.svg", HTTP_GET, handleFavicon);

#ifdef ENABLE_ESPNOW
  server.on("/espnow/status", HTTP_GET, handleESPNowStatus);
#endif

  // Serve static files from SPIFFS (CSS, JS, Favicon)
  server.onNotFound([]() {
    String uri = server.uri();

    // First try to find file in /data folder (root of SPIFFS)
    if (SPIFFS.exists(uri)) {
      File file = SPIFFS.open(uri, "r");
      String contentType = "text/plain";
      if (uri.endsWith(".css"))
        contentType = "text/css";
      else if (uri.endsWith(".js"))
        contentType = "application/javascript";
      else if (uri.endsWith(".html"))
        contentType = "text/html";
      else if (uri.endsWith(".svg"))
        contentType = "image/svg+xml";
      else if (uri.endsWith(".png"))
        contentType = "image/png";
      else if (uri.endsWith(".ico"))
        contentType = "image/x-icon";

      server.streamFile(file, contentType);
      file.close();
      return;
    }

    // Default 404 behavior: redirect to home
    DBG_VERBOSE("404 redirect: %s\n", uri.c_str());
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.enableCORS(true);

  DBG_INFOLN("\n[INFO] Configuring WiFi Manager...");
  wifiManager.setConnectionTimeout(15000);
  wifiManager.setMaxRetries(8);
  wifiManager.setValidationTimeout(30000);
  wifiManager.init(&server);
  DBG_INFOLN("[OK] WiFi Manager initialized");

#ifdef ENABLE_ESPNOW
  DBG_INFOLN("\n[INFO] Configuring ESP-NOW...");
  bool espnowEnabled = config["espnow_enabled"] | false;

  if (espnowEnabled) {
    // Auto-detect role or use forced mode
    String forcedMode = config["espnow_force_mode"] | "";
    String espnowMode;

    if (forcedMode != "") {
      DBG_INFO("[INFO] Forced mode: %s\n", forcedMode.c_str());
      espnowMode = forcedMode;
    } else {
      espnowMode = detectRole(config);
    }

    uint8_t espnowChannel = config["espnow_channel"] | 1;

    if (espnowChannel < 1 || espnowChannel > 13) {
      DBG_INFO("[WARN] Invalid channel %d, using 1\n", espnowChannel);
      espnowChannel = 1;
    }

    DBG_INFO("[INFO] ESP-NOW mode: %s (ch %d)\n", espnowMode.c_str(),
             espnowChannel);

    if (espnowMode == "gateway") {
      if (wifiManager.isOnline()) {
        uint8_t wifiChannel = WiFi.channel();
        if (wifiChannel >= 1 && wifiChannel <= 13) {
          espnowChannel = wifiChannel;
          DBG_VERBOSE("[INFO] Gateway using WiFi channel: %d\n", espnowChannel);
        }
      }
    }

    if (espnowMgr.init(espnowMode, espnowChannel)) {
      DBG_INFOLN("[OK] ESP-NOW initialized");

      if (espnowMode == "sensor") {
        DBG_INFOLN("[INFO] Sensor mode: searching gateway...");
        if (espnowMgr.waitForDiscovery()) {
          DBG_INFOLN("[OK] Gateway found and paired");
        } else {
          DBG_INFOLN("[WARN] Gateway not found (will retry)");
        }
      } else {
        espnowMgr.setMeshDataCallback(onMeshDataReceived);
        DBG_INFOLN("[INFO] Gateway mode: beacon + forwarding");
      }
    } else {
      DBG_ERRORLN("[ERR] ESP-NOW init failed");
    }
  } else {
    DBG_INFOLN("[INFO] ESP-NOW disabled in config");
  }
#endif

  server.begin();
  DBG_INFOLN("[OK] Web server started on port 80");

#ifdef ENABLE_OTA
  DBG_INFOLN("[INFO] OTA will initialize when WiFi connects");
#endif

  DBG_INFOLN("\n=== SYSTEM READY ===");
  DBG_INFO("  AP: %s\n", wifiManager.getAPSSID().c_str());
  DBG_INFOLN("  Config: http://192.168.16.10");
  DBG_INFOLN("  Data:   http://<IP>/data\n");
}

void loop() {
#ifdef ENABLE_OTA
  // Lazy OTA initialization - wait for WiFi to be connected
  if (wifiManager.isOnline() && !isLocalOTAReady()) {
    initLocalOTA(wifiManager.getAPSSID().c_str());
  }
  handleLocalOTA();
#endif
  wifiManager.update();

  IF_VERBOSE({
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint > 30000) {
      lastStatusPrint = millis();
      if (wifiManager.isOnline()) {
        DBG_VERBOSE("WiFi: %s IP: %s\n", wifiManager.getCurrentSSID().c_str(),
                    wifiManager.getLocalIP().toString().c_str());
      } else {
        DBG_VERBOSE("WiFi: Disconnected, AP: %s\n",
                    wifiManager.getAPSSID().c_str());
      }
    }
  });

  server.handleClient();

#ifdef ENABLE_ESPNOW
  espnowMgr.update();

  // Process buffered mesh data (gateway only)
  while (meshBufferTail != meshBufferHead) {
    MeshDataBuffer *data = &meshBuffer[meshBufferTail];
    if (data->valid) {
      char deviceid[32];
      snprintf(deviceid, sizeof(deviceid), "moni-%02X%02X%02X%02X%02X%02X",
               data->senderMAC[0], data->senderMAC[1], data->senderMAC[2],
               data->senderMAC[3], data->senderMAC[4], data->senderMAC[5]);

      DBG_VERBOSE("[MESH] %s: T=%.1f H=%.1f CO2=%.0f\n", deviceid, data->temp,
                  data->hum, data->co2);

      sendDataGrafana(data->temp, data->hum, data->co2, data->sensorId,
                      deviceid);
      data->valid = false;
    }
    meshBufferTail = (meshBufferTail + 1) % MESH_BUFFER_SIZE;
  }
#endif

  unsigned long currentMillis = millis();

  // ── Mediator tick (auto-off, timer sources) ───────────────────────────
  mediator.tick();
  timerSource.tick(mediator);

  // Check for firmware updates
  if (currentMillis - lastUpdateCheck >= UPDATE_INTERVAL) {
    DBG_VERBOSE("Free heap: %d bytes\n", ESP.getFreeHeap());
    checkForUpdates();
    lastUpdateCheck = currentMillis;
  }

  // ── Sensor READ every read_interval_ms ───────────────────────────────
#ifdef SENSOR_MULTI
  if (currentMillis - lastReadTime >= readIntervalMs) {
    lastReadTime = currentMillis;
    sensorMgr.readAllAndNotify(mediator);   // read + notify mediator
  }
#endif

  // ── Data SEND every send_interval_ms ─────────────────────────────────
  if (currentMillis - lastSendTime >= sendIntervalMs) {
    lastSendTime = currentMillis;

#ifdef SENSOR_MULTI
    sensorMgr.readAll();  // legacy read for Grafana send path

    for (auto *s : sensorMgr.getSensors()) {
      if (s->isActive()) {
        float temperature = -999, humidity = -999, co2 = -999;

        auto *tempSensor = dynamic_cast<ITemperatureSensor *>(s);
        auto *humSensor = dynamic_cast<IHumiditySensor *>(s);
        auto *moistSensor = dynamic_cast<IMoistureSensor *>(s);
        auto *co2Sensor = dynamic_cast<ICO2Sensor *>(s);

        if (tempSensor)
          temperature = tempSensor->getTemperature();
        if (humSensor)
          humidity = humSensor->getHumidity();
        if (moistSensor)
          humidity = moistSensor->getMoisture();
        if (co2Sensor)
          co2 = co2Sensor->getCO2();

        DBG_INFO("[%s] %s\n", s->getSensorID(), s->getMeasurementsString());

        sendDataGrafana(s->getMeasurementsString(), s->getSensorID());

#ifdef ENABLE_RS485
        rs485.sendSensorData(temperature, humidity, co2, s->getSensorID());
#endif

#ifdef ENABLE_ESPNOW
        if (espnowMgr.getMode() == "sensor" && espnowMgr.isPaired()) {
          espnowMgr.sendSensorData(temperature, humidity, co2,
                                   s->getSensorID());
        }
#endif
      }
    }

#else
    float temperature = 99, humidity = 100, co2 = 999999;

    if (sensor && sensor->isActive() && sensor->dataReady()) {
      if (sensor->read()) {
        auto *tempSensor = dynamic_cast<ITemperatureSensor *>(sensor);
        auto *humSensor = dynamic_cast<IHumiditySensor *>(sensor);
        auto *moistSensor = dynamic_cast<IMoistureSensor *>(sensor);
        auto *co2Sensor = dynamic_cast<ICO2Sensor *>(sensor);

        if (tempSensor)
          temperature = tempSensor->getTemperature();
        if (humSensor)
          humidity = humSensor->getHumidity();
        if (moistSensor)
          humidity = moistSensor->getMoisture();
        if (co2Sensor)
          co2 = co2Sensor->getCO2();

        DBG_INFO("[%s] %s\n", sensor->getSensorType(),
                 sensor->getMeasurementsString());
      } else {
        DBG_ERRORLN("Sensor read error!");
        return;
      }
    } else {
      DBG_VERBOSELN("Sensor not ready...");
    }

    sendDataGrafana(sensor->getMeasurementsString(), sensor->getSensorID());

#ifdef ENABLE_RS485
    rs485.sendSensorData(temperature, humidity, co2,
                         sensor ? sensor->getSensorType() : "Unknown");
#endif
#endif

    // Report Relays to Grafana
    for (auto *r : relayMgr.getRelays()) {
      if (r) {
        r->syncState();
        r->syncInputs(mediator);

        if (r->isActive()) {
          String data = r->getGrafanaString();
          String id = r->getAlias();
          if (id.length() == 0)
            id = "relay_" + String(r->getAddress());
          id.replace(" ", "_");
          sendDataGrafana(data.c_str(), id.c_str());
        }
      }
    }

    // Report GPIO Relays to Grafana
    for (auto& g : relayMgr.getGpioRelays()) {
      if (g.actuator) {
        String data = "relay1=" + String(g.actuator->getState() ? 1 : 0);
        String id = g.alias;
        if (id.length() == 0)
          id = "gpio_" + String(g.pin);
        id.replace(" ", "_");

        sendDataGrafana(data.c_str(), id.c_str());
      }
    }
  }
  delay(10);
}

#endif // UNIT_TEST