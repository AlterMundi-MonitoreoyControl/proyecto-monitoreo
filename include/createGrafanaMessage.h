#ifndef CREATE_GRAFANA_MESSAGE_H
#define CREATE_GRAFANA_MESSAGE_H

#include <cstdint>
#include <cstddef>

void createGrafanaMessage(char *buffer, size_t bufferSize, float temperature, float humidity, float co2, uint32_t heap, uint32_t uptime);

#endif // CREATE_GRAFANA_MESSAGE_H