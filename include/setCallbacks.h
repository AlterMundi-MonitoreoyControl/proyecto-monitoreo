#ifndef SETCALLBACKS_H
#define SETCALLBACKS_H

#include <WiFiManager.h>

void saveWifiCallback();

void configModeCallback (WiFiManager *myWiFiManager);

void saveParamCallback();

void bindServerCallback();

void handlePreOtaUpdateCallback();

#endif // SETCALLBACKS_H