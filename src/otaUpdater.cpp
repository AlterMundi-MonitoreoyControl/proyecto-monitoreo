#include <Arduino.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>  
#include <WiFiClientSecure.h>  
#include <esp_ota_ops.h>
#include "version.h"
#include "otaUpdater.h"
#include "globals.h"
#include "constants.h"

String getLatestReleaseTag(const char* repoOwner, const char* repoName) {
    HTTPClient http; 
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