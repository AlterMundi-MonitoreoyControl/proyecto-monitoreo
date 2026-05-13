#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include "globals.h"

WebServer server(80);
ISensor* sensor = nullptr;
WiFiManager wifiManager;
WiFiClientSecure clientSecure;
WiFiClient client;
SecretManager secrets;