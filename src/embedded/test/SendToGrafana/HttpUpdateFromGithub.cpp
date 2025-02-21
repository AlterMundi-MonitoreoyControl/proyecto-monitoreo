#include "GithubHttpUpdate.h"

HttpUpdateFromGithub::HttpUpdateFromGithub(const char* username, const char* repo, const char* currentVersion)
    : _username(username)
    , _repo(repo)
    , _currentVersion(currentVersion)
    , _clientSecure(nullptr)
    , _onStart(nullptr)
    , _onEnd(nullptr)
    , _onProgress(nullptr)
    , _onError(nullptr)
{
}

void HttpUpdateFromGithub::begin(WiFiClientSecure& secureClient) {
    _clientSecure = &secureClient;
}

void HttpUpdateFromGithub::setCallbacks(
    void (*onStart)(),
    void (*onEnd)(),
    void (*onProgress)(int current, int total),
    void (*onError)(int error)
) {
    _onStart = onStart;
    _onEnd = onEnd;
    _onProgress = onProgress;
    _onError = onError;
}

String HttpUpdateFromGithub::createApiUrl() const {
    return String("https://api.github.com/repos/") + _username + "/" + _repo + "/releases/latest";
}

String HttpUpdateFromGithub::createFirmwareUrl(const String& tag) const {
    return String("https://github.com/") + _username + "/" + _repo + 
           "/releases/download/" + tag + "/SendToGrafana.ino.bin";
}

String HttpUpdateFromGithub::getLatestReleaseTag() {
    String apiUrl = createApiUrl();
    Serial.println("API URL: " + apiUrl);

    if (!_http.begin(*_clientSecure, apiUrl)) {
        Serial.println("Failed to begin HTTP connection");
        return "";
    }

    String result = "";
    int httpCode = _http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = _http.getString();
        int tagPos = payload.indexOf("\"tag_name\":");
        
        if (tagPos != -1) {
            int startQuote = payload.indexOf("\"", tagPos + 11);
            int endQuote = payload.indexOf("\"", startQuote + 1);
            
            if (startQuote != -1 && endQuote != -1) {
                result = payload.substring(startQuote + 1, endQuote);
            }
        }
        
        if (result.isEmpty()) {
            Serial.println("Tag not found in JSON!");
        }
    } else {
        Serial.printf("HTTP GET failed, error: %d\n", httpCode);
    }

    _http.end();
    return result;
}

bool HttpUpdateFromGithub::handleRedirect(const String& url, String& redirectUrl) {
    HTTPClient redirectHttp;
    if (!redirectHttp.begin(*_clientSecure, url)) {
        Serial.println("Failed to begin redirect HTTP connection");
        return false;
    }

    const char *headerKeys[] = {"Location"};
    const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
    redirectHttp.collectHeaders(headerKeys, headerKeysCount);

    int redirectCode = redirectHttp.GET();
    bool success = false;

    if (redirectCode == HTTP_CODE_FOUND || redirectCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String location = redirectHttp.header("Location");
        if (location.length() > 0) {
            redirectUrl = location;
            success = true;
        }
    }

    redirectHttp.end();
    return success;
}

bool HttpUpdateFromGithub::checkForUpdates() {
    String latestTag = getLatestReleaseTag();
    if (latestTag.isEmpty()) {
        Serial.println("Unable to check for updates... empty release tag.");
        return false;
    }

    Serial.printf("Current version: %s, Available version: %s\n", _currentVersion, latestTag.c_str());

    if (latestTag == _currentVersion) {
        Serial.println("Firmware is up to date.");
        return true;
    }

    const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        Serial.println("No OTA partition available");
        return false;
    }

    String firmwareUrl = createFirmwareUrl(latestTag);
    String redirectUrl;
    
    if (handleRedirect(firmwareUrl, redirectUrl)) {
        firmwareUrl = redirectUrl;
    }

    HTTPUpdate httpUpdate;

    // Set callbacks if they were provided
    if (_onStart) httpUpdate.onStart(_onStart);
    if (_onEnd) httpUpdate.onEnd(_onEnd);
    if (_onProgress) httpUpdate.onProgress(_onProgress);
    if (_onError) httpUpdate.onError(_onError);

    t_httpUpdate_return ret = httpUpdate.update(*_clientSecure, firmwareUrl);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", 
                         httpUpdate.getLastError(), 
                         httpUpdate.getLastErrorString().c_str());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            return true;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            esp_ota_set_boot_partition(update_partition);
            return true;
    }

    return false;
}