#include "createGrafanaMessage.h"
#include <Arduino.h>
#include <cstdio>

String create_grafana_message(float temperature, float humidity, float co2, const char* sensorId, const char* deviceId) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "medicionesCO2,device=%s temp=%.2f,hum=%.2f,co2=%.2f %s",
             deviceId, temperature, humidity, co2, sensorId);
    return String(buffer);
}
