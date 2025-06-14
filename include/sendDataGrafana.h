#ifndef SEND_DATA_GRAFANA_H
#define SEND_DATA_GRAFANA_H

#include <Arduino.h>

void sendDataGrafana(float temperature, float humidity, float co2, uint32_t heap, uint32_t uptime);

#endif // SEND_DATA_GRAFANA_H