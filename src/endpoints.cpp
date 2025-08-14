#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_SCD30.h>
#include <SPIFFS.h>
#include "endpoints.h"
#include "globals.h"
#include "configFile.h"

#include <ArduinoJson.h>

void handleMediciones() {
    float temperature = 99, humidity = 100, co2 = 999999, presion = 99;
    String wifiStatus = "unknown";
    bool rotation = false;
    
    #if defined(MODO_SIMULACION)
        temperature = 22.5 + random(-100, 100) * 0.01;
        humidity = 50 + random(-500, 500) * 0.01;
        co2 = 400 + random(0, 200);
        presion = 850 + random(-100, 100) * 0.01;
        wifiStatus = "disconnected";
        rotation = false;
    #else
        if (scd30.dataReady() && scd30.read()) {
            temperature = scd30.temperature;
            humidity = scd30.relative_humidity;
            co2 = scd30.CO2;
            wifiStatus = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
        }
    #endif

    JsonDocument doc;
    doc["rotation"] = rotation;
    doc["a_pressure"] = String(presion, 2);

    JsonObject errors = doc["errors"].to<JsonObject>();
    errors["rotation"].to<JsonArray>();
    errors["temperature"].to<JsonArray>();
    errors["sensors"].to<JsonArray>();
    errors["humidity"].to<JsonArray>();
    errors["wifi"].to<JsonArray>();

    doc["a_temperature"] = String(temperature, 2);
    doc["a_humidity"] = String(humidity, 2);
    doc["a_co2"] = String(co2, 2); // HAY QUE AÑADIR ESTE A LA APLICACION
    doc["wifi_status"] = wifiStatus;  

    String output;
    serializeJsonPretty(doc, output);
    server.send(200, "application/json", output);
}

void handleData() {
      float temperature = 99, humidity = 100, co2 = 999999, presion = 99;
    String wifiStatus = "unknown";
    bool rotation = false;
    
    #if defined(MODO_SIMULACION)
        temperature = 22.5 + random(-100, 100) * 0.01;
        humidity = 50 + random(-500, 500) * 0.01;
        co2 = 400 + random(0, 200);
        presion = 850 + random(-100, 100) * 0.01;
        wifiStatus = "disconnected";
        rotation = false;
    #else
        if (scd30.dataReady() && scd30.read()) {
            temperature = scd30.temperature;
            humidity = scd30.relative_humidity;
            co2 = scd30.CO2;
            wifiStatus = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
        }
    #endif
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='10'>"; // refresh every 10 seconds
  html += "<title>Sensor Data</title>";
  html += "<style>";
  html += "body{font-family:Arial, sans-serif; text-align:center; background:#f4f4f4;}";
  html += "h1{color:#333;}";
  html += "table{margin:auto; border-collapse:collapse;}";
  html += "td,th{padding:8px 15px; border:1px solid #ccc;}";
  html += "</style></head><body>";
  html += "<h1>SCD30 Sensor Data</h1>";
  html += "<table>";
  html += "<tr><th>Temperature (°C)</th><th>Humidity (%)</th><th>CO₂ (ppm)</th><th>wifi</th></tr>";
  html += "<tr>";
  html += "<td>" + String(temperature, 2) + "</td>";
  html += "<td>" + String(humidity, 2) + "</td>";
  html += "<td>" + String(co2, 2) + "</td>";
  html += "<td>" + wifiStatus + "</td>";
  html += "</tr></table>";
  html += "<p>Last update: " + String(millis() / 1000) + "s since start</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleConfiguracion() {
    String jsonfile = getConfigFile();
    if (jsonfile.isEmpty()) {
    server.send(500, "application/json", "{\"error\": \"No se pudo abrir config.json\"}");
    }
    else
    {
    server.send(200, "application/json", jsonfile);
    }
}

void habldePostConfig() {
    Serial.println("set config ... ");
    
    if (server.hasArg("plain")) {
      String jsonString = server.arg("plain");
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, jsonString);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        server.send(400, "text/plain", "Invalid JSON format");
        return;
      }

      // Extract SSID and password from JSON
      const char* new_ssid = doc["ssid"];
      const char* new_password = doc["passwd"];

      if (new_ssid && strlen(new_ssid) > 0 && strcmp(new_ssid, "ToChange") != 0) {
        wifiManager.onChange(String(new_ssid), String(new_password));
        Serial.printf("New SSID: %s\n, new password %s \n", new_ssid, new_password);
        server.send(200, "text/plain", "Configuration updated. Attempting to connect to " + String(new_ssid));
      } else {
        Serial.printf("SSID is empty in received JSON\n");
        server.send(400, "text/plain", "SSID cannot be empty");
      }
    } else {
      Serial.println("no json");
      server.send(400, "text/plain", "No JSON data received");
    }
  }

void handleSCD30Calibration() {
  Serial.println("Endpoint /calibrate-scd30 called");
  
  String response = "{";
  int httpStatus = 200;
  
  #if defined(MODO_SIMULACION)
      // En modo simulación, simular la respuesta
      response += "\"status\":\"simulated\",";
      response += "\"message\":\"Simulation mode - calibration simulated\",";
      response += "\"sensor_detected\":false,";
      response += "\"calibration_performed\":true,";
      response += "\"target_co2\":400";
  #else
          try {
              // Verificar que el sensor esté disponible
              if (!scd30.begin()) {
                  response += "\"status\":\"error\",";
                  response += "\"message\":\"Failed to communicate with SCD30 sensor\",";
                  response += "\"sensor_detected\":false,";
                  response += "\"calibration_performed\":false";
                  httpStatus = 503;
              } else {
                  Serial.println("SCD30 detected, attempting forced recalibration to 400 ppm...");
                  
                        
                  // Forzar recalibración a 400 ppm
                  bool calibrationSuccess = scd30.forceRecalibrationWithReference(400);
                  delay(100);

                  
                  if (calibrationSuccess) {
                      response += "\"status\":\"success\",";
                      response += "\"message\":\"SCD30 calibration completed successfully\",";
                      response += "\"sensor_detected\":true,";
                      response += "\"calibration_performed\":true,";
                      response += "\"target_co2\":400,";
                      response += "\"note\":\"Allow 2-3 minutes for sensor to stabilize after calibration\"";
                      Serial.println("SCD30 calibration successful!");
                  } else {
                      response += "\"status\":\"error\",";
                      response += "\"message\":\"SCD30 calibration failed - sensor may be busy or in error state\",";
                      response += "\"sensor_detected\":true,";
                      response += "\"calibration_performed\":false";
                      httpStatus = 500; // Internal Server Error
                      Serial.println("SCD30 calibration failed!");
                  }
              }
          } catch (...) {
              response += "\"status\":\"error\",";
              response += "\"message\":\"Exception occurred during calibration\",";
              response += "\"sensor_detected\":true,";
              response += "\"calibration_performed\":false";
              httpStatus = 500;
              Serial.println("Exception during SCD30 calibration");
          }
      
  #endif
  
  response += "}";
  
  server.send(httpStatus, "application/json", response);
  Serial.println("Calibration response sent: " + response);
}