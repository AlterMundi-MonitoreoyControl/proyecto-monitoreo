#ifndef CREATE_GRAFANA_MESSAGE_H
#define CREATE_GRAFANA_MESSAGE_H

#include <cstddef>

void createGrafanaMessage(char *buffer, size_t bufferSize, float temperature, float humidity, float co2);

#endif // CREATE_GRAFANA_MESSAGE_H