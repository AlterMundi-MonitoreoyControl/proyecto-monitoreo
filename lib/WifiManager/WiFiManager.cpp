#include "WiFiManager.h"
#include <cmath>

// Static instance for event handling
WiFiManager *WiFiManager::instance = nullptr;

WiFiManager::WiFiManager()
{
    instance = this;

    // Generate unique AP SSID based on MAC address
    String mac = getMacAddress();
    mac.replace(":", "");
    ap_config.ssid = "moni-" + mac;
}

WiFiManager::~WiFiManager()
{
    // if (webServer) delete webServer;
    if (dnsServer)
        delete dnsServer;
    preferences.end();
}

void WiFiManager::init(WebServer *server)
{
    LOG_TRACE("Initializing WiFi Manager...");

    // Initialize preferences
    preferences.begin("wifi", false);
    loadCredentials();

    // Set up WiFi event handler
    WiFi.onEvent(onWiFiEvent);

    // Configure WiFi mode (AP + STA)
    WiFi.mode(WIFI_AP_STA);

    // Configure Access Point
    WiFi.softAPConfig(sta_cfg.ip, sta_cfg.gateway, sta_cfg.netmask);
    WiFi.softAP(ap_config.ssid.c_str(), ap_config.password.c_str(),
                ap_config.channel, 0, ap_config.max_connections);

    String msg = "Access Point started: " + ap_config.ssid;
    LOG_TRACE(msg);

    // Set hostname
    WiFi.setHostname(ap_config.ssid.c_str());

    // Setup web server and DNS
    setupWebServer(server);
    setupDNS();

    // Attempt to connect if credentials are available
    if (!station_cfg.ssid.isEmpty())
    {
        connect();
    }
    else
    {
        LOG_TRACE("No WiFi credentials provided. Please configure WiFi settings.");
    }
}

bool WiFiManager::connect()
{
    if (station_cfg.ssid.isEmpty())
    {
        LOG_TRACE("No WiFi credentials provided. Please configure WiFi settings.");
        return false;
    }

    String msg = "Connecting to WiFi: " + station_cfg.ssid;
    LOG_TRACE(msg);

    WiFi.disconnect();
    delay(100);
    LOG_TRACE("Attempting to connect to WiFi...with SSID/" + station_cfg.ssid + "/pass/ " + station_cfg.password + "/");
    WiFi.begin(station_cfg.ssid.c_str(), station_cfg.password.c_str());

    return true;
}

void WiFiManager::onChange(const String &new_ssid, const String &new_password)
{
    if (status.is_transitioning)
    {
        LOG_TRACE("Configuration change already in progress, please wait...");
        return;
    }

    bool config_changed = false;
    status.is_transitioning = true;

    // Backup current configuration before any changes
    if (!new_ssid.isEmpty() && new_ssid != station_cfg.ssid)
    {
        old_ssid = station_cfg.ssid;
        old_password = station_cfg.password;
        config_changed = setNewSSID(new_ssid);
    }

    if (!new_ssid.isEmpty() && new_password != station_cfg.password)
    {
        old_ssid = station_cfg.ssid;
        old_password = station_cfg.password;
        config_changed = setPassword(new_password) || config_changed;
    }

    if (config_changed)
    {
        resetState();
        status.is_transitioning = true;
        connect();
        startValidationTimer();
        saveCredentials();
    }
    else
    {
        status.is_transitioning = false;
    }
}

bool WiFiManager::setNewSSID(const String &new_ssid)
{
    if (!new_ssid.isEmpty())
    {
        station_cfg.ssid = new_ssid;
        return true;
    }
    return false;
}

bool WiFiManager::setPassword(const String &new_password)
{
    station_cfg.password = new_password;
    LOG_TRACE("Password updated:/"+new_password+"/");
    return true;
}

void WiFiManager::resetState()
{
    current_retry = 0;
    status.is_transitioning = false;
    status.pending_fallback = false;
    validation_timer = 0;
    reconnect_timer = 0;
}

void WiFiManager::startValidationTimer()
{
    validation_timer = millis() + status.validation_timeout;
}

void WiFiManager::scheduleReconnect()
{
    // Exponential backoff for reconnection attempts
    unsigned long delay = min((unsigned long)(connection_timeout * pow(1.5, current_retry - 1)), 300000UL);
    reconnect_timer = millis() + delay;

    String msg = "Scheduling reconnection in " + String(delay) + "ms";
    LOG_TRACE(msg);
}

// void WiFiManager::update()
// {
//     // Handle validation timer
//     if (validation_timer > 0 && millis() > validation_timer)
//     {
//         validation_timer = 0;

//         if (status.is_transitioning && !online)
//         {
//             LOG_ERROR("New credentials validation failed, reverting to previous configuration...");
//             status.pending_fallback = true;

