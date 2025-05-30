#ifndef GLOBALS_H
#define GLOBALS_H

#include <WebServer.h>
#include <Adafruit_SCD30.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

extern WebServer server;
extern Adafruit_SCD30 scd30;
extern WiFiManager wifiManager;
extern WiFiClientSecure clientSecure;
extern WiFiClient client;
extern HTTPClient http;
extern char deviceName[32];

#endif // GLOBALS_H