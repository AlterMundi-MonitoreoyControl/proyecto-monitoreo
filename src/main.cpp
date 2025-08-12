#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SCD30.h>
#include <time.h>
#include <WiFiManager.h> 
#include <HTTPUpdate.h>  
#include <WiFiClientSecure.h>  
#include <esp_ota_ops.h>
#include "version.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "sendDataGrafana.h"
#include "createGrafanaMessage.h"
#include "constants.h"
#include "globals.h"
#include "endpoints.h"
#include "configFile.h"
#include "otaUpdater.h"

unsigned long lastUpdateCheck = 0;
unsigned long lastSendTime = 0;
bool sensorActivo = false;

#ifndef UNIT_TEST

void setup() {
  Serial.begin(115200);
  
  
  #if defined(MODO_SIMULACION)
    Serial.print("Dirección IP asignada: ");  
    Serial.println(WiFi.localIP());   
  #endif

  Serial.println("Conectado a WiFi");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  if (!SPIFFS.begin(true)) { 
    Serial.println("Error montando SPIFFS");
  }

  createConfigFile(); 

  #if !defined(MODO_SIMULACION)
    sensorActivo = scd30.begin(); 
    if (!sensorActivo) {  
      Serial.println("No se pudo inicializar el sensor SCD30!");
    }
  #endif

  clientSecure.setInsecure(); 

  server.on("/actual", HTTP_GET, handleMediciones);
  server.on("/config", HTTP_GET, handleConfiguracion);
  server.on("/config", HTTP_POST, habldePostConfig);
  server.on("/data", HTTP_GET,handleData);
    // Add handler for undefined routes
  server.onNotFound([]() {
      // Redirect all undefined routes to root page
      Serial.println("Redirecting to root page for undefined route");
      Serial.printf("Requested route: %s\n", server.uri().c_str());
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "");
  });



  server.enableCORS(true);

  // Optional: Configure timeouts and retries
  wifiManager.setConnectionTimeout(15000);  // 15 seconds
  wifiManager.setMaxRetries(8);
  wifiManager.setValidationTimeout(30000);  // 30 seconds
  wifiManager.init(&server);
    
  Serial.println("Setup complete!");
  Serial.println("Access Point: " + wifiManager.getAPSSID());
  Serial.println("Connect to the AP and go to http://192.168.16.10 to configure WiFi");

  server.begin();
  Serial.println("Servidor web iniciado en el puerto 80");
    
}

void loop() {
  wifiManager.update();
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 30000) {  // Print status every 30 seconds
      lastStatusPrint = millis();
      
      if (wifiManager.isOnline()) {
          Serial.println("WiFi Status: Connected to " + wifiManager.getCurrentSSID());
          Serial.println("IP Address: " + wifiManager.getLocalIP().toString());
      } else {
          Serial.println("WiFi Status: Disconnected - AP available at " + wifiManager.getAPSSID());
      }
  }
  server.handleClient();

  unsigned long currentMillis = millis();

  //// 1. Verificamos si hay que chequear actualizaciones
  if (currentMillis - lastUpdateCheck >= UPDATE_INTERVAL) {
    Serial.printf("Free heap before checking: %d bytes\n", ESP.getFreeHeap());
    checkForUpdates();
    Serial.printf("Free heap after checking: %d bytes\n", ESP.getFreeHeap());
    lastUpdateCheck = currentMillis;
  }

  //// 2. Enviamos datos a Grafana cada 10 segundos
  if (currentMillis - lastSendTime >= 10000) {
    lastSendTime = currentMillis;

    float temperature = 99, humidity = 100, co2 = 999999;

    #if defined(MODO_SIMULACION)
      // Datos simulados
      temperature = 22.5 + random(-100, 100) * 0.01;
      humidity = 50 + random(-500, 500) * 0.01;
      co2 = 400 + random(0, 200);      
      Serial.println("Enviando datos simulados...");
    #else
      if (sensorActivo && scd30.dataReady()) { 
        if (!scd30.read()) {
          Serial.println("Error leyendo el sensor!");
          return;
        }
        temperature = scd30.temperature;
        humidity = scd30.relative_humidity;
        co2 = scd30.CO2;
      } else {
        Serial.println("Sensor no listo, esperando..."); 
      }
    #endif

    Serial.printf("Free heap before sending: %d bytes\n", ESP.getFreeHeap());
    sendDataGrafana(temperature, humidity, co2);
    Serial.printf("Free heap after sending: %d bytes\n", ESP.getFreeHeap());
  }
  delay(10);
}

#endif  // UNIT_TEST