//             if (!old_ssid.isEmpty())
//             {
//                 setNewSSID(old_ssid);
//                 setPassword(old_password);
//                 old_ssid = "";
//                 old_password = "";
//                 resetState();
//                 connect();
//                 saveCredentials();
//             }
//         }
//     }

//     // Handle reconnection timer
//     if (reconnect_timer > 0 && millis() > reconnect_timer)
//     {
//         LOG_ERROR("reconnect timer is over ......................");
//         reconnect_timer = 0;
//         connect();
//     }

//     // Update web server and DNS
//     if (dnsServer)
//         dnsServer->processNextRequest();
// }

void WiFiManager::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (!instance)
        return;

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        instance->current_retry = 0;
        {
            String ssid = String((char *)info.wifi_sta_connected.ssid);
            String msg = "Connection to AP " + ssid + " established!";
            LOG_TRACE(msg);
        }
        LOG_TRACE("Waiting for IP address...");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        instance->online = true;

        // If this was a credential change and it succeeded
        if (instance->status.is_transitioning)
        {
            instance->resetState();
            instance->old_ssid = "";
            instance->old_password = "";
            LOG_TRACE("New credentials validated successfully");
        }

        LOG_TRACE("Network Configuration:");
        {
            String msg = "IP: " + WiFi.localIP().toString();
            LOG_TRACE(msg);
            msg = "Netmask: " + WiFi.subnetMask().toString();
            LOG_TRACE(msg);
            msg = "Gateway: " + WiFi.gatewayIP().toString();
            LOG_TRACE(msg);
            msg = "DNS: " + WiFi.dnsIP().toString();
            LOG_TRACE(msg);
        }

        // Setup NTP
        instance->setupNTP();

        LOG_TRACE("System is online and ready!");
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        instance->online = false;

        {
            String ssid = String((char *)info.wifi_sta_disconnected.ssid);
            String reason = String(info.wifi_sta_disconnected.reason);
            String msg = "WiFi disconnected from AP(" + ssid + "). Reason: " + reason;
            LOG_TRACE(msg);
        }

        // Don't retry if we're waiting for fallback
        if (instance->status.pending_fallback)
        {
            return;
        }

        if (instance->reconnect_timer > 0)
        {
            LOG_TRACE("Reconnection already scheduled, ignoring new disconnection event.");
            return;
        }

        if (instance->current_retry < instance->max_retries)
        {
            instance->current_retry++;
            String msg = "Attempting reconnection... (" +
                         String(instance->current_retry) + "/" +
                         String(instance->max_retries) + ")";
            LOG_TRACE(msg);
            instance->scheduleReconnect();
        }
        else
        {
            if (!instance->old_ssid.isEmpty() && !instance->status.is_transitioning)
            {
                LOG_TRACE("Maximum retries reached. Attempting to connect with previous credentials...");
                instance->setNewSSID(instance->old_ssid);
                instance->setPassword(instance->old_password);
                instance->old_ssid = "";
                instance->old_password = "";
                instance->resetState();
                instance->connect();
            }
            else
            {
                LOG_ERROR("Maximum retries reached. Please check WiFi configuration.");
                instance->resetState();
                // If the router is down we want to reconnect immediately once it comes back online
                instance->scheduleReconnect();
            }
        }
        break;

    default:
        break;
    }
}

