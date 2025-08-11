#ifndef WIFIMANAGER_UTILS_H
#define WIFIMANAGER_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// ============================================================================
// WiFi Scanner Utility
// ============================================================================

class WiFiScanner {
public:
    struct NetworkInfo {
        String ssid;
        int32_t rssi;
        wifi_auth_mode_t authMode;
        bool isOpen;
        uint8_t channel;
        
        String getAuthModeString() const {
            switch (authMode) {
                case WIFI_AUTH_OPEN: return "Open";
                case WIFI_AUTH_WEP: return "WEP";
                case WIFI_AUTH_WPA_PSK: return "WPA";
                case WIFI_AUTH_WPA2_PSK: return "WPA2";
                case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
                case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
                case WIFI_AUTH_WPA3_PSK: return "WPA3";
                default: return "Unknown";
            }
        }
        
        String getSignalStrengthString() const {
            if (rssi > -50) return "Excellent";
            if (rssi > -60) return "Good";
            if (rssi > -70) return "Fair";
            return "Weak";
        }
    };
    
    static std::vector<NetworkInfo> scanNetworks(int maxResults = WIFI_SCAN_MAX_RESULTS) {
        std::vector<NetworkInfo> networks;
        
        LOG_TRACE("Scanning for WiFi networks...");
        int n = WiFi.scanNetworks(false, false, false, WIFI_SCAN_TIMEOUT);
        
        if (n == -1) {
            LOG_ERROR("WiFi scan failed");
            return networks;
        }
        
        LOG_TRACE("Found " + String(n) + " networks");
        
        for (int i = 0; i < n && i < maxResults; ++i) {
            NetworkInfo info;
            info.ssid = WiFi.SSID(i);
            info.rssi = WiFi.RSSI(i);
            info.authMode = WiFi.encryptionType(i);
            info.isOpen = (info.authMode == WIFI_AUTH_OPEN);
            info.channel = WiFi.channel(i);
            
            networks.push_back(info);
        }
        
        // Sort by signal strength
        std::sort(networks.begin(), networks.end(), 
                 [](const NetworkInfo& a, const NetworkInfo& b) {
                     return a.rssi > b.rssi;
                 });
        
        WiFi.scanDelete();
        return networks;
    }
    
    static String getNetworksAsJson() {
        auto networks = scanNetworks();
        String json = "[";
        
        for (size_t i = 0; i < networks.size(); i++) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + networks[i].ssid + "\",";
            json += "\"rssi\":" + String(networks[i].rssi) + ",";
            json += "\"auth\":\"" + networks[i].getAuthModeString() + "\",";
            json += "\"open\":" + String(networks[i].isOpen ? "true" : "false") + ",";
            json += "\"channel\":" + String(networks[i].channel) + ",";
            json += "\"strength\":\"" + networks[i].getSignalStrengthString() + "\"";
            json += "}";
        }
        
        json += "]";
        return json;
    }
};

// ============================================================================
// Status LED Controller
// ============================================================================

class StatusLED {
private:
    int pin;
    unsigned long lastBlink = 0;
    bool currentState = false;
    int blinkInterval = 1000;
    bool enabled = false;
    
public:
    StatusLED(int ledPin = STATUS_LED_PIN) : pin(ledPin) {
        if (pin >= 0) {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LED_OFF);
            enabled = true;
        }
    }
    
    void update() {
        if (!enabled) return;
        
        unsigned long now = millis();
        if (now - lastBlink >= blinkInterval) {
            lastBlink = now;
            currentState = !currentState;
            digitalWrite(pin, currentState ? LED_ON : LED_OFF);
        }
    }
    
    void setMode(int mode) {
        if (!enabled) return;
        
        switch (mode) {
            case 0: // Off
                blinkInterval = 0;
                digitalWrite(pin, LED_OFF);
                break;
            case 1: // Slow blink - AP mode
                blinkInterval = 1000;
                break;
            case 2: // Fast blink - Connecting
                blinkInterval = 200;
                break;
            case 3: // Solid on - Connected
                blinkInterval = 0;
                digitalWrite(pin, LED_ON);
                break;
        }
    }
    
    void off() { setMode(0); }
    void apMode() { setMode(1); }
    void connecting() { setMode(2); }
    void connected() { setMode(3); }
};

// ============================================================================
// Reset Button Handler
// ============================================================================

class ResetButton {
private:
    int pin;
    unsigned long pressStartTime = 0;
    bool isPressed = false;
    bool longPressTriggered = false;
    bool enabled = false;
    
public:
    ResetButton(int buttonPin = RESET_BUTTON_PIN) : pin(buttonPin) {
        if (pin >= 0) {
            pinMode(pin, INPUT_PULLUP);
            enabled = true;
        }
    }
    
    bool update() {
        if (!enabled) return false;
        
        bool currentPressed = (digitalRead(pin) == BUTTON_PRESSED);
        
        if (currentPressed && !isPressed) {
            // Button just pressed
            isPressed = true;
            pressStartTime = millis();
            longPressTriggered = false;
        } else if (!currentPressed && isPressed) {
            // Button just released
            isPressed = false;
            if (!longPressTriggered) {
                // Short press - could trigger scan or status print
                LOG_TRACE("Reset button short press");
                return false;
            }
        } else if (isPressed && !longPressTriggered) {
            // Button held down
            if (millis() - pressStartTime >= BUTTON_HOLD_TIME) {
                longPressTriggered = true;
                LOG_TRACE("Reset button long press - resetting configuration");
                return true;
            }
        }
        
        return false;
    }
};

