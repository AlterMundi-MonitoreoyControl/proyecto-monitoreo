#include "handles.h"
#include "globals.h"

void handleRoute(){
  Serial.println("[HTTP] handle custom route");
  wifiManager.server->send(200, "text/plain", "hello from user code");
}

void handleNotFound(){
  Serial.println("[HTTP] override handle route");
  wifiManager.handleNotFound();
}