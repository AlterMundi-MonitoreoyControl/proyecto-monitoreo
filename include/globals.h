#ifndef GLOBALS_H
#define GLOBALS_H

#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "sensors/ISensor.h"
#include "secret_manager.h"

extern WebServer server;
extern ISensor* sensor;
extern WiFiManager wifiManager;
extern WiFiClientSecure clientSecure;
extern WiFiClient client;
extern HTTPClient http;
extern SecretManager secrets;

#endif // GLOBALS_H