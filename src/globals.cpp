#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Adafruit_SCD30.h>
#include <WiFiManager.h>
#include "globals.h"

//WebServer server(80);
WebServer server(8080);
Adafruit_SCD30 scd30;
//WiFiManager wifiManager(&server);
WiFiManager wifiManager;
WiFiClientSecure clientSecure;
WiFiClient client;
char deviceName[32] = "";