// ============================================================================
// Memory Monitor
// ============================================================================

class MemoryMonitor {
public:
    static void printMemoryUsage() {
        LOG_TRACE("=== Memory Usage ===");
        LOG_TRACE("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        LOG_TRACE("Heap size: " + String(ESP.getHeapSize()) + " bytes");
        LOG_TRACE("Free PSRAM: " + String(ESP.getFreePsram()) + " bytes");
        LOG_TRACE("PSRAM size: " + String(ESP.getPsramSize()) + " bytes");
        LOG_TRACE("Flash size: " + String(ESP.getFlashChipSize()) + " bytes");
        LOG_TRACE("====================");
    }
    
    static bool isMemoryLow(size_t threshold = 50000) {
        return ESP.getFreeHeap() < threshold;
    }
    
    static void logIfMemoryLow(size_t threshold = 50000) {
        if (isMemoryLow(threshold)) {
            LOG_WARN("Low memory warning: " + String(ESP.getFreeHeap()) + " bytes free");
        }
    }
};

// ============================================================================
// Network Diagnostics
// ============================================================================

class NetworkDiagnostics {
public:
    static bool pingHost(const char* host, int timeout = 5000) {
        // Note: ESP32 doesn't have built-in ping, this is a simplified connectivity test
        WiFiClient client;
        bool connected = client.connect(host, 80, timeout);
        client.stop();
        return connected;
    }
    
    static void runDiagnostics() {
        LOG_TRACE("=== Network Diagnostics ===");
        
        // WiFi Status
        LOG_TRACE("WiFi Status: " + String(WiFi.status()));
        LOG_TRACE("SSID: " + WiFi.SSID());
        LOG_TRACE("RSSI: " + String(WiFi.RSSI()) + " dBm");
        LOG_TRACE("Channel: " + String(WiFi.channel()));
        LOG_TRACE("Local IP: " + WiFi.localIP().toString());
        LOG_TRACE("Gateway: " + WiFi.gatewayIP().toString());
        LOG_TRACE("DNS: " + WiFi.dnsIP().toString());
        LOG_TRACE("MAC Address: " + WiFi.macAddress());
        
        // Connectivity Tests
        LOG_TRACE("Testing connectivity...");
        bool google = pingHost("google.com");
        bool cloudflare = pingHost("1.1.1.1");
        
        LOG_TRACE("Google connectivity: " + String(google ? "OK" : "FAIL"));
        LOG_TRACE("Cloudflare DNS: " + String(cloudflare ? "OK" : "FAIL"));
        
        LOG_TRACE("===========================");
    }
};

// ============================================================================
// Web Interface Enhancements
// ============================================================================

class WebInterfaceHelper {
public:
    static String getAdvancedConfigPage(const WiFiScanner::NetworkInfo& networks) {
        String html = "<!DOCTYPE html><html><head><title>";
        html += WEB_TITLE;
        html += "</title><meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>";
        html += "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5}";
        html += ".container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}";
        html += "h1{color:" + String(WEB_THEME_COLOR) + ";text-align:center}";
        html += ".network{background:#f9f9f9;margin:10px 0;padding:10px;border-radius:4px;cursor:pointer}";
        html += ".network:hover{background:#e9e9e9}";
        html += "input{width:100%;padding:10px;margin:10px 0;border:1px solid #ddd;border-radius:4px}";
        html += "button{background:" + String(WEB_THEME_COLOR) + ";color:white;padding:12px 24px;border:none;border-radius:4px;cursor:pointer;width:100%}";
        html += "button:hover{opacity:0.9}";
        html += ".status{padding:10px;margin:10px 0;border-radius:4px;text-align:center}";
        html += ".connected{background:#d4edda;color:#155724;border:1px solid #c3e6cb}";
        html += ".disconnected{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}";
        html += ".signal{float:right;font-size:0.8em}";
        html += ".auth{color:#666;font-size:0.8em}";
        html += "</style>";
        html += "<script>";
        html += "function selectNetwork(ssid,auth){";
        html += "document.getElementById('ssid').value=ssid;";
        html += "if(auth=='Open'){document.getElementById('password').style.display='none';}";
        html += "else{document.getElementById('password').style.display='block';}";
        html += "}";
        html += "</script></head><body>";
        html += "<div class='container'>";
        html += "<h1>" + String(WEB_HEADER) + "</h1>";
        
        return html;
    }
    
    static String getStatusBadge(bool connected, const String& ssid = "") {
        if (connected) {
            return "<div class='status connected'>✓ Connected" + 
                   (ssid.isEmpty() ? "" : " to " + ssid) + "</div>";
        } else {
            return "<div class='status disconnected'>✗ Not Connected</div>";
        }
    }
};

#endif // WIFIMANAGER_UTILS_H