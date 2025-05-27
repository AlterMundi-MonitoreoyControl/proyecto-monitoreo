#include "createGrafanaMessage.h"
#include <cstdio>


void createGrafanaMessage(char* buffer, size_t bufferSize, const char* device, float temperature, float humidity, float co2, unsigned long long timestamp) {
    snprintf(buffer, bufferSize,
             "medicionesCO2,device=%s temp=%.2f,hum=%.2f,co2=%.2f %llu",
             device, temperature, humidity, co2, timestamp);
}
