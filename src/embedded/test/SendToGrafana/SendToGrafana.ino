#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SCD30.h>
#include <time.h>
#include <WiFiManager.h> 
#include <HTTPUpdate.h>  
#include <WiFiClientSecure.h>  
#include <esp_ota_ops.h>
#include "version.h"

const char* url = "http://grafana.altermundi.net:8086/write?db=cto";
const char* INICIALES = "ASC02";
const char* token_grafana = "token:e98697797a6a592e6c886277041e6b95";
const char* FIRMWARE_BIN_URL = "http://192.168.0.106:8080/bins/SendToGrafana.ino.bin";
const char* YOUR_GITHUB_USERNAME = "AlterMundi-MonitoreoyControl";
const char* YOUR_REPO_NAME = "proyecto-monitoreo";
const unsigned long UPDATE_INTERVAL = 3600000; // Check updates every 1 hour

Adafruit_SCD30 scd30;
WiFiManager wifiManager; 
WiFiClientSecure clientSecure;  
WiFiClient client;
HTTPClient http;

char device_name[64];
unsigned long lastUpdateCheck = 0;

void setup() {
  Serial.begin(115200);
  wifiManager.autoConnect("ESP32-AP"); 

  Serial.println("Conectado a WiFi");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  if (!scd30.begin()) {
    Serial.println("No se pudo inicializar el sensor SCD30!");
  }

  clientSecure.setInsecure();

  String mac = WiFi.macAddress(); 
  mac.replace(":", "");           
  snprintf(device_name, sizeof(device_name), "%smoni-%s", INICIALES, mac.c_str());

  }

String getLatestReleaseTag(const char* repoOwner, const char* repoName) {
 
  String apiUrl = "https://api.github.com/repos/" + String(repoOwner) + "/" + String(repoName) + "/releases/latest";
  Serial.println("API URL: " + apiUrl);

  if (http.begin(clientSecure, apiUrl)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();  

      // Manually find "tag_name": "..."
      int tagPos = payload.indexOf("\"tag_name\":");
      if (tagPos != -1) {
        int startQuote = payload.indexOf("\"", tagPos + 11);  // Skip `"tag_name":`
        int endQuote = payload.indexOf("\"", startQuote + 1);
        if (startQuote != -1 && endQuote != -1) {
          return payload.substring(startQuote + 1, endQuote);
        }
      }
      
      Serial.println("Tag not found in JSON!");
    } else {
      Serial.printf("HTTP GET failed, error: %d\n", httpCode);
    }

    
  } else {
    Serial.println("Unable to connect to GitHub API.");
  }
  http.end();  
  return "";
}
void checkForUpdates() {
  String latestTag = getLatestReleaseTag(YOUR_GITHUB_USERNAME, YOUR_REPO_NAME);
  Serial.printf("Current version: %s, Available version: %s\n", FIRMWARE_VERSION, latestTag.c_str());

  if (latestTag != "") {
    if (latestTag != FIRMWARE_VERSION) {
      const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);
      
      String firmwareURL = "https://github.com/" + String(YOUR_GITHUB_USERNAME) + "/" + String(YOUR_REPO_NAME) + "/releases/download/" + latestTag + "/SendToGrafana.ino.bin";
      Serial.println("Firmware URL: " + firmwareURL);

      // Create the HTTP client to follow redirects
      HTTPClient redirectHttp;
      redirectHttp.begin(clientSecure, firmwareURL);  // Start the initial request
    
      const char *headerKeys[] = {"Location"};
      const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
      redirectHttp.collectHeaders(headerKeys, headerKeysCount);

      int redirectCode = redirectHttp.GET();  // Perform GET request
      if (redirectCode == HTTP_CODE_FOUND || redirectCode == HTTP_CODE_MOVED_PERMANENTLY) {
        Serial.println("Redirect found, HTTP code: " + String(redirectCode));        
        Serial.println(redirectHttp.getString());
        Serial.println(redirectHttp.headers());
        Serial.println(redirectHttp.header("Location"));
        // Now HTTPClient automatically handles redirection
        String redirectedURL = redirectHttp.header("Location");
        if (redirectedURL.length() > 0) {
          Serial.println("Redirected to: " + redirectedURL);
          firmwareURL = redirectedURL;  // Update firmware URL to the redirected one
        } else {
          Serial.println("No 'Location' header found in redirect response.");
        }
        // Proceed to update using the final URL
        HTTPUpdate httpUpdate;

        httpUpdate.onStart([](){ Serial.println("Update started..."); });
        httpUpdate.onEnd([](){ Serial.println("Update finished!"); });
        httpUpdate.onProgress([](int cur, int total) { Serial.printf("Update progress: %d/%d\n", cur, total); });
        httpUpdate.onError([](int err) { Serial.printf("Update error: %d\n", err); });

        t_httpUpdate_return ret = httpUpdate.update(clientSecure, firmwareURL);

        if (ret == HTTP_UPDATE_OK) {
          Serial.println("Update successful!");
          esp_ota_set_boot_partition(update_partition);
        } else {
          Serial.printf("Update failed: %d\n", httpUpdate.getLastError());
        }

      } else {
        Serial.printf("Error following redirect. HTTP code: %d\n", redirectCode);
      }
      // Always end the HTTP connection
      redirectHttp.end();
    } else {
      Serial.println("Firmware is up to date.");
    }
  } else {
    Serial.println("Unable to check for updates... empty release tag.");
  }
}

String create_grafana_message(float temperature, float humidity, float co2) {
  unsigned long long timestamp = time(nullptr) * 1000000000ULL;

  String message = "medicionesCO2,device=" + String(device_name) + 
                   " temp=" + String(temperature, 2) +
                   ",hum=" + String(humidity, 2) + 
                   ",co2=" + String(co2) + 
                   " " + String(timestamp);

  return message;
}


void send_data_grafana(float temperature, float humidity, float co2) {
  if (WiFi.status() == WL_CONNECTED) {
  http.begin(client, url);
  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Authorization", "Basic " + String(token_grafana));
  String data = create_grafana_message(temperature, humidity, co2);


    int httpResponseCode = http.POST(data);

    if (httpResponseCode == 204) {
      Serial.println("Datos enviados correctamente");
    } else {
      Serial.printf("Error en el envío: %d\n", httpResponseCode);
    }

    http.end();  
  } else {
    Serial.println("Error en la conexión WiFi");
  }
}

void loop() {

  // Check for updates periodically instead of every loop
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdateCheck >= UPDATE_INTERVAL) {
    Serial.printf("Free heap before checking: %d bytes\n", ESP.getFreeHeap());
    checkForUpdates();
    Serial.printf("Free heap after checking: %d bytes\n", ESP.getFreeHeap());
    lastUpdateCheck = currentMillis;
  }


  float temperature = 99, humidity = 100, co2 = 999999;

  if (scd30.dataReady()) {
    if (!scd30.read()) {
      Serial.println("Error leyendo el sensor!");
      return;
    }
    temperature = scd30.temperature;
    humidity = scd30.relative_humidity;
    co2 = scd30.CO2;
  }

  Serial.printf("Free heap before sending: %d bytes\n", ESP.getFreeHeap());
  send_data_grafana(temperature, humidity, co2);
  Serial.printf("Free heap after sending: %d bytes\n", ESP.getFreeHeap());

  delay(10000);
}
