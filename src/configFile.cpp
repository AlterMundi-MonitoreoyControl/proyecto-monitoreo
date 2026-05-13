#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "configFile.h"
#include "globals.h"
#include "constants.h"
#include "debug.h"



void createConfigFile() {
    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      DBG_VERBOSE("Config exists\n");
      return;
    }

    DBG_INFO("Creating config...\n");

    File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_WRITE);
    if (!file) {
      DBG_ERROR("Open config.json failed\n");
      return;
    }
  
    JsonDocument config;
    config["max_temperature"] = 37.7;
    config["min_temperature"] = 37.3;
    config["rotation_duration"] = 50000;
    config["rotation_period"] = 3600000;
    config["ssid"] = "ToChange";
    config["passwd"] = "ToChange";
    config["tray_one_date"] = 0;
    config["tray_two_date"] = 0;
    config["tray_three_date"] = 0;
    config["incubation_period"] = 18;
    config["max_hum"] = 65;
    config["min_hum"] = 55;

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    config["hash"] = mac;
    config["incubator_name"] = "moni-" + mac; // en algun momento se debe a cambiar por el nombre del incubador por moni

    // Configuración de sensores (para modo multi-sensor)
    JsonArray sensors = config["sensors"].to<JsonArray>();

    JsonObject cap1 = sensors.add<JsonObject>();
    cap1["type"] = "capacitive";
    cap1["enabled"] = true;
    JsonObject cap1Cfg = cap1["config"].to<JsonObject>();
    cap1Cfg["pin"] = 34;
    cap1Cfg["name"] = "Soil1";

    JsonObject onewire = sensors.add<JsonObject>();
    onewire["type"] = "onewire";
    onewire["enabled"] = false;  // Disabled by default
    JsonObject onewireCfg = onewire["config"].to<JsonObject>();
    onewireCfg["pin"] = 4;
    onewireCfg["scan"] = true;

    // Configuración de Relays
    JsonArray relays = config["relays"].to<JsonArray>();
    JsonObject r1 = relays.add<JsonObject>();
    r1["type"] = "relay_2ch";
    r1["enabled"] = false;
    JsonObject r1c = r1["config"].to<JsonObject>();
    r1c["address"] = 1;
    r1c["alias"] = "Relay 01";

    // Configuración RS485 (bus compartido para sensores Modbus, relés, etc.)
    JsonObject rs485 = config["rs485"].to<JsonObject>();
    rs485["enabled"] = false;
    rs485["rx_pin"] = 16;
    rs485["tx_pin"] = 17;
    rs485["de_pin"] = 18;
    rs485["baudrate"] = 9600;
    rs485["raw_send_enabled"] = false;  // Enviar datos crudos por serial

    // Configuración ESP-NOW (auto-adaptativo)
    config["espnow_enabled"] = false;
    config["espnow_force_mode"] = "";  // "" = auto-detect, "gateway" o "sensor" = force mode
    config["espnow_channel"] = 1;      // WiFi channel (1-13)
    config["beacon_interval_ms"] = 2000;
    config["discovery_timeout_ms"] = 15000;
    config["send_interval_ms"] = 30000;
    config["grafana_ping_url"] = "http://192.168.1.1/ping";  // URL for connectivity test

    if (serializeJsonPretty(config, file) == 0) {
      DBG_ERROR("Write JSON failed\n");
    } else {
      DBG_INFO("Config created\n");
    }

    file.close();
  }

String getConfigFile() {
  File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_READ);
  if (!file || file.isDirectory()) {
      DBG_ERROR("Open config failed\n");
      return String();
  }
  String json = file.readString();
  file.close();
  return json;
}

