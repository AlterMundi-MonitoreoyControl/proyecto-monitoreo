#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>  
#include "constants.h"
#include "globals.h"
#include "sendDataGrafana.h"
#include "createGrafanaMessage.h"

void sendDataGrafana(float temperature, float humidity, float co2, uint32_t heap, uint32_t uptime) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient localHttp;  

        localHttp.begin(client, URL);
        localHttp.setTimeout(2000); 
        localHttp.addHeader("Content-Type", "text/plain");
        char authHeader[128];
        snprintf(authHeader, sizeof(authHeader), "Basic %s", TOKEN_GRAFANA);
        localHttp.addHeader("Authorization", authHeader);        

        static char data[128];
        createGrafanaMessage(data, sizeof(data), temperature, humidity, co2, heap, uptime);
        
        int httpResponseCode = localHttp.POST(data);
        if (httpResponseCode == 204) {
            Serial.println("Datos enviados correctamente");
        } else {
            Serial.printf("Error en el envío: %d\n", httpResponseCode);
            Serial.println(localHttp.getString());
        }

        localHttp.end();  
    } else {
        Serial.println("Error en la conexión WiFi");
    }
}