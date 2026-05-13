#include "endpoints.h"
#include "actuators/RelayManager.h"
#include "configFile.h"
#include "constants.h"
#include "debug.h"
#include "globals.h"
#include "version.h"
#include "web_assets.h"
// #include "webConfigPage.h" // Removed
#include "sensors/ICO2Sensor.h"
#include "sensors/IHumiditySensor.h"
#include "sensors/IMoistureSensor.h"
#include "sensors/IPressureSensor.h"
#include "sensors/ISensor.h"
#include "sensors/ISoilSensor.h"
#include "sensors/ITemperatureSensor.h"
#include "sensors/SensorCapacitive.h"
#include "sensors/HD38Sensor.h"
#include "core/ControlMediator.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#ifdef SENSOR_MULTI
#include "sensors/ICO2Sensor.h"
#include "sensors/IHumiditySensor.h"
#include "sensors/IMoistureSensor.h"
#include "sensors/IPressureSensor.h"
#include "sensors/ISensor.h"
#include "sensors/ISoilSensor.h"
#include "sensors/ITemperatureSensor.h"
#include <vector>
class SensorManager;
extern SensorManager sensorMgr;
extern std::vector<ISensor *> &getSensorList();
#endif

#ifdef ENABLE_ESPNOW
#include "ESPNowManager.h"
extern ESPNowManager espnowMgr;
#endif

extern RelayManager relayMgr;
extern ControlMediator mediator;

// Helper to get sensor icon based on capabilities
String getSensorIcon(ISensor *s) {
#ifdef SENSOR_MULTI
  if (dynamic_cast<ICO2Sensor *>(s))
    return "🌬️";
  if (dynamic_cast<ISoilSensor *>(s))
    return "🌱";
  if (dynamic_cast<IMoistureSensor *>(s))
    return "🌱";
  if (dynamic_cast<IPressureSensor *>(s))
    return "🌡️";
  if (dynamic_cast<ITemperatureSensor *>(s))
    return "🌡️";
#endif
  return "📊";
}

// Helper to serve static files from SPIFFS with content-type and cache control
void serveStaticFile(const char *path, const char *contentType) {
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      return;
    }
  }
  server.send(404, "text/plain", "File not found");
}

// Basic Auth gate. Opt-in: returns true (authorized) when no admin password
// is configured, so existing devices in the field keep working. Once the user
// sets a password via /api/admin/password, every protected handler enforces it.
bool requireAdminAuth() {
  if (!secrets.hasAdminPassword()) {
    return true;
  }
  if (server.authenticate("admin", secrets.getAdminPassword().c_str())) {
    return true;
  }
  server.requestAuthentication();
  return false;
}