void WiFiManager::setupWebServer(WebServer *server)
{
    webServer = server;
    if (!webServer)
        return;

    // Función principal del endpoint /wifi - VERSIÓN CON PAUSA DE RECONEXIÓN
    webServer->on("/wifi", HTTP_GET, [this]()
                  {
    LOG_TRACE("WiFi scan request received");
    
    // Verificar si ya hay un scan en progreso
    if (scan_requested) {
        LOG_TRACE("Scan already in progress, checking status");
        
        // Verificar si el scan completó
        int result = WiFi.scanComplete();
        if (result == WIFI_SCAN_RUNNING) {
            // Verificar timeout
            if (millis() - scan_start_time > SCAN_TIMEOUT_MS) {
                LOG_TRACE("Scan timeout, cleaning up and resuming reconnect");
                WiFi.scanDelete();
                resumeReconnection();
                String json = "{\"message\":\"scan timeout\",\"error\":-3}";
                webServer->send(408, "application/json", json);
                return;
            }
            String json = "{\"message\":\"scan in progress\",\"status\":\"scanning\"}";
            webServer->send(202, "application/json", json);
            return;
        } else if (result >= 0) {
            // Scan completado exitosamente
            LOG_TRACE("Scan completed with " + String(result) + " networks");
            resumeReconnection();
            sendScanResults(result);
            return;
        } else {
            // Error en el scan
            LOG_TRACE("Scan failed with error: " + String(result));
            resumeReconnection();
            String json = "{\"message\":\"scan failed\",\"error\":" + String(result) + "}";
            webServer->send(503, "application/json", json);
            return;
        }
    }
    
    // Verificar si WiFi está en un estado válido para scan
    if (WiFi.getMode() == WIFI_OFF) {
        LOG_TRACE("WiFi is off, cannot scan");
        String json = "{\"message\":\"WiFi is off\",\"error\":-1}";
        webServer->send(503, "application/json", json);
        return;
    }
    
    // PAUSA LA RECONEXIÓN Y FUERZA ESTADO IDLE
    LOG_TRACE("Pausing reconnection for WiFi scan");
    pauseReconnection();
    
    // Detener cualquier intento de conexión actual
    WiFi.disconnect();
    delay(100);  // Pequeña pausa para que se establezca el estado
    
    // Reiniciar WiFi en modo AP+STA para asegurar estado limpio
    WiFi.mode(WIFI_AP_STA);
    delay(100);
    
    // Iniciar scan asíncrono
    LOG_TRACE("Starting async WiFi scan after reconnection pause");
    int result = WiFi.scanNetworks(); // true = async mode
    
    if (result == WIFI_SCAN_RUNNING) {
        scan_requested = true;
        scan_start_time = millis();
        LOG_TRACE("Async scan started successfully");
        String json = "{\"message\":\"scan started\",\"status\":\"scanning\"}";
        webServer->send(202, "application/json", json);
    } else if (result == WIFI_SCAN_FAILED) {
        LOG_TRACE("Failed to start async scan even after reconnection pause");
        resumeReconnection(); // Reanudar inmediatamente si falla
        String json = "{\"message\":\"failed to start scan\",\"error\":" + String(result) + "}";
        webServer->send(503, "application/json", json);
    } else if (result >= 0) {
        // Scan completado inmediatamente (caso raro)
        LOG_TRACE("Scan completed immediately with " + String(result) + " networks");
        resumeReconnection();
        sendScanResults(result);
    } else {
        LOG_TRACE("Unexpected scan result: " + String(result));
        resumeReconnection();
        String json = "{\"message\":\"unexpected scan result\",\"error\":" + String(result) + "}";
        webServer->send(503, "application/json", json);
    } });

    // Root page - WiFi configuration interface
    webServer->on("/", [this]()
                  {
        String html = "<!DOCTYPE html><html><head><title>WiFi Setup</title>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<style>body{font-family:Arial,sans-serif;margin:40px auto;max-width:600px;line-height:1.6}";
        html += "input{width:100%;padding:10px;margin:10px 0;border:1px solid #ddd}";
        html += "button{background:#007cba;color:white;padding:10px 20px;border:none;cursor:pointer}";
        html += "button:hover{background:#005a87}</style></head><body>";
        html += "<h1>WiFi Configuration</h1>";
        
        // Status
        html += "<p>Current Status: ";
        html += online ? "Connected" : "Disconnected";
        html += "</p>";
        
        if (online) {
            html += "<p>Connected to: ";
            html += station_cfg.ssid;
            html += "</p>";
            html += "<p>IP Address: ";
            html += WiFi.localIP().toString();
            html += "</p>";
        }
        
        // Form
        html += "<form action='/save' method='POST'>";
        html += "<label>WiFi Network:</label>";
        html += "<input type='text' name='ssid' placeholder='Enter WiFi SSID' value='";
        html += station_cfg.ssid;
        html += "'>";
        html += "<label>Password:</label>";
        html += "<input type='password' name='password' placeholder='Enter WiFi Password'>";
        html += "<button type='submit'>Save & Connect</button>";
        html += "</form></body></html>";
        
        webServer->send(200, "text/html", html); });

    // Save configuration
    webServer->on("/save", HTTP_POST, [this]()
                  {
        String ssid = webServer->arg("ssid");
        String password = webServer->arg("password");
        
        if (!ssid.isEmpty()) {
            String msg = "Received new WiFi configuration: " + ssid;
            LOG_TRACE(msg);
            onChange(ssid, password);
            
            String html = "<!DOCTYPE html><html><head><title>WiFi Setup</title>";
            html += "<meta http-equiv='refresh' content='5;url=/'></head><body>";
            html += "<h1>Configuration Saved</h1>";
            html += "<p>Attempting to connect to: ";
            html += ssid;
            html += "</p>";
            html += "<p>You will be redirected in 5 seconds...</p>";
            html += "</body></html>";
            
            webServer->send(200, "text/html", html);
        } else {
            webServer->send(400, "text/plain", "SSID cannot be empty");
        } });
}

void WiFiManager::setupDNS()
{
    dnsServer = new DNSServer();
    dnsServer->start(53, "*", sta_cfg.ip);
    LOG_TRACE("DNS server started - captive portal active");
}

