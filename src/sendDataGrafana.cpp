#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "constants.h"
#include "globals.h"
#include "sendDataGrafana.h"
#include "createGrafanaMessage.h"
#include "debug.h"



void sendDataGrafana(float temperature, float humidity, float co2, const char* sensorId, const char* deviceId)  {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient localHttp;

        localHttp.begin(client, URL);
        localHttp.setTimeout(5000);
        localHttp.addHeader("Content-Type", "text/plain");
        localHttp.addHeader("Authorization", "Basic " + String(TOKEN_GRAFANA));

        String data = create_grafana_message(temperature, humidity, co2, sensorId, deviceId);

        DBG_VERBOSE("Grafana: %s\n", data.c_str());

        int httpResponseCode = localHttp.POST(data);
        if (httpResponseCode != 204) {
            DBG_ERROR("Grafana error: %d\n", httpResponseCode);
        }

        localHttp.end();
    } else {
        DBG_ERROR("WiFi disconnected\n");
    }
}

void sendDataGrafana(const char* message, const char* sensorId, const char* deviceId) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient localHttp;

        localHttp.begin(client, URL);
        localHttp.setTimeout(5000);
        localHttp.addHeader("Content-Type", "text/plain");
        localHttp.addHeader("Authorization", "Basic " + String(TOKEN_GRAFANA));

        String data = create_grafana_message(message, sensorId, deviceId);

        DBG_VERBOSE("Grafana: %s\n", data.c_str());

        int httpResponseCode = localHttp.POST(data);
        if (httpResponseCode != 204) {
            DBG_ERROR("Grafana error: %d\n", httpResponseCode);
        }

        localHttp.end();
    } else {
        DBG_ERROR("WiFi disconnected\n");
    }
}