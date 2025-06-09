#include "setCallbacks.h"
#include "globals.h"
#include "handles.h"


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Abriendo portal de configuracion");
  Serial.println(WiFi.softAPIP()); // muestra la IP del portal
  Serial.println(myWiFiManager->getConfigPortalSSID()); // SSID del AP
}

// se llama Justo después de que se crea el servidor web (internamente es un WebServer o AsyncWebServer), antes de que se empiece a usar.
// se usa para agregar endpoints personalizados al servidor web:
//void bindServerCallback(){
//  wifiManager.server->on("/custom",handleRoute);
//  // you can override wifiManager route endpoints, I have not found a way to remove handlers, but this would let you disable them or add auth etc.
//  // wifiManager.server->on("/info",handleNotFound);
//  // wifiManager.server->on("/update",handleNotFound);
//  wifiManager.server->on("/erase",handleNotFound); // disable erase
//}

// se llama Cuando el usuario presiona “Guardar” y WiFiManager guarda los datos de la red WiFi (SSID y password).
void saveWifiCallback(){
  Serial.println("Se guardó nueva configuración de WiFi.");
}

// se llama Cuando se guardan los parámetros personalizados agregados con addParameter(...) en configParamsAndCallbacks.cpp
void saveParamCallback(){
  Serial.println("Guardando parámetros:");
}

// se llama Antes de iniciar una actualización OTA (Over The Air), si usás la funcionalidad de OTA integrada en WiFiManager.
void handlePreOtaUpdateCallback(){
  //Update.onProgress([](unsigned int progress, unsigned int total) {
  //      Serial.printf("CUSTOM Progress: %u%%\r", (progress / (total / 100)));
  //});
  Serial.println("Preparando para OTA...");
  // Cerrar conexiones, guardar estado, etc.  
}