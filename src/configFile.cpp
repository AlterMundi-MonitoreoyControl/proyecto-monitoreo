#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "configFile.h"
#include "globals.h"
#include "constants.h"



void createConfigFile() {
    
    //SPIFFS.remove(path);

    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      Serial.println("Archivo de configuración ya existe.");
      return;
    }
  
    Serial.println("Creando archivo de configuración...");
  
    File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_WRITE);
    if (!file) {
      Serial.println("Error al abrir config.json para escritura.");
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
  
    if (serializeJsonPretty(config, file) == 0) {
      Serial.println("Error al escribir JSON en archivo.");
    } else {
      Serial.println("Archivo config.json creado correctamente.");
    }
  
    file.close();
  }

  String getConfigFile() {
    File file = SPIFFS.open(CONFIG_FILE_PATH, FILE_READ);
    if (!file || file.isDirectory()) {
        Serial.println("Error al abrir config.json");
        return String();
    }
    String json = file.readString();
    file.close();
    return json;
  }