void handleMediciones() {
  JsonDocument doc;
  JsonArray sensors = doc["sensors"].to<JsonArray>();

  String wifiStatus =
      (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";

#ifdef SENSOR_MULTI
  for (auto *s : getSensorList()) {
    if (!s)
      continue;

    // Read sensor (if not already read recently)
    // Note: main loop handles periodic reading, but we can try read here if
    // needed. However, read() might be slow. Better to use cached values or
    // assume main loop updates them. For single sensor architecture, read() was
    // called here. Let's assume data is relatively fresh or read it if ready.
    if (s->dataReady())
      s->read();

    JsonObject sensorObj = sensors.add<JsonObject>();
    sensorObj["type"] = s->getSensorType();
    sensorObj["id"] = s->getSensorID();
    sensorObj["icon"] = getSensorIcon(s);
    sensorObj["active"] = s->isActive();
    sensorObj["error"] = !s->isActive();

    JsonArray readings = sensorObj["readings"].to<JsonArray>();

    auto *tempSensor = dynamic_cast<ITemperatureSensor *>(s);
    auto *humSensor = dynamic_cast<IHumiditySensor *>(s);
    auto *co2Sensor = dynamic_cast<ICO2Sensor *>(s);
    auto *moistSensor = dynamic_cast<IMoistureSensor *>(s);
    auto *capSensor = dynamic_cast<SensorCapacitive *>(s);
    auto *hd38Sensor = dynamic_cast<HD38Sensor *>(s);
    auto *presSensor = dynamic_cast<IPressureSensor *>(s);
    auto *soilSensor = dynamic_cast<ISoilSensor *>(s);

    if (tempSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Temp";
      r["value"] = String(tempSensor->getTemperature(), 1);
      r["unit"] = "°C";
      r["status"] = s->isActive() ? "ok" : "warn";
      r["id"] = tempSensor->getSensorID();
      r["key"] = tempSensor->getKey().toU32();
      r["key_device"] = tempSensor->getKey().deviceId;
      r["key_sensor"] = tempSensor->getKey().sensorId;
      r["key_var"] = (uint8_t)SensorVariable::TEMPERATURE;
    }
    if (humSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Humedad";
      r["value"] = String(humSensor->getHumidity(), 1);
      r["unit"] = "%";
      r["status"] = s->isActive() ? "ok" : "warn";
      r["id"] = humSensor->getSensorID();
      r["key"] = humSensor->getKey().toU32();
      r["key_device"] = humSensor->getKey().deviceId;
      r["key_sensor"] = humSensor->getKey().sensorId;
      r["key_var"] = (uint8_t)SensorVariable::HUMIDITY;
    }
    if (co2Sensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "CO2";
      r["value"] = String(co2Sensor->getCO2(), 0);
      r["unit"] = "ppm";
      r["id"] = co2Sensor->getSensorID();
      r["key"] = co2Sensor->getKey().toU32();
      r["key_device"] = co2Sensor->getKey().deviceId;
      r["key_sensor"] = co2Sensor->getKey().sensorId;
      r["key_var"] = (uint8_t)SensorVariable::CO2;

      float val = co2Sensor->getCO2();
      if (val > 1000)
        r["status"] = "warn";
      else if (val > 1500)
        r["status"] = "bad";
      else
        r["status"] = "ok";
    }
    if (moistSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Humedad";
      r["value"] = String(moistSensor->getMoisture(), 1);
      r["unit"] = "%";
      r["status"] = s->isActive() ? "ok" : "warn";
      r["id"] = moistSensor->getSensorID();
      r["key"] = moistSensor->getKey().toU32();
      r["key_device"] = moistSensor->getKey().deviceId;
      r["key_sensor"] = moistSensor->getKey().sensorId;
      r["key_var"] = (uint8_t)SensorVariable::MOISTURE;
    }
    if (capSensor) {
      JsonObject diag = sensorObj["diagnostics"].to<JsonObject>();
      diag["raw"] = capSensor->getRawValue();
      diag["pin"] = capSensor->getPin();
      diag["id"] = capSensor->getSensorID();
      diag["key"] = capSensor->getKey().toU32();
      JsonObject cal = sensorObj["calibration"].to<JsonObject>();
      cal["dry"] = capSensor->getDryValue();
      cal["wet"] = capSensor->getWetValue();
    }
    if (hd38Sensor) {
      JsonObject diag = sensorObj["diagnostics"].to<JsonObject>();
      diag["raw"] = hd38Sensor->getRawValue();
      diag["pin"] = hd38Sensor->getPin();
      diag["id"] = hd38Sensor->getSensorID();
      diag["key"] = hd38Sensor->getKey().toU32();
      JsonObject cal = sensorObj["calibration"].to<JsonObject>();
      cal["dry"] = hd38Sensor->getDryValue();
      cal["wet"] = hd38Sensor->getWetValue();
    }
    if (presSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Presión";
      r["value"] = String(presSensor->getPressure(), 1);
      r["unit"] = "hPa";
      r["status"] = s->isActive() ? "ok" : "warn";
      r["id"] = presSensor->getSensorID();
      r["key"] = presSensor->getKey().toU32();
    }

    // Soil Sensor specific extras (N, P, K, pH, EC) if available via generic
    // interface Currently specialized casting might be needed if ISoilSensor
    // exposes them directly
    if (soilSensor) {
      // Assuming ISoilSensor might have these methods in future or we need to
      // cast to specific implementation For now, let's stick to standard
      // interfaces. If specific ModbusSoilSensor, we might need dynamic_cast if
      // headers are available
    }
  }
#else
  if (sensor && sensor->isActive()) {
    if (sensor->dataReady())
      sensor->read();

    JsonObject sensorObj = sensors.add<JsonObject>();
    sensorObj["type"] = sensor->getSensorType();
    sensorObj["id"] = sensor->getSensorID();
    sensorObj["icon"] = getSensorIcon(sensor);
    sensorObj["active"] = true;
    sensorObj["error"] = !sensor->dataReady(); // Rough check

    JsonArray readings = sensorObj["readings"].to<JsonArray>();

    auto *tempSensor = dynamic_cast<ITemperatureSensor *>(sensor);
    auto *humSensor = dynamic_cast<IHumiditySensor *>(sensor);
    auto *moistSensor = dynamic_cast<IMoistureSensor *>(sensor);
    auto *capSensor = dynamic_cast<SensorCapacitive *>(sensor);
    auto *hd38Sensor = dynamic_cast<HD38Sensor *>(sensor);
    auto *co2Sensor = dynamic_cast<ICO2Sensor *>(sensor);

    if (tempSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Temp";
      r["value"] = String(tempSensor->getTemperature(), 1);
      r["unit"] = "°C";
      r["status"] = "ok";
    }
    if (humSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Humedad";
      r["value"] = String(humSensor->getHumidity(), 1);
      r["unit"] = "%";
      r["status"] = "ok";
    }
    if (moistSensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "Humedad";
      r["value"] = String(moistSensor->getMoisture(), 1);
      r["unit"] = "%";
      r["status"] = "ok";
    }
    if (co2Sensor) {
      JsonObject r = readings.add<JsonObject>();
      r["label"] = "CO2";
      r["value"] = String(co2Sensor->getCO2(), 0);
      r["unit"] = "ppm";
      r["status"] = "ok";
    }
    if (capSensor) {
      JsonObject diag = sensorObj["diagnostics"].to<JsonObject>();
      diag["raw"] = capSensor->getRawValue();
      diag["pin"] = capSensor->getPin();

      JsonObject cal = sensorObj["calibration"].to<JsonObject>();
      cal["dry"] = capSensor->getDryValue();
      cal["wet"] = capSensor->getWetValue();
    }
    if (hd38Sensor) {
      JsonObject diag = sensorObj["diagnostics"].to<JsonObject>();
      diag["raw"] = hd38Sensor->getRawValue();
      diag["pin"] = hd38Sensor->getPin();

      JsonObject cal = sensorObj["calibration"].to<JsonObject>();
      cal["dry"] = hd38Sensor->getDryValue();
      cal["wet"] = hd38Sensor->getWetValue();
    }
  }
#endif
  DBG_INFO("[Endpoint] entradas digitales ....... " );

  // Exponer Entradas Digitales de los reles Modbus como sensores visuales
  for (auto *r : relayMgr.getRelays()) {
    if (!r) continue;
    DBG_INFO("[Endpoint] Syncing relay addr=%d inputs is active= %i\n", r->getAddress(), r->isActive() );

    r->syncState();
    r->syncInputs(mediator);

    JsonObject sensorObj = sensors.add<JsonObject>();
    sensorObj["type"] = "Entradas Digitales";
    sensorObj["id"] = "modbus_relay_" + String(r->getAddress());
    sensorObj["icon"] = "🔌";
    sensorObj["error"] = !r->isActive();

    JsonArray readings = sensorObj["readings"].to<JsonArray>();

    JsonObject in1 = readings.add<JsonObject>();
    in1["label"] = "IN 1";
    in1["value"] = String(r->getInputState(0) ? 1 : 0);
    in1["unit"]  = "";
    in1["status"]= r->isActive() ? "ok" : "warn";
    in1["key_device"] = (uint8_t)(ESP.getEfuseMac() & 0xFF);
    in1["key_sensor"] = r->getAddress();
    in1["key_var"] = (uint8_t)SensorVariable::DIGITAL_IN_1;

    JsonObject in2 = readings.add<JsonObject>();
    in2["label"] = "IN 2";
    in2["value"] = String(r->getInputState(1) ? 1 : 0);
    in2["unit"]  = "";
    in2["status"]= r->isActive() ? "ok" : "warn";
    in2["key_device"] = (uint8_t)(ESP.getEfuseMac() & 0xFF);
    in2["key_sensor"] = r->getAddress();
    in2["key_var"] = (uint8_t)SensorVariable::DIGITAL_IN_2;
  }

  // Legacy fields for backward compatibility (optional, but good practice)
  // We can populate them from the first sensor found, or leave empty.
  doc["wifi_status"] = wifiStatus;

  // Add Uptime for Live Data page
  unsigned long uptimeSec = millis() / 1000;
  String uptime = String(uptimeSec / 3600) + "h " +
                  String((uptimeSec % 3600) / 60) + "m " +
                  String(uptimeSec % 60) + "s";
  doc["uptime"] = uptime;

  JsonArray actuators = doc["actuators"].to<JsonArray>();
  for (uint8_t i = 0; i < mediator.getActuatorCount(); i++) {
    IActuator* a = mediator.getActuator(i);
    if (!a) continue;
    JsonObject o = actuators.add<JsonObject>();
    o["id"]    = a->getId();
    o["name"]  = a->getName();
    o["state"] = a->getState();
    o["status"] = a->getStatus()? "ok" : "warn";
    o["key_actuator"] = a->getId();
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleStatus() {
  JsonDocument doc;

  // System
  JsonDocument config = loadConfig();
  doc["device_name"] = config["incubator_name"] | "Unknown";
  doc["device_id"] = config["hash"] | "N/A";
  doc["firmware_version"] = FIRMWARE_VERSION;

  // WiFi
  bool connected = (WiFi.status() == WL_CONNECTED);
  doc["wifi_connected"] = connected;
  if (connected) {
    doc["wifi_ssid"] = WiFi.SSID();
    doc["wifi_ip"] = WiFi.localIP().toString();
    int rssi = WiFi.RSSI();
    doc["wifi_rssi"] = rssi;

    String signalClass = "signal-weak";
    if (rssi > -50)
      signalClass = "signal-excellent";
    else if (rssi > -60)
      signalClass = "signal-good";
    else if (rssi > -70)
      signalClass = "signal-fair";
    doc["signal_class"] = signalClass;
  }

  // Sensors
  int activeSensors = 0;
  int totalSensors = 0;
#ifdef SENSOR_MULTI
  for (auto *s : getSensorList()) {
    totalSensors++;
    if (s && s->isActive())
      activeSensors++;
  }
#else
  totalSensors = 1;
  if (sensor && sensor->isActive())
    activeSensors = 1;
#endif


  doc["active_sensors"] = activeSensors;
  doc["total_sensors"] = totalSensors;
  //actuators
  doc["total_actuators"] = mediator.getActuatorCount();
  doc["total_active_actuators"] = mediator.getActiveActuatorCount();

  // Uptime
  unsigned long uptimeSec = millis() / 1000;
  String uptime =
      String(uptimeSec / 3600) + "h " + String((uptimeSec % 3600) / 60) + "m";
  doc["uptime"] = uptime;

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleHome() {
  // If WiFi not connected, redirect to Portal
  if (WiFi.status() != WL_CONNECTED) {
    server.sendHeader("Location", "/wifi-setup", true);
    server.send(302, "text/plain", "");
    return;
  }
  server.send(200, "text/html", index_html);
}

void handleData() { server.send(200, "text/html", data_html); }

void handleSettings() { server.send(200, "text/html", config_html); }

void handleStyle() { server.send(200, "text/css", style_css); }

void handleConfigJs() { server.send(200, "application/javascript", config_js); }

void handleFavicon() { server.send(200, "image/svg+xml", favicon_svg); }

void handleConfiguracion() {
  JsonDocument doc = loadConfig();

  if (doc.isNull() || doc.size() == 0) {
    server.send(500, "application/json",
                "{\"error\": \"No se pudo cargar config.json\"}");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    doc["current_wifi_channel"] = WiFi.channel();
  } else {
    doc["current_wifi_channel"] = 0;
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void habldePostConfig() {
  if (!requireAdminAuth()) return;
  DBG_INFO("Updating config...\n");

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No JSON data received");
    return;
  }

  String jsonString = server.arg("plain");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    DBG_ERROR("JSON parse error: %s\n", error.c_str());
    server.send(400, "text/plain", "Invalid JSON format");
    return;
  }

  const char *new_ssid = doc["ssid"];
  const char *new_password = doc["passwd"];

  // Validate WiFi password length (WPA2: 8-63 chars). Empty is allowed for
  // open networks or to keep the previously stored value untouched.
  if (new_password) {
    size_t plen = strlen(new_password);
    if (plen > 0 && (plen < 8 || plen > 63)) {
      server.send(400, "text/plain",
                  "wifi password must be 8-63 characters (WPA2) or empty");
      return;
    }
  }

  if (new_ssid && strlen(new_ssid) > 0 && strcmp(new_ssid, "ToChange") != 0) {
    wifiManager.onChange(String(new_ssid), String(new_password));
    DBG_INFO("WiFi updated: %s\n", new_ssid);
  }

  if (updateConfig(doc)) {
    server.send(
        200, "text/plain",
        "Configuration updated successfully. Some changes require restart.");
    DBG_INFO("Config saved\n");
  } else {
    server.send(500, "text/plain", "Failed to save configuration");
    DBG_ERROR("Config save failed\n");
  }
}

void handleSCD30Calibration() {
  if (!requireAdminAuth()) return;
  DBG_VERBOSE("Calibration called: %s\n",
              sensor ? sensor->getSensorType() : "NULL");

  String response = "{";
  int httpStatus = 200;

  ISensor* scd30 = nullptr;
#ifdef SENSOR_MULTI
  for (auto* s : getSensorList()) {
    if (s && s->isActive() && String(s->getSensorType()).equalsIgnoreCase("SCD30")) {
      scd30 = s;
      break;
    }
  }
#else
  if (sensor && sensor->isActive() && String(sensor->getSensorType()).equalsIgnoreCase("SCD30")) {
    scd30 = sensor;
  }
#endif

  if (!scd30) {
    response += "\"status\":\"error\",";
    response += "\"message\":\"SCD30 sensor not found or not active\",";
    response += "\"sensor_detected\":false,";
    response += "\"calibration_performed\":false";
    httpStatus = 503;
  } else {
    bool calibrationSuccess = scd30->calibrate(400);

    if (calibrationSuccess) {
      response += "\"status\":\"success\",";
      response += "\"message\":\"Sensor calibration completed successfully\",";
      response +=
          "\"sensor_type\":\"" + String(scd30->getSensorType()) + "\",";
      response += "\"sensor_detected\":true,";
      response += "\"calibration_performed\":true,";
      response += "\"target_co2\":400,";
      response += "\"note\":\"Allow 2-3 minutes for sensor to stabilize after "
                  "calibration\"";
      DBG_INFO("Calibration OK\n");
    } else {
      response += "\"status\":\"error\",";
      response += "\"message\":\"Calibration not supported or failed for " +
                  String(scd30->getSensorType()) + "\",";
      response +=
          "\"sensor_type\":\"" + String(scd30->getSensorType()) + "\",";
      response += "\"sensor_detected\":true,";
      response += "\"calibration_performed\":false";
      httpStatus = 500;
      DBG_ERROR("Calibration failed: %s\n", scd30->getSensorType());
    }
  }

  response += "}";
  server.send(httpStatus, "application/json", response);
}

void handleRestart() {
  if (!requireAdminAuth()) return;
  server.send(200, "text/plain", "Restarting ESP32...");
  delay(1000);
  ESP.restart();
}

void handleConfigReset() {
  if (!requireAdminAuth()) return;
  DBG_INFO("Resetting config...\n");

  if (SPIFFS.exists(CONFIG_FILE_PATH)) {
    if (SPIFFS.remove(CONFIG_FILE_PATH)) {
      DBG_INFO("Config removed\n");
    } else {
      DBG_ERROR("Remove failed\n");
    }
  }

  createConfigFile();

  JsonDocument doc;
  doc["success"] = true;
  doc["message"] = "Configuration reset to defaults. Restarting...";

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);

  DBG_INFO("Restarting...\n");
  delay(1000);
  ESP.restart();
}

#ifdef ENABLE_ESPNOW
void handleESPNowStatus() {
  JsonDocument doc;

  JsonDocument config = loadConfig();
  bool espnowEnabled = config["espnow_enabled"] | false;
  String forcedMode = config["espnow_force_mode"] | "";
  String actualMode = espnowMgr.getMode();

  doc["enabled"] = espnowEnabled;
  doc["mode"] = actualMode;
  doc["forced_mode"] = forcedMode;
  doc["mac_address"] = espnowMgr.getMACAddress();
  doc["channel"] = (WiFi.status() == WL_CONNECTED) ? WiFi.channel() : 0;

  if (actualMode == "sensor") {
    doc["paired"] = espnowMgr.isPaired();
    doc["peer_count"] = 0;
  } else {
    doc["paired"] = true;
    doc["peer_count"] = espnowMgr.getActivePeerCount();
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}
#endif

void handleRelayList() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (auto *r : relayMgr.getRelays()) {
    if (r) {
      r->syncState();
      JsonDocument tempDoc;
      deserializeJson(tempDoc, r->getStatusJSON());
      arr.add(tempDoc);
    }
  }

  for (auto& g : relayMgr.getGpioRelays()) {
    if (g.actuator) {
      JsonObject obj = arr.add<JsonObject>();
      obj["type"] = "gpio";
      obj["address"] = g.pin;
      obj["alias"] = g.alias;
      JsonArray stateArr = obj["state"].to<JsonArray>();
      stateArr.add(g.actuator->getState());
    }
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleRelayToggle() {
  if (!requireAdminAuth()) return;
  if (!server.hasArg("addr") || !server.hasArg("ch")) {
    server.send(400, "text/plain", "Missing addr or ch param");
    return;
  }

  int addr = server.arg("addr").toInt();
  int ch = server.arg("ch").toInt();

  for (auto *r : relayMgr.getRelays()) {
    if (r->getAddress() == addr) {
      ActuatorCommand cmd;
      cmd.actuatorId = r->getChannel(ch)->getId();
      cmd.state = !r->getChannel(ch)->getState(); // Commuta el estado LÓGICO
      cmd.durationMs = 0;
      cmd.priority = 3;
      mediator.onManualCommand(cmd);
      server.send(200, "text/plain", "OK");
      return;
    }
  }

  for (auto& g : relayMgr.getGpioRelays()) {
    if (g.pin == addr && ch == 0) {
      ActuatorCommand cmd;
      cmd.actuatorId = g.actuator->getId();
      cmd.state = !g.actuator->getState();
      cmd.durationMs = 0;
      cmd.priority = 3;  // Manual toggle priority
      mediator.onManualCommand(cmd);
      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(404, "text/plain", "Relay not found");
}

// ── Mediator / Rules endpoints ────────────────────────────────────────────────

#include "core/RuleLoader.h"

/**
 * POST /actuator/command
 * Body: { "id": <uint>, "state": <bool>, "duration": <ms>, "priority": <1-3> }
 * Sends a manual command to the mediator.
 */
void handleActuatorCommand() {
  if (!requireAdminAuth()) return;
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body required");
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  ActuatorCommand cmd;
  cmd.actuatorId = doc["id"]       | (uint8_t)0;
  cmd.state      = doc["state"]    | false;
  cmd.durationMs = doc["duration"] | (uint32_t)0;
  cmd.priority   = doc["priority"] | (uint8_t)3;   // default: manual

  mediator.onManualCommand(cmd);

  server.send(200, "application/json",
              "{\"ok\":true,\"id\":" + String(cmd.actuatorId) +
              ",\"state\":" + (cmd.state ? "true" : "false") + "}");
}

/**
 * GET /actuator/status
 * Returns JSON array of registered actuator states.
 */
void handleActuatorStatus() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (uint8_t i = 0; i < mediator.getActuatorCount(); i++) {
    IActuator* a = mediator.getActuator(i);
    if (!a) continue;
    JsonObject o = arr.add<JsonObject>();
    o["id"]    = a->getId();
    o["name"]  = a->getName();
    o["state"] = a->getState();
    o["status"] = a->getStatus();
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

/**
 * POST /rules/reload
 * Re-reads /rules.json from SPIFFS and reloads mediator rules.
 */
void handleRulesReload() {
  if (!requireAdminAuth()) return;
  int n = RuleLoader::load(mediator, true);
  if (n < 0) {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"Cannot open rules.json\"}");
    return;
  }
  server.send(200, "application/json",
              "{\"ok\":true,\"rules_loaded\":" + String(n) + "}");
}

/**
 * GET /rules
 * Returns raw content of /rules.json from SPIFFS.
 */
void handleRulesGet() {
  if (!SPIFFS.exists("/rules.json")) {
    server.send(200, "application/json", "{\"rules\":[]}");
    return;
  }
  File f = SPIFFS.open("/rules.json", "r");
  if (!f) {
    server.send(500, "application/json", "{\"error\":\"Cannot open file\"}");
    return;
  }
  server.streamFile(f, "application/json");
  f.close();
}

/**
 * POST /rules/save
 * Receives full rules JSON, writes to /rules.json, reloads mediator.
 */
void handleRulesSave() {
  if (!requireAdminAuth()) return;
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body required");
    return;
  }

  // Validate JSON
  const String& body = server.arg("plain");
  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  // Write to SPIFFS
  File f = SPIFFS.open("/rules.json", "w");
  if (!f) {
    server.send(500, "text/plain", "Cannot write file");
    return;
  }
  f.print(body);
  f.close();

  // Reload mediator
  int n = RuleLoader::load(mediator, true);
  DBG_INFO("[Rules] Saved & reloaded: %d rules\n", n);

  server.send(200, "application/json",
              "{\"ok\":true,\"rules_loaded\":" + String(n) + "}");
}

/**
 * GET /rules-editor
 * Serves the rules editor SPA.
 */
void handleRulesEditor() {
  server.send(200, "text/html", rules_html);
}
// GET /api/admin/info -> { "configured": bool }
// Used by the UI to decide whether to show the "current password" input.
// Public (no auth) so the unconfigured first-boot case can read it.
void handleApiAdminInfo() {
  JsonDocument doc;
  doc["configured"] = secrets.hasAdminPassword();
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

// POST /api/admin/password
// Body: { "current": "...", "new": "..." }
// Rules:
//   - new length in [8, 64]
//   - new must differ from current stored value
//   - if a password is already configured, `current` must match it
//   - if not configured (default state), allow first-set without `current`
void handleApiAdminPassword() {
  if (!requireAdminAuth()) return;

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method not allowed");
    return;
  }

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No body");
    return;
  }

  JsonDocument body;
  DeserializationError err = deserializeJson(body, server.arg("plain"));
  if (err) {
    server.send(400, "text/plain", String("JSON error: ") + err.c_str());
    return;
  }

  String current = body["current"] | "";
  String next = body["new"] | "";

  if (next.length() < 8) {
    server.send(400, "text/plain",
                "new password must be at least 8 characters");
    return;
  }
  if (next.length() > 64) {
    server.send(400, "text/plain",
                "new password must be 64 characters or fewer");
    return;
  }

  if (secrets.hasAdminPassword()) {
    if (current != secrets.getAdminPassword()) {
      server.send(401, "text/plain", "current password mismatch");
      return;
    }
  }

  if (next == secrets.getAdminPassword()) {
    server.send(400, "text/plain", "new password must differ from current");
    return;
  }

  if (!secrets.setAdminPassword(next)) {
    server.send(500, "text/plain", "Failed to persist new password");
    return;
  }

  server.send(200, "text/plain", "Admin password updated");
}