String WiFiManager::getMacAddress()
{
    return WiFi.macAddress();
}

void WiFiManager::setupNTP()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    LOG_TRACE("NTP configured");
}

void WiFiManager::saveCredentials()
{
    preferences.putString("ssid", station_cfg.ssid);
    preferences.putString("password", station_cfg.password);
    LOG_TRACE("Credentials saved to flash");
}

void WiFiManager::loadCredentials()
{
    station_cfg.ssid = preferences.getString("ssid", "");
    station_cfg.password = preferences.getString("password", "");

    if (!station_cfg.ssid.isEmpty())
    {
        String msg = "Loaded saved credentials for: " + station_cfg.ssid;
        LOG_TRACE(msg);
    }
}

void WiFiManager::disconnect()
{
    WiFi.disconnect();
    online = false;
    resetState();
}

void WiFiManager::reset()
{
    preferences.clear();
    station_cfg.ssid = "";
    station_cfg.password = "";
    disconnect();
    LOG_TRACE("WiFi configuration reset");
}

void WiFiManager::printStatus()
{
    LOG_TRACE("=== WiFi Manager Status ===");
    String msg = "Online: " + String(online ? "Yes" : "No");
    LOG_TRACE(msg);
    msg = "AP SSID: " + ap_config.ssid;
    LOG_TRACE(msg);
    msg = "Station SSID: " + station_cfg.ssid;
    LOG_TRACE(msg);
    msg = "Current Retry: " + String(current_retry) + "/" + String(max_retries);
    LOG_TRACE(msg);
    msg = "Is Transitioning: " + String(status.is_transitioning ? "Yes" : "No");
    LOG_TRACE(msg);
    if (online)
    {
        msg = "Local IP: " + WiFi.localIP().toString();
        LOG_TRACE(msg);
    }
    LOG_TRACE("========================");
}

// Método para pausar la reconexión
void WiFiManager::pauseReconnection() {
    reconnect_paused = true;
    reconnect_timer = 0; // Cancelar timer de reconexión pendiente
    LOG_TRACE("Reconnection paused for WiFi scan");
}

// Método para reanudar la reconexión
void WiFiManager::resumeReconnection() {
    scan_requested = false;
    scan_start_time = 0;
    
    if (reconnect_paused) {
        reconnect_paused = false;
        LOG_TRACE("Resuming reconnection after WiFi scan");
        
        // Solo intentar reconectar si no estamos online y tenemos credenciales
        if (!online && !station_cfg.ssid.isEmpty()) {
            LOG_TRACE("Attempting to reconnect after scan completion");
            // Programar reconexión inmediata
            reconnect_timer = millis() + 1000; // 1 segundo de delay
        }
    }
}

// Modificar el método update() para manejar scan timeout automáticamente
void WiFiManager::update() {
    // Limpiar scan si se cuelga
    if (scan_requested && (millis() - scan_start_time > SCAN_TIMEOUT_MS)) {
        LOG_TRACE("Auto-cleanup: Scan timeout reached");
        WiFi.scanDelete();
        resumeReconnection();
    }
    
    // Handle validation timer
    if (validation_timer > 0 && millis() > validation_timer) {
        validation_timer = 0;
        
        if (status.is_transitioning && !online) {
            LOG_ERROR("New credentials validation failed, reverting to previous configuration...");
            status.pending_fallback = true;
            
            if (!old_ssid.isEmpty()) {
                setNewSSID(old_ssid);
                setPassword(old_password);
                old_ssid = "";
                old_password = "";
                resetState();
                connect();
                saveCredentials();
            }
        }
    }

    // Handle reconnection timer - SOLO SI NO ESTÁ PAUSADO
    if (!reconnect_paused && reconnect_timer > 0 && millis() > reconnect_timer) {
        LOG_ERROR("reconnect timer is over ......................");
        reconnect_timer = 0;
        connect();
    }
    
    // Update web server and DNS
    if (dnsServer) dnsServer->processNextRequest();
}

// Método sendScanResults manteniendo formato original
void WiFiManager::sendScanResults(int networkCount) {
    String json = "{";
    
    if (networkCount < 0) {
        LOG_TRACE("wifi scan error: " + String(networkCount));
        json += "\"message\":\"scan failed\",\"error\":" + String(networkCount) + "}";
    } else if (networkCount == 0) {
        LOG_TRACE("wifi scan no networks");
        json += "\"message\":\"no networks found\"}";
    } else {
        LOG_TRACE("wifi scan found " + String(networkCount) + " networks");
        
        json += "\"message\":\"success\",\"networks\":[";
        for (int i = 0; i < networkCount; ++i) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i));
            json += "}";
        }
        json += "]}";
    }

    webServer->send(200, "application/json", json);
    
    // Limpiar resultados del scan
    WiFi.scanDelete();
}