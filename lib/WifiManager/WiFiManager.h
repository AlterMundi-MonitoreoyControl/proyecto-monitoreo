#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "../../include/debug.h"

// Logging macros using debug system
#define LOG_TRACE(msg) DBG_VERBOSE("[WiFi] %s\n", String(msg).c_str())
#define LOG_ERROR(msg) DBG_ERROR("[WiFi] %s\n", String(msg).c_str())



class WiFiManager {
private:
    // Configuration structures
    bool scan_requested = false;
    bool reconnect_paused = false;
    unsigned long scan_start_time = 0;
    static const unsigned long SCAN_TIMEOUT_MS = 15000;
    
    struct StaConfig {
        IPAddress ip = IPAddress(192, 168, 16, 10);
        IPAddress netmask = IPAddress(255, 255, 255, 0);
        IPAddress gateway = IPAddress(192, 168, 16, 10);
        IPAddress dns = IPAddress(0, 0, 0, 0);
    } sta_cfg;
    
    struct ApConfig {
        String ssid;
        String password = "12345678";
        int auth = WIFI_AUTH_WPA2_PSK;
        int channel = 1;
        int max_connections = 4;
    } ap_config;
    
    struct StationConfig {
        String ssid = "";
        String password = "";
        int scan_method = WIFI_ALL_CHANNEL_SCAN;
    } station_cfg;
    
    struct Status {
        bool is_transitioning = false;
        unsigned long last_change_time = 0;
        unsigned long validation_timeout = 30000; // 30 seconds
        bool pending_fallback = false;
    } status;
    
    // State variables
    bool online = false;
    unsigned long connection_timeout = 100000; // 10 seconds
    int max_retries = 10;
    int current_retry = 0;
    
    // Backup credentials for fallback
    String old_ssid = "";
    String old_password = "";
    
    // Timers
    unsigned long reconnect_timer = 0;
    unsigned long validation_timer = 0;
    
    // Internal objects
    Preferences preferences;
    WebServer* webServer = nullptr;
    DNSServer* dnsServer = nullptr;
    
    // Private methods
    void resetState();
    void startValidationTimer();
    void scheduleReconnect();
    void handleWiFiEvent();
    void setupWebServer(WebServer* server) ;
    void setupDNS();
    String generateCaptivePortalPage();
    String getMacAddress();
    void setupNTP();
    void saveCredentials();
    void loadCredentials();
    void sendScanResults(int networkCount); 
    void pauseReconnection();
    void resumeReconnection();
    // Event handlers
    static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
    static WiFiManager* instance; // For static callback
    
public:
    WiFiManager();
    ~WiFiManager();
    
    // Main interface methods
    void init(WebServer* server);
    bool connect();
    void onChange(const String& new_ssid, const String& new_password = "");
    
    // Getters
    bool isOnline() const { return online; }
    String getCurrentSSID() const { return station_cfg.ssid; }
    String getAPSSID() const { return ap_config.ssid; }
    IPAddress getLocalIP() const { return WiFi.localIP(); }
    
    // Setters
    bool setNewSSID(const String& new_ssid);
    bool setPassword(const String& new_password);
    void setConnectionTimeout(unsigned long timeout) { connection_timeout = timeout; }
    void setMaxRetries(int retries) { max_retries = retries; }
    void setValidationTimeout(unsigned long timeout) { status.validation_timeout = timeout; }
    
    // Utility methods
    void update(); // Call in loop()
    void disconnect();
    void reset();
    void printStatus();
};

#endif // WIFIMANAGER_H