#ifndef SEND_DATA_GRAFANA_H
#define SEND_DATA_GRAFANA_H

#include <Arduino.h>

void sendDataGrafana(float temperature, float humidity, float co2, const char* sensorId= "Unknown", const char* deviceId = "Unknown");
void sendDataGrafana(const char* message, const char* sensorId= "Unknown", const char* deviceId = "Unknown");

#endif // SEND_DATA_GRAFANA_H