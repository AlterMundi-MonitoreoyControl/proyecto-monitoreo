#include "createGrafanaMessage.h"
#include <WiFi.h>
#include <constants.h>

/**
 * Helper function to build the device name from deviceId
 * If deviceId is a mesh sensor (starts with "mesh_"), use it directly
 * Otherwise, use the gateway MAC address
 */
static void buildDeviceName(char* device_name, size_t size, const char* deviceId) {
  if (deviceId && strncmp(deviceId, "moni-", 5) == 0) {
    snprintf(device_name, size, "%s", deviceId);
    
  } else {
    // For local sensors, use gateway MAC
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    snprintf(device_name, size, "moni-%s", mac.c_str());
  }
}

/**
 * Helper function to build the InfluxDB line protocol message
 * Returns: "medicionesCO2,device=<device>,sensor=<sensor> <fields> <timestamp>"
 */
static String buildInfluxMessage(const char* device_name, const char* sensorId, const char* fields, unsigned long long timestamp) {
  return "medicionesCO2,device=" + String(device_name) + 
         ",sensor=" + String(sensorId) + 
         " " + String(fields) + 
         " " + String(timestamp);
}

String create_grafana_message(float temperature, float humidity, float co2, const char* sensorId, const char* deviceId)
{
  unsigned long long timestamp = time(nullptr) * 1000000000ULL;
  char device_name[64] = {0};
  buildDeviceName(device_name, sizeof(device_name), deviceId);

  // Build fields string: "temp=X,hum=Y,co2=Z"
  String fields = "temp=" + String(temperature, 2) +
                  ",hum=" + String(humidity, 2) +
                  ",co2=" + String(co2);

  return buildInfluxMessage(device_name, sensorId, fields.c_str(), timestamp);
}

/**
 * Overload that accepts a pre-formatted fields string
 * The message parameter should already be formatted as: field1=value1,field2=value2
 */
String create_grafana_message(const char* message, const char* sensorId, const char* deviceId)
{
  unsigned long long timestamp = time(nullptr) * 1000000000ULL;
  char device_name[64] = {0};
  buildDeviceName(device_name, sizeof(device_name), deviceId);

  return buildInfluxMessage(device_name, sensorId, message, timestamp);
}