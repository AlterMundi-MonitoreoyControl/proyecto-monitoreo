#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_SCD30.h>
#include <SPIFFS.h>
#include "endpoints.h"
#include "globals.h"
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
    wifiManager.server->send(200, "application/json", output);
}

  
void handleConfiguracion() {
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (!file || file.isDirectory()) {
    wifiManager.server->send(500, "application/json", "{\"error\": \"No se pudo abrir config.json\"}");
    return;
    }

    String json = file.readString(); 
    file.close();

    wifiManager.server->send(200, "application/json", json);
}