JsonDocument loadConfig() {
  JsonDocument doc;

  File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_READ);
  if (!file || file.isDirectory()) {
      DBG_ERROR("Open config failed\n");
      return doc;
  }

  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
      DBG_ERROR("JSON error: %s\n", error.c_str());
      return doc;
  }

  bool configModified = false;

  // Automatic migration: add sensors array if missing
  if (!doc["sensors"].is<JsonArray>() || doc["sensors"].size() == 0) {
      DBG_INFO("Migrating: adding sensors\n");

      JsonArray sensors = doc["sensors"].to<JsonArray>();

      // Add SCD30 as default (enabled)
      JsonObject scd30 = sensors.add<JsonObject>();
      scd30["type"] = "scd30";
      scd30["enabled"] = true;
      scd30["config"].to<JsonObject>();

      // Add other sensors (disabled by default)
      JsonObject bme280 = sensors.add<JsonObject>();
      bme280["type"] = "bme280";
      bme280["enabled"] = false;
      bme280["config"].to<JsonObject>();

      JsonObject cap = sensors.add<JsonObject>();
      cap["type"] = "capacitive";
      cap["enabled"] = false;
      JsonObject capCfg = cap["config"].to<JsonObject>();
      capCfg["pin"] = 34;
      capCfg["name"] = "Soil1";

      JsonObject onewire = sensors.add<JsonObject>();
      onewire["type"] = "onewire";
      onewire["enabled"] = false;
      JsonObject onewireCfg = onewire["config"].to<JsonObject>();
      onewireCfg["pin"] = 4;
      onewireCfg["scan"] = true;
      
      configModified = true;
  }

  // Automatic migration: add relays array if missing
  if (!doc["relays"].is<JsonArray>()) {
      DBG_INFO("Migrating: adding relays\n");

      JsonArray relays = doc["relays"].to<JsonArray>();
      JsonObject r1 = relays.add<JsonObject>();
      r1["type"] = "relay_2ch";
      r1["enabled"] = false;
      JsonObject r1c = r1["config"].to<JsonObject>();
      r1c["address"] = 1;
      r1c["alias"] = "Relay 01";

      configModified = true;
  }

  // Automatic migration: convert flat rs485_* fields to nested rs485 object
  if (!doc["rs485"].is<JsonObject>() && !doc["rs485_enabled"].isNull()) {
      DBG_INFO("Migrating: RS485 format\n");

      JsonObject rs485 = doc["rs485"].to<JsonObject>();
      rs485["enabled"] = doc["rs485_enabled"] | false;
      rs485["rx_pin"] = doc["rs485_rx"] | 16;
      rs485["tx_pin"] = doc["rs485_tx"] | 17;
      rs485["de_pin"] = doc["rs485_de"] | 18;
      rs485["baudrate"] = doc["rs485_baud"] | 9600;
      rs485["raw_send_enabled"] = false;  // New field, default off

      // Remove old flat fields
      doc.remove("rs485_enabled");
      doc.remove("rs485_rx");
      doc.remove("rs485_tx");
      doc.remove("rs485_de");
      doc.remove("rs485_baud");

      // Also remove per-sensor RS485 config from modbus_th sensors (use global)
      if (doc["sensors"].is<JsonArray>()) {
          for (JsonObject sensor : doc["sensors"].as<JsonArray>()) {
              if (strcmp(sensor["type"] | "", "modbus_th") == 0) {
                  JsonObject cfg = sensor["config"];
                  if (cfg) {
                      cfg.remove("rx_pin");
                      cfg.remove("tx_pin");
                      cfg.remove("de_pin");
                      cfg.remove("baudrate");
                  }
              }
          }
      }

      configModified = true;
  }

  // Automatic migration: add rs485 object if completely missing
  if (!doc["rs485"].is<JsonObject>()) {
      DBG_INFO("Migrating: adding RS485\n");

      JsonObject rs485 = doc["rs485"].to<JsonObject>();
      rs485["enabled"] = false;
      rs485["rx_pin"] = 16;
      rs485["tx_pin"] = 17;
      rs485["de_pin"] = 18;
      rs485["baudrate"] = 9600;
      rs485["raw_send_enabled"] = false;

      configModified = true;
  }

  // Save migrated config if modified
  if (configModified) {
      DBG_INFO("Saving migrated config...\n");
      File outFile = SPIFFS.open(CONFIG_FILE_PATH, FILE_WRITE);
      if (outFile) {
          serializeJsonPretty(doc, outFile);
          outFile.close();
          DBG_INFO("Migration saved\n");
      }
  }

  return doc;
}

bool updateConfig(JsonDocument& newConfig) {
  if (SPIFFS.exists(CONFIG_FILE_PATH)) {
    SPIFFS.remove(CONFIG_FILE_PATH);
  }

  File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_WRITE);
  if (!file) {
    DBG_ERROR("Open config failed\n");
    return false;
  }

  if (serializeJsonPretty(newConfig, file) == 0) {
    DBG_ERROR("Write failed\n");
    file.close();
    return false;
  }

  file.close();
  DBG_INFO("Config updated\n");
  return true;
}
