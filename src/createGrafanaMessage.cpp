#include "createGrafanaMessage.h"
#include <WiFi.h>
#include <constants.h>

String create_grafana_message(float temperature, float humidity, float co2) {
  unsigned long long timestamp = time(nullptr) * 1000000000ULL;

  char device_name[64]={0};
  String mac = WiFi.macAddress(); 
  mac.replace(":", "");            
  snprintf(device_name, sizeof(device_name), "moni-%s", mac.c_str());

  String message = "medicionesCO2,device=" + String(device_name) + 
                   " temp=" + String(temperature, 2) +
                   ",hum=" + String(humidity, 2) + 
                   ",co2=" + String(co2) + 
                   " " + String(timestamp);

  return message;
}
