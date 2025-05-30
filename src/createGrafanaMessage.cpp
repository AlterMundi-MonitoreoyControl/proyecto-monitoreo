#include "createGrafanaMessage.h"
#include <constants.h>
#include "globals.h"

void createGrafanaMessage(char *buffer, size_t bufferSize, float temperature, float humidity, float co2){
  unsigned long long timestamp = static_cast<unsigned long long>(time(nullptr)) * 1000000000ULL;
  snprintf(
      buffer,
      bufferSize,
      "medicionesCO2,device=%s temp=%.2f,hum=%.2f,co2=%.0f %llu",
      deviceName,
      temperature,
      humidity,
      co2,
      timestamp
  );
}
