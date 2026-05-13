#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <ArduinoJson.h>

void createConfigFile();
String getConfigFile();
JsonDocument loadConfig();  // Nueva función para leer config como JsonDocument
bool updateConfig(JsonDocument& newConfig);  // Actualizar configuración en SPIFFS

#endif // CONFIGFILE_H