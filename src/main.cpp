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
bool enModoLocal = false;

#ifndef UNIT_TEST

void setup() {
  Serial.begin(115200);

  #if defined(MODO_SIMULACION)
    delay(2000); 
  #endif

  WiFi.mode(WIFI_AP_STA);

  // Intentar conectarse a red WiFi guardada
  WiFi.begin();
  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 10000; // 10 segundos

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado a WiFi");
    enModoLocal = false;
  } else {
    Serial.println("No se pudo conectar a WiFi, funcionando en modo local");
    enModoLocal = true;
  }

  WiFi.softAP("ESP32-SENSOR", "12345678"); // nombre y contraseña del AP

  #if defined(MODO_SIMULACION)
    if(!enModoLocal){
      Serial.print("STA IP: ");
      Serial.println(WiFi.localIP());
    }
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP()); // 192.168.4.1
  #endif

  if (!enModoLocal) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  }

  if (!SPIFFS.begin(true)) { 
    Serial.println("Error montando SPIFFS");
  }

  createConfigFile(); 

  String mac = WiFi.macAddress(); 
  mac.replace(":", "");            
  snprintf(deviceName, sizeof(deviceName), "moni-%s", mac.c_str());

  #if !defined(MODO_SIMULACION)
    sensorActivo = scd30.begin(); 
    if (!sensorActivo) {  
      Serial.println("No se pudo inicializar el sensor SCD30!");
    }
  #endif

  clientSecure.setInsecure(); 

  server.on("/actual", HTTP_GET, handleMediciones);
  server.on("/config", HTTP_GET, handleConfiguracion);
  server.begin();
  
  Serial.println("Servidor web iniciado en el puerto 80");
    
}

void loop() {
  server.handleClient();

  // Detectar si el WiFi se desconectó luego de haber estado conectado
  if (!enModoLocal && WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Cambiando a modo local.");
    enModoLocal = true;
  }

  unsigned long currentMillis = millis();

  //// 1. Verificamos si hay que chequear actualizaciones
  if (!enModoLocal && currentMillis - lastUpdateCheck >= UPDATE_INTERVAL) {
    Serial.printf("Free heap before checking: %d bytes\n", ESP.getFreeHeap());
    checkForUpdates();
    Serial.printf("Free heap after checking: %d bytes\n", ESP.getFreeHeap());
    lastUpdateCheck = currentMillis;
  }

  //// 2. Enviamos datos a Grafana cada 10 segundos si no estamos en modo local
  if(!enModoLocal) {
    if (currentMillis - lastSendTime >= 10000) {
      lastSendTime = currentMillis;

      float temperature = 99, humidity = 100, co2 = 999999;

      uint32_t uptime = millis() / 1000;

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
      uint32_t heap = ESP.getFreeHeap();
      Serial.printf("Free heap before sending: %d bytes\n", heap);
      sendDataGrafana(temperature, humidity, co2, heap, uptime);
      Serial.printf("Free heap after sending: %d bytes\n", ESP.getFreeHeap());
          
    }    
  }
}

#endif  // UNIT_TEST