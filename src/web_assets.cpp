#include "web_assets.h"

const char *style_css = R"=====(:root {
    --altermundi-green: #55d400;
    --altermundi-orange: #F39100;
    --altermundi-blue: #0198fe;
    --gray-dark: #333;
    --gray-medium: #666;
    --gray-light: #f5f5f5;
    --red-error: #dc3545;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
    background: linear-gradient(135deg, #f5f5f5 0%, #e8e8e8 100%);
    padding: 15px;
    min-height: 100vh;
}

.container {
    max-width: 850px;
    margin: 0 auto;
    background: white;
    padding: 25px;
    border-radius: 12px;
    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

h1 {
    color: var(--gray-dark);
    margin-bottom: 10px;
    border-bottom: 3px solid var(--altermundi-green);
    padding-bottom: 12px;
    font-size: 24px;
    text-align: center;
}

.subtitle {
    color: var(--gray-medium);
    font-size: 14px;
    margin-bottom: 20px;
    text-align: center;
}

.nav {
    text-align: center;
    margin-bottom: 20px;
}

.nav a {
    display: inline-block;
    padding: 10px 20px;
    margin: 5px;
    background: var(--altermundi-blue);
    color: white;
    text-decoration: none;
    border-radius: 6px;
    font-weight: 500;
    transition: background 0.2s;
}

.nav a:hover {
    background: #017dd1;
}

/* CARDS (Data Page) */
.cards {
    display: flex;
    flex-wrap: wrap;
    gap: 12px;
    justify-content: center;
}

.card {
    background: #fff;
    border-radius: 8px;
    padding: 15px;
    min-width: 280px;
    max-width: 350px;
    flex: 1;
    box-shadow: 0 2px 4px rgba(0, 0, 0, .1);
    border-left: 4px solid var(--altermundi-green);
}

.card.err {
    border-left-color: var(--red-error);
    opacity: .7;
}

.card.warn {
    border-left-color: var(--altermundi-orange);
}

.hdr {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 10px;
}

.type {
    font-weight: 600;
    color: #333;
    font-size: 1.1em;
}

.id {
    font-size: .7em;
    color: #888;
    background: #f0f0f0;
    padding: 2px 6px;
    border-radius: 3px;
}

.vals {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px;
}

.val {
    padding: 10px 8px;
    background: #f9f9f9;
    border-radius: 6px;
    text-align: center;
}

.val span {
    display: block;
    font-size: .7em;
    color: #666;
    margin-bottom: 2px;
}

.val b {
    font-size: 1.3em;
    color: #333;
}

.val.ok b { color: var(--altermundi-green); }
.val.warn b { color: var(--altermundi-orange); }
.val.bad b { color: var(--red-error); }

.status {
    text-align: center;
    margin-top: 15px;
    padding: 10px;
    background: #fff;
    border-radius: 6px;
    font-size: .85em;
    color: #666;
    box-shadow: 0 1px 3px rgba(0, 0, 0, .08);
}

.status b { color: #333; }
.empty { text-align: center; padding: 40px; color: #888; }


/* CONFIG PAGE STYLES */
.section {
    margin-bottom: 25px;
    padding: 20px;
    background: #fafafa;
    border-radius: 8px;
    border-left: 4px solid var(--altermundi-green);
    transition: all 0.3s ease;
}

.section:hover {
    box-shadow: 0 2px 8px rgba(85, 212, 0, 0.15);
}

.form-group {
    margin-bottom: 18px;
}

label {
    display: block;
    margin-bottom: 6px;
    color: var(--gray-dark);
    font-weight: 600;
    font-size: 14px;
}

input[type="text"], input[type="number"], select {
    width: 100%;
    padding: 10px 12px;
    border: 2px solid #ddd;
    border-radius: 6px;
    font-size: 14px;
    transition: border-color 0.3s ease;
}

input[type="text"]:focus, input[type="number"]:focus, select:focus {
    outline: none;
    border-color: var(--altermundi-green);
}

input[type="checkbox"] {
    width: 20px;
    height: 20px;
    margin-right: 10px;
    vertical-align: middle;
    cursor: pointer;
}

.checkbox-label {
    display: inline;
    font-weight: normal;
    cursor: pointer;
}

.sensor-item {
    background: white;
    padding: 15px;
    margin-bottom: 12px;
    border-radius: 8px;
    border-left: 4px solid var(--altermundi-orange);
    box-shadow: 0 1px 3px rgba(0,0,0,0.08);
    transition: transform 0.2s ease;
}

.sensor-item:hover {
    transform: translateX(5px);
}

.btn {
    background: var(--altermundi-green);
    color: white;
    padding: 12px 28px;
    border: none;
    border-radius: 6px;
    cursor: pointer;
    font-size: 16px;
    font-weight: 600;
    margin-right: 10px;
    margin-top: 10px;
    transition: all 0.3s ease;
    box-shadow: 0 2px 4px rgba(85, 212, 0, 0.3);
    text-align: center;
    text-decoration: none;
    display: inline-block;
}

.btn:hover {
    background: #48b800;
    transform: translateY(-2px);
    box-shadow: 0 4px 8px rgba(85, 212, 0, 0.4);
}

.btn-secondary {
    background: var(--gray-medium);
    box-shadow: 0 2px 4px rgba(0,0,0,0.2);
}

.btn-secondary:hover {
    background: #555;
    box-shadow: 0 4px 8px rgba(0,0,0,0.3);
}

.btn-warning {
    background: var(--altermundi-orange);
    color: white;
    box-shadow: 0 2px 4px rgba(243,145,0,0.3);
}

.btn-warning:hover {
    background: #d47e00;
    transform: translateY(-2px);
}

.message {
    padding: 14px 18px;
    margin-top: 20px;
    border-radius: 8px;
    display: none;
    font-weight: 500;
}

.message.success {
    background: #d4f4dd;
    color: #1e7e34;
    border: 2px solid var(--altermundi-green);
}

.message.error {
    background: #ffe6e6;
    color: #c82333;
    border: 2px solid #dc3545;
}

.message.warning {
    background: #fff8e1;
    color: #856404;
    border: 2px solid var(--altermundi-orange);
}

.loading {
    text-align: center;
    padding: 40px 20px;
    color: var(--altermundi-green);
    font-size: 18px;
    font-weight: 500;
}

.info-text {
    color: var(--gray-medium);
    font-size: 12px;
    margin-top: 3px;
}

.pwd-wrap {
    position: relative;
}

.pwd-wrap input {
    padding-right: 60px;
}

.pwd-toggle {
    position: absolute;
    right: 6px;
    top: 50%;
    transform: translateY(-50%);
    background: transparent;
    border: 1px solid var(--gray-medium);
    color: var(--gray-dark);
    font-size: 12px;
    font-weight: 500;
    padding: 4px 10px;
    border-radius: 4px;
    cursor: pointer;
    line-height: 1;
}

.pwd-toggle:hover {
    border-color: var(--altermundi-green);
    color: var(--altermundi-green);
}

.hidden {
    display: none !important;
}

.inline-group {
    display: flex;
    gap: 10px;
}

.inline-group .form-group {
    flex: 1;
}

/* HOME PAGE SPECIFIC */
.info-card {
    background: #fafafa;
    border-left: 4px solid var(--altermundi-green);
    border-radius: 8px;
    padding: 15px;
    margin-bottom: 15px;
}

.info-card h2 {
    color: var(--altermundi-green);
    font-size: 16px;
    margin-bottom: 10px;
    font-weight: 600;
}

.info-row {
    display: flex;
    justify-content: space-between;
    padding: 8px 0;
    border-bottom: 1px solid #e0e0e0;
}

.info-row:last-child {
    border-bottom: none;
}

.info-label {
    color: var(--gray-medium);
    font-weight: 600;
    font-size: 14px;
}

.info-value {
    color: var(--gray-dark);
    font-size: 14px;
    text-align: right;
    word-break: break-all;
}

.nav-buttons {
    display: flex;
    gap: 10px;
    margin-top: 20px;
    flex-wrap: wrap;
}

.nav-buttons .btn {
    flex: 1;
    min-width: 140px;
}

.signal-excellent { color: #28a745; }
.signal-good { color: #ffc107; }
.signal-fair { color: #fd7e14; }
.signal-weak { color: #dc3545; }

/* Status Badges */
.status-badge {
    display: inline-block;
    padding: 4px 8px;
    border-radius: 12px;
    font-size: 12px;
    font-weight: 600;
    line-height: 1;
}

.status-online {
    background-color: #d4f4dd;
    color: #1e7e34;
    border: 1px solid #c3e6cb;
}

.status-online::before {
    content: '';
    display: inline-block;
    width: 6px;
    height: 6px;
    background-color: #28a745;
    border-radius: 50%;
    margin-right: 4px;
    vertical-align: middle;
}

.status-offline {
    background-color: #f8d7da;
    color: #721c24;
    border: 1px solid #f5c6cb;
}

.status-offline::before {
    content: '';
    display: inline-block;
    width: 6px;
    height: 6px;
    background-color: #dc3545;
    border-radius: 50%;
    margin-right: 4px;
    vertical-align: middle;
}

@media (max-width: 480px) {
    .nav-buttons {
        flex-direction: column;
    }
    .btn {
        width: 100%;
        margin-right: 0;
    }
    .vals {
        grid-template-columns: 1fr;
    }
}
)=====";

const char *index_html = R"=====(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Monitor - Dashboard</title>
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <link rel="stylesheet" href="style.css">
    <script>
        document.addEventListener('DOMContentLoaded', () => {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('deviceName').textContent = data.device_name;
                    document.getElementById('deviceId').textContent = data.device_id;
                    document.getElementById('firmwareVersion').textContent = 'v' + data.firmware_version;
                    
                    const wifiStatus = document.getElementById('wifiStatus');
                    wifiStatus.textContent = data.wifi_connected ? 'Conectado' : 'Desconectado';
                    wifiStatus.className = 'status-badge ' + (data.wifi_connected ? 'status-online' : 'status-offline');
                    
                    if (data.wifi_connected) {
                        document.getElementById('wifiSSID').textContent = data.wifi_ssid;
                        document.getElementById('wifiIP').textContent = data.wifi_ip;
                        const rssiElem = document.getElementById('wifiRSSI');
                        rssiElem.textContent = data.wifi_rssi + ' dBm';
                        rssiElem.className = 'info-value ' + data.signal_class;
                    }
                    
                    document.getElementById('activeSensors').textContent = data.active_sensors + " / " + data.total_sensors;
                    document.getElementById('uptime').textContent = data.uptime;
                })
                .catch(err => {
                    console.error('Error fetching status:', err);
                });
        });
    </script>
</head>
<body>
    <div class="container">
        <h1>🏠 Monitor AlterMundi</h1>
        <div class="subtitle">La pata tecnológica de ese otro mundo posible</div>

        <!-- Card Identificación -->
        <div class="info-card">
            <h2>📟 Identificación</h2>
            <div class="info-row"><span class="info-label">Dispositivo:</span><span class="info-value" id="deviceName">-</span></div>
            <div class="info-row"><span class="info-label">ID:</span><span class="info-value" id="deviceId">-</span></div>
            <div class="info-row"><span class="info-label">Firmware:</span><span class="info-value" id="firmwareVersion">-</span></div>
        </div>

        <!-- Card Conectividad -->
        <div class="info-card">
            <h2>📡 Conectividad</h2>
            <div class="info-row"><span class="info-label">Estado:</span><span id="wifiStatus" class="status-badge status-offline">Cargando...</span></div>
            <div class="info-row"><span class="info-label">Red:</span><span class="info-value" id="wifiSSID">-</span></div>
            <div class="info-row"><span class="info-label">IP:</span><span class="info-value" id="wifiIP">-</span></div>
            <div class="info-row"><span class="info-label">Señal:</span><span class="info-value" id="wifiRSSI">-</span></div>
        </div>

        <!-- Card Estado -->
        <div class="info-card">
            <h2>📊 Estado del Sistema</h2>
            <div class="info-row"><span class="info-label">Sensores Activos:</span><span class="info-value" id="activeSensors">-</span></div>
            <div class="info-row"><span class="info-label">Tiempo Encendido:</span><span class="info-value" id="uptime">-</span></div>
        </div>

        <div class="nav-buttons">
            <a href="/data" class="btn btn-primary">📊 Ver Datos</a>
            <a href="/settings" class="btn btn-secondary">⚙️ Configuración</a>
            <a href="/rules-editor" class="btn btn-secondary">⚡ Reglas Automáticas</a>
            <a href="/wifi-setup" class="btn btn-secondary">📡 Configurar WiFi</a>
        </div>
        
        <!-- Restart button removed as per user request -->
    </div>
</body>
</html>
)=====";

const char *data_html = R"=====(<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Datos - Monitor</title>
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <link rel="stylesheet" href="style.css">
</head>

<body>
    <h1>📊 Datos de Sensores</h1>
    <div class="nav">
        <a href="/">🏠 Inicio</a>
        <a href="/settings">⚙️ Configuración</a>
        <a href="/rules-editor">⚡ Reglas</a>
    </div>

    <div class="cards" id="sensorCards">
        <!-- Rendered by JS -->
        <div class="loading">Cargando sensores...</div>
    </div>

    <h1 style="margin-top:25px" id="relayHeader">🔌 Relés / Actuadores</h1>
    <div class="cards" id="relayCards">
        <!-- Rendered by JS -->
    </div>

    <div class="status" id="statusBar">
        Cargando estado...
    </div>

    <script>
        function updateData() {
            // Fetch Sensor Data
            fetch('/actual')
                .then(r => r.json())
                .then(renderSensors)
                .catch(e => console.error("Error fetching data:", e));

            // Fetch Relays
            fetch('/api/relays')
                .then(r => r.json())
                .then(renderRelays)
                .catch(e => console.error("Error fetching relays:", e));
        }

        function renderSensors(data) {
            // Note: Currently /actual returns a semi-structured JSON.
            // Ideally, we'd want a clean list of sensors.
            // For now, we mimic the existing logic based on what /actual returns OR
            // better yet, we might need to improve /actual to be more like 'getSensorList'
            // BUT, to avoid breaking changes in the first pass, let's look at what we have.

            // Wait! The /actual endpoint implementation in endpoints.cpp returns a summarized JSON
            // (a_temperature, etc.) suitable for simple parsing, but NOT the full list of sensors for the cards.
            // RE-READING endpoints.cpp: handleData() generates HTML.
            // We need a NEW endpoint or modified /actual to return the full array of sensors.

            // PLAN ADJUSTMENT: We will assume we'll update /actual or create /api/sensors 
            // to return the ARRAY of sensor data needed for these cards.
            // let's assume /api/sensors returns [ {type, id, values: [{label, value, unit, status}], ...} ]

            // Since we haven't implemented that C++ part yet, let's write the JS 
            // expecting a structure we will implement.

            // For now, let's assume `data.sensors` contains the list if we update C++.
            // If we use the EXISTING /actual, it only has summarized data (single values).
            // We MUST update the C++ to provide the list. 
            // I will implement /api/sensors in C++ later.

            const container = document.getElementById('sensorCards');
            if (!data.sensors || data.sensors.length === 0) {
                if (data.a_temperature) {
                    // Fallback for single sensor legacy /actual structure if we don't have the list
                    // (This is just a bridge until we implement full list)
                    // Actually, we will make /api/sensors the source of truth.
                } else {
                    container.innerHTML = "<div class='empty'>No hay datos de sensores</div>";
                    return;
                }
            } else {
                container.innerHTML = '';
            }

            // ... Rendering logic to be filled when we define the JSON structure exactly ...
            // Let's assume the C++ will return:
            // sensors: [ { type, id, active, ready, readings: [ {label, value, unit, status} ] } ]

            data.sensors.forEach(s => {
                const card = document.createElement('div');
                card.className = 'card' + (s.error ? ' err' : '');

                let html = `<div class='hdr'>
                   <span class='type'>${s.icon} ${s.type}</span>
                   <span class='id'>${s.id}</span>
               </div>
               <div class='vals'>`;

                s.readings.forEach(r => {
                    html += `<div class='val ${r.status}'>
                       <span>${r.label}</span>
                       <b>${r.value} ${r.unit}</b>
                   </div>`;
                });

                if (s.readings.length === 0) {
                    html += `<div class='val'><span>Estado</span><b>${s.active ? "Sin datos" : "Inactivo"}</b></div>`;
                }

                html += `</div>`;
                card.innerHTML = html;
                container.appendChild(card);
            });

            // Update status bar
            const bar = document.getElementById('statusBar');
            bar.innerHTML = `<b>WiFi:</b> ${data.wifi_status} | <b>Sensores:</b> ${data.sensors.length} | <b>Activo:</b> ${data.uptime}`;
        }

        function renderRelays(data) {
            const container = document.getElementById('relayCards');
            const header = document.getElementById('relayHeader');

            if (!data || data.length === 0) {
                container.style.display = 'none';
                header.style.display = 'none';
                return;
            }

            container.style.display = 'flex';
            header.style.display = 'block';
            container.innerHTML = '';

            data.forEach(r => {
                const card = document.createElement('div');
                card.className = 'card';
                card.style.borderLeftColor = '#0198fe';

                let html = `<div class='hdr'>
                    <span class='type'>${r.type === 'gpio' ? 'Relé GPIO' : 'Relé Modbus'}</span>
                    <span class='id'>${r.type === 'gpio' ? 'Pin: ' : 'Dir: '}${r.address}</span>
                </div>
                <div class='vals'>`;

                if (r.alias) {
                    html += `<div class='val' style='grid-column:span 2;background:none;text-align:left;padding:0 5px'>
                        <span>${r.alias}</span>
                    </div>`;
                }

                // Outputs (Channels)
                if (r.state) {
                    r.state.forEach((state, idx) => {
                        const status = state ? "ok" : "warn";
                        const label = state ? "ON" : "OFF";
                        const chanLabel = r.type === 'gpio' ? 'Estado' : `Canal ${idx + 1}`;
                        // Note: toggle function needs to be global or accessible
                        html += `<div class='val ${status}' style='cursor:pointer' 
                             onclick='toggle(${r.address}, ${idx})'>
                             <span>${chanLabel}</span><b>${label}</b>
                        </div>`;
                    });
                }

                // Inputs
                if (r.input_state) {
                    r.input_state.forEach((state, idx) => {
                        const status = state ? "ok" : "warn";
                        const label = state ? "ON" : "OFF";
                        html += `<div class='val ${status}'>
                             <span>Entrada ${idx + 1}</span><b>${label}</b>
                        </div>`;
                    });
                }

                html += `</div>`;
                card.innerHTML = html;
                container.appendChild(card);
            });
        }

        function toggle(addr, ch) {
            fetch(`/api/relay/toggle?addr=${addr}&ch=${ch}`, { method: 'POST' })
                .then(r => {
                    if (r.ok) updateData(); // Reload data
                    else alert('Error al cambiar estado');
                });
        }

        // Initial load
        updateData();
        // Auto-refresh every 10s
        setInterval(updateData, 10000);
    </script>
</body>

</html>
)=====";

const char *config_html = R"=====(<!DOCTYPE html>
<html lang="es">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <title>Configuración - Monitor</title>
    <link rel="stylesheet" href="style.css">
</head>

<body>
    <div class="container">
        <h1>Configuración del Sistema</h1>
        <div class="subtitle">AlterMundi - La pata tecnológica de ese otro mundo posible</div>
        <div class="nav"><a href="/">🏠 Inicio</a><a href="/data">📊 Datos</a><a href="/rules-editor">⚡ Reglas</a></div>
        <div id="loading" class="loading">Cargando configuración...</div>

        <form id="configForm" style="display:none;">
            <!-- WiFi Section -->
            <div class="section">
                <h2>WiFi</h2>
                <div class="form-group">
                    <label for="ssid">SSID</label>
                    <input type="text" id="ssid" name="ssid" required>
                </div>
                <div class="form-group">
                    <label for="passwd">Contraseña</label>
                    <div class="pwd-wrap">
                        <input type="password" id="passwd" name="passwd" minlength="8" maxlength="63">
                        <button type="button" class="pwd-toggle" data-target="passwd" aria-label="Mostrar contraseña">Ver</button>
                    </div>
                    <div class="info-text">8-63 caracteres (WPA2). Dejar vacío para mantener contraseña actual.</div>
                </div>
                <div class="form-group">
                    <label>Canal WiFi Actual</label>
                    <div id="wifi_channel_status"
                        style="padding: 10px; background: #f0f0f0; border-radius: 6px; color: var(--gray-dark); font-weight: 500;">
                        -</div>
                    <div class="info-text">Usar este canal para todos los dispositivos ESP-NOW</div>
                </div>
            </div>

            <!-- Sistema Section -->
            <div class="section">
                <h2>Sistema</h2>
                <div class="form-group">
                    <label for="incubator_name">Nombre del Dispositivo</label>
                    <input type="text" id="incubator_name" name="incubator_name">
                </div>
                <div class="inline-group">
                    <div class="form-group">
                        <label for="min_temperature">Temperatura Mín (°C)</label>
                        <input type="number" id="min_temperature" name="min_temperature" step="0.1">
                    </div>
                    <div class="form-group">
                        <label for="max_temperature">Temperatura Máx (°C)</label>
                        <input type="number" id="max_temperature" name="max_temperature" step="0.1">
                    </div>
                </div>
                <div class="inline-group">
                    <div class="form-group">
                        <label for="min_hum">Humedad Mín (%)</label>
                        <input type="number" id="min_hum" name="min_hum" min="0" max="100">
                    </div>
                    <div class="form-group">
                        <label for="max_hum">Humedad Máx (%)</label>
                        <input type="number" id="max_hum" name="max_hum" min="0" max="100">
                    </div>
                </div>
            </div>

            <!-- ESP-NOW Section -->
            <div class="section">
                <h2>ESP-NOW</h2>
                <div class="form-group">
                    <input type="checkbox" id="espnow_enabled" name="espnow_enabled">
                    <label class="checkbox-label" for="espnow_enabled">Habilitar ESP-NOW</label>
                </div>
                <div id="espnow_config">
                    <div class="form-group">
                        <label for="espnow_force_mode">Modo de Operación</label>
                        <select id="espnow_force_mode" name="espnow_force_mode">
                            <option value="">Auto-detectar (recomendado)</option>
                            <option value="gateway">Forzar Gateway</option>
                            <option value="sensor">Forzar Sensor</option>
                        </select>
                        <small style="color: var(--gray-medium);">Auto-detecta basándose en conectividad
                            WiFi/Grafana</small>
                    </div>
                    <div class="form-group">
                        <label for="grafana_ping_url">URL de Prueba Grafana</label>
                        <input type="text" id="grafana_ping_url" name="grafana_ping_url"
                            placeholder="http://192.168.1.1/ping">
                        <small style="color: var(--gray-medium);">URL para verificar conectividad a Grafana</small>
                    </div>
                    <div class="form-group">
                        <label for="espnow_channel">Canal WiFi</label>
                        <select id="espnow_channel" name="espnow_channel">
                            <option value="1">1</option>
                            <option value="6">6</option>
                            <option value="11">11</option>
                        </select>
                        <small style="color: var(--gray-medium);">Debe coincidir con el canal WiFi del gateway</small>
                    </div>
                    <div class="form-group">
                        <label for="send_interval_ms">Intervalo de envío (ms)</label>
                        <input type="number" id="send_interval_ms" name="send_interval_ms" min="5000" max="300000"
                            step="1000" value="30000">
                        <small style="color: var(--gray-medium);">Tiempo entre envíos de datos (solo sensores)</small>
                    </div>
                    <div id="espnow_status"
                        style="margin-top: 15px; padding: 10px; background: #f9f9f9; border-radius: 4px;">
                        <strong>Estado:</strong> <span id="espnow_status_text">Cargando...</span><br>
                        <strong>Modo Actual:</strong> <span id="espnow_current_mode">-</span><br>
                        <strong>Canal WiFi Actual:</strong> <span id="espnow_channel_status">-</span><br>
                        <strong>MAC Address:</strong> <span id="espnow_mac">-</span><br>
                        <span id="espnow_paired_status"></span>
                        <span id="espnow_peer_count"></span>
                    </div>
                </div>
            </div>

            <!-- RS485 Section (Unified Bus Config) -->
            <div class="section">
                <h2>RS485 (Bus Compartido por sensores, rele o envio crudo)</h2>
                <div class="info-text"
                    style="background: #e3f2fd; padding: 10px; border-radius: 4px; margin-bottom: 15px;">
                    Configuración única del bus RS485. Todos los dispositivos Modbus (sensores, relés) comparten este
                    bus.
                </div>
                <div class="form-group">
                    <input type="checkbox" id="rs485_enabled" name="rs485_enabled">
                    <label class="checkbox-label" for="rs485_enabled">Habilitar Bus RS485</label>
                </div>
                <div id="rs485_config">
                    <div class="inline-group">
                        <div class="form-group">
                            <label for="rs485_rx">Pin RX</label>
                            <input type="number" id="rs485_rx" name="rs485_rx" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="rs485_tx">Pin TX</label>
                            <input type="number" id="rs485_tx" name="rs485_tx" min="0" max="39">
                        </div>
                        <div class="form-group">
                            <label for="rs485_de">Pin DE/RE</label>
                            <input type="number" id="rs485_de" name="rs485_de" min="-1" max="39">
                            <div class="info-text">-1 si no usa control DE/RE</div>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="rs485_baud">Baudrate</label>
                        <select id="rs485_baud" name="rs485_baud">
                            <option value="4800">4800</option>
                            <option value="9600">9600</option>
                            <option value="19200">19200</option>
                        </select>
                    </div>
                    <div class="form-group" style="margin-top: 15px; padding-top: 15px; border-top: 1px solid #ddd;">
                        <input type="checkbox" id="rs485_raw_send" name="rs485_raw_send">
                        <label class="checkbox-label" for="rs485_raw_send">Enviar datos crudos por serial</label>
                        <div class="info-text">Envía lecturas de sensores como texto plano por RS485 (además del
                            protocolo Modbus)</div>
                    </div>
                </div>
            </div>

            <!-- Sensors Section -->
            <div class="section">
                <h2>Sensores</h2>
            <div style="margin-bottom: 15px; display: flex; gap: 8px; flex-wrap: wrap; align-items: center;">
                    <button type="button" class="btn" onclick="loadDefaultSensors()">📋 Por Defecto</button>
                    <button type="button" class="btn btn-secondary" style="background: var(--altermundi-orange);"
                        onclick="forgetWiFi()">📡 Olvidar WiFi</button>
                    <button type="button" class="btn btn-secondary" style="background: #dc3545;"
                        onclick="clearAllConfig()">🗑️ Limpiar Todo</button>
                    <span style="flex:1"></span>
                    <select id="newMoistureType" style="padding:8px 10px;border-radius:6px;border:1px solid #ccc;font-size:14px;">
                        <option value="capacitive">Capacitivo</option>
                        <option value="hd38">HD38 / Resistivo</option>
                    </select>
                    <button type="button" class="btn" style="padding:8px 16px;"
                        onclick="addMoistureSensor()">+ Sensor Humedad</button>
                </div>
                <div id="sensors-list"></div>
            </div>

            <!-- Relays Section -->
            <div class="section">
                <h2>Relés / Actuadores</h2>
                <div id="relays-list"></div>
                <div style="display: flex; gap: 10px; margin-top: 10px;">
                    <button type="button" class="btn btn-secondary"
                        style="padding: 6px 12px; font-size: 12px;" onclick="addRelay('relay_2ch')">+ Agregar Relé Modbus</button>
                    <button type="button" class="btn btn-secondary"
                        style="padding: 6px 12px; font-size: 12px;" onclick="addRelay('gpio')">+ Agregar Relé GPIO</button>
                </div>
            </div>

            <div class="message" id="message"></div>

            <button type="submit" class="btn">Guardar Configuración</button>
            <button type="button" class="btn btn-secondary" onclick="location.reload()">Recargar</button>
            <button type="button" class="btn btn-secondary"
                onclick="if(confirm('¿Reiniciar ESP32?')) restartDevice()">Reiniciar Dispositivo</button>
        </form>

        <!-- Admin password change (out of the main form to avoid sending it with the rest of the config) -->
        <form id="adminPwdForm" autocomplete="off" style="margin-top: 20px;">
            <div class="section">
                <h2>Acceso admin</h2>
                <div class="info-text" style="margin-bottom: 10px;">
                    Cambiar la contraseña usada por el panel y la API (Basic Auth).
                    Si nunca configuraste una, los endpoints sensibles aceptan acceso libre
                    hasta que la fijes desde aquí.
                </div>
                <div class="form-group" id="currentAdminPassRow">
                    <label for="currentAdminPass">Contraseña actual</label>
                    <div class="pwd-wrap">
                        <input type="password" id="currentAdminPass" autocomplete="current-password" minlength="8" maxlength="64">
                        <button type="button" class="pwd-toggle" data-target="currentAdminPass" aria-label="Mostrar contraseña">Ver</button>
                    </div>
                </div>
                <div class="form-group">
                    <label for="newAdminPass">Nueva contraseña</label>
                    <div class="pwd-wrap">
                        <input type="password" id="newAdminPass" autocomplete="new-password" minlength="8" maxlength="64" required>
                        <button type="button" class="pwd-toggle" data-target="newAdminPass" aria-label="Mostrar contraseña">Ver</button>
                    </div>
                    <div class="info-text">8 a 64 caracteres.</div>
                </div>
                <div class="form-group">
                    <label for="confirmAdminPass">Confirmar nueva contraseña</label>
                    <div class="pwd-wrap">
                        <input type="password" id="confirmAdminPass" autocomplete="new-password" minlength="8" maxlength="64" required>
                        <button type="button" class="pwd-toggle" data-target="confirmAdminPass" aria-label="Mostrar contraseña">Ver</button>
                    </div>
                </div>
                <button type="submit" class="btn">Cambiar contraseña</button>
                <div class="message" id="adminPwdMessage"></div>
            </div>
        </form>
    </div>

    <!-- We can move the large JS logic to a separate file, but for now inline to match previous structure, 
         or better, separate it since we are in separated mode now! -->
    <script src="config.js"></script>
</body>

</html>
)=====";

const char *config_js = R"=====(
let currentConfig = {};

// Default sensor configuration
// Buses: I2C, Modbus, OneWire
// Sensores: ADC, digital
// Nota: modbus_th solo necesita direcciones, la config del bus RS485 es global
const DEFAULT_SENSORS = [
    // Buses
    { type: "scd30", enabled: true, config: {} },
    { type: "bme280", enabled: false, config: {} },
    { type: "modbus_th", enabled: false, config: { addresses: [1] } },
    { type: "modbus_soil_7in1", enabled: false, config: { addresses: [1] } },
    { type: "onewire", enabled: false, config: { pin: 4, scan: true } },
    // Sensores
    { type: "internal_temp", enabled: false, config: {} },
    { type: "capacitive", enabled: false, config: { pin: 34, name: "Soil1" } },
    { type: "hd38", enabled: false, config: { analog_pins: [35], voltage_divider: true, invert_logic: false } }
];

const DEFAULT_RELAY_TEMPLATE = {
    type: "relay_2ch", enabled: true, config: { address: 1, alias: "Nuevo Relé" }
};
const DEFAULT_GPIO_RELAY_TEMPLATE = {
    type: "gpio", enabled: true, config: { pin: 2, alias: "Nuevo Relé GPIO", active_low: false }
};

// Load configuration on page load
window.addEventListener('DOMContentLoaded', () => {
    loadConfig();
    initPasswordToggles();
    initAdminPasswordForm();
    refreshAdminPasswordState();
});

function initPasswordToggles() {
    document.querySelectorAll('.pwd-toggle').forEach((btn) => {
        btn.addEventListener('click', () => {
            const target = document.getElementById(btn.dataset.target);
            if (!target) return;
            if (target.type === 'password') {
                target.type = 'text';
                btn.textContent = 'Ocultar';
            } else {
                target.type = 'password';
                btn.textContent = 'Ver';
            }
        });
    });
}

async function refreshAdminPasswordState() {
    try {
        const r = await fetch('/api/admin/info', { cache: 'no-store' });
        if (!r.ok) return;
        const data = await r.json();
        const row = document.getElementById('currentAdminPassRow');
        const input = document.getElementById('currentAdminPass');
        if (!row || !input) return;
        if (data.configured) {
            row.classList.remove('hidden');
            input.required = true;
        } else {
            row.classList.add('hidden');
            input.required = false;
            input.value = '';
        }
    } catch (e) {
        // Silent: leave UI as default (current visible).
    }
}

function showAdminPwdMessage(text, type) {
    const el = document.getElementById('adminPwdMessage');
    if (!el) return;
    el.className = 'message ' + type;
    el.textContent = text;
    el.style.display = 'block';
    setTimeout(() => { el.style.display = 'none'; }, 6000);
}

function initAdminPasswordForm() {
    const form = document.getElementById('adminPwdForm');
    if (!form) return;
    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        const next = document.getElementById('newAdminPass').value;
        const confirm = document.getElementById('confirmAdminPass').value;
        if (next.length < 8 || next.length > 64) {
            showAdminPwdMessage('La nueva contraseña debe tener entre 8 y 64 caracteres.', 'error');
            return;
        }
        if (next !== confirm) {
            showAdminPwdMessage('La confirmación no coincide.', 'error');
            return;
        }
        const payload = {
            current: document.getElementById('currentAdminPass').value,
            new: next
        };
        showAdminPwdMessage('Actualizando contraseña admin...', 'info');
        try {
            const r = await fetch('/api/admin/password', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            const text = await r.text();
            if (!r.ok) throw new Error(text || 'No se pudo cambiar la contraseña.');
            showAdminPwdMessage(text + ' En el próximo acceso el navegador pedirá la nueva.', 'success');
            form.reset();
            document.querySelectorAll('.pwd-toggle').forEach(b => { b.textContent = 'Ver'; });
            refreshAdminPasswordState();
        } catch (err) {
            showAdminPwdMessage(err.message || 'No se pudo cambiar la contraseña.', 'error');
        }
    });
}

function showMessage(msg, type) {
    const el = document.getElementById('message');
    el.className = 'message ' + type;
    el.textContent = msg;
    el.style.display = 'block';
    setTimeout(() => { el.style.display = 'none'; }, 5000);
}

function restartDevice() {
    fetch('/restart', { method: 'POST' })
        .then(r => r.text())
        .then(t => showMessage('Reiniciando... Espera unos segundos y recarga.', 'success'));
}

function forgetWiFi() {
    if (confirm("¿Seguro que deseas olvidar la red WiFi? El dispositivo reiniciará en modo AP.")) {
        // We probably need an endpoint for this, or just clear config ssid/pass and save.
        // Let's implement it by clearing fields and saving.
        document.getElementById('ssid').value = "ToChange";
        document.getElementById('passwd').value = "";

        // Trigger save
        document.querySelector('#configForm button[type="submit"]').click();
    }
}

function clearAllConfig() {
    if (confirm("ATENCIÓN: Esto borrará TODA la configuración y restablecerá los valores de fábrica. ¿Continuar?")) {
        fetch('/config/reset', { method: 'POST' })
            .then(r => r.json())
            .then(data => {
                if (data.success) {
                    showMessage(data.message, 'success');
                    setTimeout(() => location.reload(), 3000);
                } else {
                    showMessage("Error al resetear", 'error');
                }
            });
    }
}

function loadDefaultSensors() {
    if (confirm("Esto reemplazará la lista actual de sensores con los valores por defecto. ¿Continuar?")) {
        currentConfig.sensors = JSON.parse(JSON.stringify(DEFAULT_SENSORS));
        renderSensors(currentConfig.sensors);
    }
}


async function loadConfig() {
    try {
        const response = await fetch('/config');
        if (!response.ok) throw new Error('Error al cargar configuración');

        currentConfig = await response.json();
        populateForm(currentConfig);

        document.getElementById('loading').style.display = 'none';
        document.getElementById('configForm').style.display = 'block';
    } catch (error) {
        document.getElementById('loading').innerHTML =
            '<div class="message error" style="display:block">Error: ' + error.message + '</div>';
    }
}

function populateForm(config) {
    // WiFi
    const ssidEl = document.getElementById('ssid');
    if (ssidEl) ssidEl.value = config.ssid || '';

    const passwdEl = document.getElementById('passwd');
    if (passwdEl) passwdEl.value = ''; // Never show password

    // WiFi Channel
    const channel = config.current_wifi_channel;
    const wfChEl = document.getElementById('wifi_channel_status');
    if (wfChEl) wfChEl.textContent = channel > 0 ? channel : 'No conectado';

    // Sistema
    document.getElementById('incubator_name').value = config.incubator_name || config.moni_name || '';
    document.getElementById('min_temperature').value = config.min_temperature || '';
    document.getElementById('max_temperature').value = config.max_temperature || '';
    document.getElementById('min_hum').value = config.min_hum || '';
    document.getElementById('max_hum').value = config.max_hum || '';

    // RS485 (nested object structure)
    const rs485 = config.rs485 || {};
    document.getElementById('rs485_enabled').checked = rs485.enabled || false;
    document.getElementById('rs485_rx').value = rs485.rx_pin !== undefined ? rs485.rx_pin : 16;
    document.getElementById('rs485_tx').value = rs485.tx_pin !== undefined ? rs485.tx_pin : 17;
    document.getElementById('rs485_de').value = rs485.de_pin !== undefined ? rs485.de_pin : 18;
    document.getElementById('rs485_baud').value = rs485.baudrate || 9600;
    document.getElementById('rs485_raw_send').checked = rs485.raw_send_enabled || false;

    // Toggle RS485 config visibility
    toggleRS485Config();
    document.getElementById('rs485_enabled').addEventListener('change', toggleRS485Config);

    // ESP-NOW
    document.getElementById('espnow_enabled').checked = config.espnow_enabled || false;
    document.getElementById('espnow_force_mode').value = config.espnow_force_mode || '';
    document.getElementById('grafana_ping_url').value = config.grafana_ping_url || 'http://192.168.1.1/ping';
    document.getElementById('espnow_channel').value = config.espnow_channel || 1;
    document.getElementById('send_interval_ms').value = config.send_interval_ms || 30000;

    // Toggle ESP-NOW config visibility
    toggleESPNowConfig();
    document.getElementById('espnow_enabled').addEventListener('change', toggleESPNowConfig);

    // Load ESP-NOW status
    loadESPNowStatus();

    // Sensors
    renderSensors(config.sensors || []);

    // Relays
    renderRelays(config.relays || []);
}

function toggleRS485Config() {
    const enabled = document.getElementById('rs485_enabled').checked;
    document.getElementById('rs485_config').style.display = enabled ? 'block' : 'none';
}

function toggleESPNowConfig() {
    const enabled = document.getElementById('espnow_enabled').checked;
    document.getElementById('espnow_config').style.display = enabled ? 'block' : 'none';
}

async function loadESPNowStatus() {
    try {
        const response = await fetch('/espnow/status');
        if (!response.ok) {
            document.getElementById('espnow_status_text').textContent = 'No disponible';
            return;
        }

        const status = await response.json();
        document.getElementById('espnow_status_text').textContent = status.enabled ? 'Habilitado' : 'Deshabilitado';
        document.getElementById('espnow_current_mode').textContent = status.mode || '-';
        document.getElementById('espnow_mac').textContent = status.mac_address || '-';
        const channel = status.channel;
        document.getElementById('espnow_channel_status').textContent = channel > 0 ? channel : 'No disponible (sin WiFi)';

        const pairedStatusEl = document.getElementById('espnow_paired_status');
        const peerCountEl = document.getElementById('espnow_peer_count');

        if (status.mode === 'sensor') {
            pairedStatusEl.innerHTML =
                '<strong>Emparejado:</strong> ' + (status.paired ? '✓ Sí' : '✗ No') + '<br>';
            peerCountEl.innerHTML = '';
        } else if (status.mode === 'gateway') {
            pairedStatusEl.innerHTML = '';
            peerCountEl.innerHTML =
                '<strong>Peers conectados:</strong> ' + (status.peer_count || 0);
        } else {
            pairedStatusEl.innerHTML = '';
            peerCountEl.innerHTML = '';
        }
    } catch (error) {
        document.getElementById('espnow_status_text').textContent = 'Error al cargar estado';
    }
}

// Kick off raw polling when settings page loads
window.addEventListener('DOMContentLoaded', () => { startRawPolling(); });

function renderSensors(sensors) {
    const container = document.getElementById('sensors-list');
    container.innerHTML = '';

    // Use default sensors if none configured
    if (!Array.isArray(sensors) || sensors.length === 0) {
        sensors = DEFAULT_SENSORS;
        container.innerHTML = '<div class="info-text" style="background: #fff3cd; border-left: 4px solid var(--altermundi-orange); padding: 12px; margin-bottom: 15px;">⚠️ Usando configuración por defecto. Guarda para aplicar.</div>';
    }

    sensors.forEach((sensor, index) => {
        const sensorDiv = document.createElement('div');
        sensorDiv.className = 'sensor-item';
        sensorDiv.innerHTML = `
            <div class="form-group">
                <input type="checkbox" id="sensor_${index}_enabled"
                       data-sensor-index="${index}" class="sensor-enabled"
                       ${sensor.enabled ? 'checked' : ''}>
                <label class="checkbox-label" for="sensor_${index}_enabled">
                    <strong>${sensor.type.toUpperCase()}</strong>
                </label>
            </div>
            ${renderSensorConfig(sensor, index)}
        `;
        container.appendChild(sensorDiv);
    });
}

function renderSensorConfig(sensor, index) {
    const config = sensor.config || {};

    switch (sensor.type) {
        case 'scd30':
            return `
                <div class="form-group" style="margin-top: 10px;">
                    <button type="button" class="btn btn-warning" style="padding: 6px 12px; font-size: 14px;" onclick="calibrateSCD30()">⚙️ Calibrar SCD30 (400ppm)</button>
                    <div class="info-text">Exponga el sensor al aire libre (400ppm) por 5 minutos antes de calibrar.</div>
                </div>
            `;

        case 'internal_temp':
            return '<div class="info-text">Sensor de temperatura interna del ESP32. No requiere configuración adicional.</div>';

        case 'capacitive':
        case 'hd38':
            return renderMoistureSensorConfig(sensor, index);

        case 'onewire':
            return `
                <div class="form-group">
                    <label for="sensor_${index}_pin">Pin Data</label>
                    <input type="number" id="sensor_${index}_pin"
                           value="${config.pin || 4}" min="0" max="39">
                </div>
                <div class="form-group">
                    <input type="checkbox" id="sensor_${index}_scan"
                           ${config.scan ? 'checked' : ''}>
                    <label class="checkbox-label" for="sensor_${index}_scan">
                        Auto-detectar sensores DS18B20
                    </label>
                </div>
            `;

        case 'modbus_th':
            // Support both 'addresses' array and legacy 'address' single value
            // RS485 bus config is now global - sensor only needs addresses
            const addrListTH = config.addresses || (config.address ? [config.address] : [1]);
            const addrStrTH = Array.isArray(addrListTH) ? addrListTH.join(', ') : addrListTH;
            return `
                <div class="form-group">
                    <label for="sensor_${index}_addresses">Direcciones Modbus</label>
                    <input type="text" id="sensor_${index}_addresses"
                           value="${addrStrTH}" placeholder="1, 45, 3">
                    <div class="info-text">Separar con comas para múltiples sensores. Usa la config RS485 global para pines y baudrate.</div>
                </div>
            `;

        case 'modbus_soil_7in1':
            // 7-in-1 Soil Sensor (ZTS-3001-TR-ECTGNPKPH-N01)
            // Measures: moisture, temp, EC, pH, N, P, K
            const addrList7in1 = config.addresses || (config.address ? [config.address] : [1]);
            const addrStr7in1 = Array.isArray(addrList7in1) ? addrList7in1.join(', ') : addrList7in1;
            return `
                <div class="form-group">
                    <label for="sensor_${index}_addresses">Direcciones Modbus</label>
                    <input type="text" id="sensor_${index}_addresses"
                           value="${addrStr7in1}" placeholder="1, 2, 3">
                    <div class="info-text">Sensor 7-en-1: Humedad, Temp, EC, pH, N, P, K. Baud rate recomendado: 4800.</div>
                </div>
            `;

        default:
            return '<div class="info-text">Sin configuración adicional</div>';
    }
}

// ── Unified moisture sensor card (capacitive & hd38) ──────────────────────
function renderMoistureSensorConfig(sensor, index) {
    const cfg = sensor.config || {};
    const isHD38 = sensor.type === 'hd38';
    const defaultPin = isHD38 ? 35 : 34;
    const defaultDry = cfg.dry !== undefined ? cfg.dry : (isHD38 ? 4095 : 3500);
    const defaultWet = cfg.wet !== undefined ? cfg.wet : (isHD38 ? 0    : 1500);
    const label = isHD38 ? 'HD38 / Resistivo' : 'Capacitivo';

    return `
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px">
            <small style="color:var(--gray-medium)">${label}</small>
            <button type="button" style="background:none;border:none;color:#dc3545;cursor:pointer;font-size:16px"
                onclick="removeSensor(${index})">🗑️</button>
        </div>
        <div class="inline-group">
            <div class="form-group">
                <label for="sensor_${index}_name">Nombre</label>
                <input type="text" id="sensor_${index}_name"
                       value="${cfg.name || ''}" placeholder="Ej: Suelo 1" style="width:100%">
            </div>
            <div class="form-group">
                <label for="sensor_${index}_pin">Pin ADC</label>
                <input type="number" id="sensor_${index}_pin"
                       value="${cfg.pin !== undefined ? cfg.pin : defaultPin}" min="0" max="39"
                       style="width:100%">
            </div>
        </div>
        <div style="background:#f0f7ff;border-left:3px solid var(--altermundi-blue);padding:8px 12px;border-radius:4px;margin-bottom:8px;font-size:13px">
            📡 RAW actual: <strong id="raw_badge_${index}">–</strong>
            <span style="color:var(--gray-medium)"> (actualiza cada 3s cuando el sensor está activo)</span>
        </div>
        <div class="inline-group">
            <div class="form-group">
                <label for="sensor_${index}_dry">Valor Seco (raw ADC)</label>
                <input type="number" id="sensor_${index}_dry"
                       value="${defaultDry}" min="0" max="4095">
                <div class="info-text">Lectura en aire / suelo seco</div>
            </div>
            <div class="form-group">
                <label for="sensor_${index}_wet">Valor Húmedo (raw ADC)</label>
                <input type="number" id="sensor_${index}_wet"
                       value="${defaultWet}" min="0" max="4095">
                <div class="info-text">Lectura en agua / suelo saturado</div>
            </div>
        </div>
        ${isHD38 ? `
        <div class="inline-group" style="margin-top:4px">
            <div class="form-group">
                <input type="checkbox" id="sensor_${index}_voltage_divider"
                       ${cfg.voltage_divider !== false ? 'checked' : ''}>
                <label class="checkbox-label" for="sensor_${index}_voltage_divider">Divisor de voltaje (5V)</label>
            </div>
            <div class="form-group">
                <input type="checkbox" id="sensor_${index}_invert_logic"
                       ${cfg.invert_logic ? 'checked' : ''}>
                <label class="checkbox-label" for="sensor_${index}_invert_logic">Invertir lógica</label>
            </div>
        </div>` : ''}
    `;
}

function addMoistureSensor() {
    const type = document.getElementById('newMoistureType').value;
    if (!currentConfig.sensors) currentConfig.sensors = [];
    const isHD38 = type === 'hd38';
    currentConfig.sensors.push({
        type,
        enabled: true,
        config: {
            pin: isHD38 ? 35 : 34,
            dry: isHD38 ? 4095 : 3500,
            wet: isHD38 ? 0    : 1500,
            name: '',
            ...(isHD38 ? { voltage_divider: false, invert_logic: false } : {})
        }
    });
    renderSensors(currentConfig.sensors);
    // Scroll to new sensor
    const items = document.querySelectorAll('#sensors-list .sensor-item');
    if (items.length) items[items.length - 1].scrollIntoView({ behavior: 'smooth' });
}

function removeSensor(index) {
    if (!confirm('¿Eliminar este sensor?')) return;
    currentConfig.sensors.splice(index, 1);
    renderSensors(currentConfig.sensors);
}

// Poll /actual every 3s and update raw badges for moisture sensors
let _rawPollTimer = null;
function startRawPolling() {
    if (_rawPollTimer) return;
    _rawPollTimer = setInterval(() => {
        fetch('/actual').then(r => r.json()).then(data => {
            if (!data.sensors) return;
            data.sensors.forEach(s => {
                if (!s.diagnostics || s.diagnostics.raw === undefined) return;
                // Match sensor by id (m-adc-<pin>)
                const pin = s.diagnostics.pin;
                if (pin === undefined) return;
                // Find all badges whose input pin matches
                document.querySelectorAll('#sensors-list .sensor-item').forEach((item, idx) => {
                    const pinEl = document.getElementById(`sensor_${idx}_pin`);
                    if (pinEl && parseInt(pinEl.value) === pin) {
                        const badge = document.getElementById(`raw_badge_${idx}`);
                        if (badge) badge.textContent = s.diagnostics.raw;
                    }
                });
            });
        }).catch(() => {});
    }, 3000);
}

function calibrateSCD30() {
    if (!confirm("¿Está seguro de calibrar el sensor SCD30 a 400ppm? Asegúrese de que ha estado en aire limpio por al menos 5 minutos.")) {
        return;
    }
    fetch('/calibrate-scd30')
        .then(r => r.json())
        .then(data => {
            if (data.status === 'success') {
                showMessage(data.message, 'success');
            } else {
                showMessage('Error: ' + data.message, 'error');
            }
        })
        .catch(err => {
            showMessage('Error de conexión al calibrar', 'error');
        });
}

function renderRelays(relays) {
    const container = document.getElementById('relays-list');
    container.innerHTML = '';

    if (!Array.isArray(relays)) relays = [];

    relays.forEach((relay, index) => {
        const relayDiv = document.createElement('div');
        relayDiv.className = 'sensor-item'; // Reuse style
        relayDiv.style.borderLeftColor = '#0198fe'; // Blue for relays

        let configHtml = '';
        if (relay.type === 'gpio') {
            configHtml = `
            <div class="inline-group">
                <div class="form-group">
                    <label>Alias</label>
                    <input type="text" id="relay_${index}_alias" value="${relay.config.alias || ''}" placeholder="Ej: Ventilador">
                </div>
                <div class="form-group">
                    <label>Pin GPIO</label>
                    <input type="number" id="relay_${index}_pin" value="${relay.config.pin !== undefined ? relay.config.pin : 2}" min="0" max="39">
                </div>
            </div>
            <div style="background:#f0f7ff; padding:12px; border-radius:4px; margin-bottom:8px; border-left: 3px solid var(--altermundi-blue);">
                <div class="inline-group">
                    <div class="form-group">
                        <label>Max ON (ms)</label>
                        <input type="number" id="relay_${index}_max_on" value="${relay.config.max_on_ms || 0}" placeholder="0 = Infinito">
                    </div>
                    <div class="form-group">
                        <label>Min OFF (ms)</label>
                        <input type="number" id="relay_${index}_min_off" value="${relay.config.min_off_ms || 0}" placeholder="0 = Sin límite">
                    </div>
                    <div class="form-group" style="padding-top:25px;">
                        <input type="checkbox" id="relay_${index}_active_low" ${relay.config.active_low ? 'checked' : ''}>
                        <label class="checkbox-label" for="relay_${index}_active_low">Lógica Invertida (Activo en BAJO)</label>
                    </div>
                </div>
            </div>`;
        } else {
            configHtml = `
            <div class="inline-group">
                <div class="form-group">
                    <label>Alias</label>
                    <input type="text" id="relay_${index}_alias" value="${relay.config.alias || ''}" placeholder="Ej: Ventilador">
                </div>
                <div class="form-group">
                    <label>Dirección Modbus</label>
                    <input type="number" id="relay_${index}_address" value="${relay.config.address || 1}" min="1" max="254">
                </div>
            </div>
            <div style="background:#f0f7ff; padding:12px; border-radius:4px; margin-bottom:8px; border-left: 3px solid var(--altermundi-blue);">
                <strong style="font-size:13px; color:var(--altermundi-blue); display:block; margin-bottom:10px;">Configuración Canal 1</strong>
                <div class="inline-group">
                    <div class="form-group">
                        <label>Max ON (ms)</label>
                        <input type="number" id="relay_${index}_ch0_max" value="${relay.config.ch0?.max_on_ms || 0}" placeholder="0 = Infinito">
                    </div>
                    <div class="form-group">
                        <label>Min OFF (ms)</label>
                        <input type="number" id="relay_${index}_ch0_min" value="${relay.config.ch0?.min_off_ms || 0}" placeholder="0 = Sin límite">
                    </div>
                    <div class="form-group" style="padding-top:25px;">
                        <input type="checkbox" id="relay_${index}_ch0_inv" ${relay.config.ch0?.inverted ? 'checked' : ''}>
                        <label class="checkbox-label" for="relay_${index}_ch0_inv">Lógica Invertida (Activo en BAJO)</label>
                    </div>
                </div>
            </div>
            <div style="background:#f0f7ff; padding:12px; border-radius:4px; margin-bottom:8px; border-left: 3px solid var(--altermundi-blue);">
                <strong style="font-size:13px; color:var(--altermundi-blue); display:block; margin-bottom:10px;">Configuración Canal 2</strong>
                <div class="inline-group">
                    <div class="form-group">
                        <label>Max ON (ms)</label>
                        <input type="number" id="relay_${index}_ch1_max" value="${relay.config.ch1?.max_on_ms || 0}" placeholder="0 = Infinito">
                    </div>
                    <div class="form-group">
                        <label>Min OFF (ms)</label>
                        <input type="number" id="relay_${index}_ch1_min" value="${relay.config.ch1?.min_off_ms || 0}" placeholder="0 = Sin límite">
                    </div>
                    <div class="form-group" style="padding-top:25px;">
                        <input type="checkbox" id="relay_${index}_ch1_inv" ${relay.config.ch1?.inverted ? 'checked' : ''}>
                        <label class="checkbox-label" for="relay_${index}_ch1_inv">Lógica Invertida (Activo en BAJO)</label>
                    </div>
                </div>
            </div>`;
        }

        relayDiv.innerHTML = `
            <div class="hdr" style="display:flex; justify-content:space-between; margin-bottom:10px;">
                <div class="form-group" style="margin-bottom:0;">
                    <input type="checkbox" id="relay_${index}_enabled" ${relay.enabled ? 'checked' : ''}>
                    <label class="checkbox-label" for="relay_${index}_enabled"><strong>Habilitado (${relay.type === 'gpio' ? 'GPIO' : 'Modbus'})</strong></label>
                </div>
                <button type="button" style="background:none; border:none; color:#dc3545; cursor:pointer;" onclick="removeRelay(${index})">🗑️</button>
            </div>
            ${configHtml}
        `;
        container.appendChild(relayDiv);
    });
}

function addRelay(type = 'relay_2ch') {
    if (!currentConfig.relays) currentConfig.relays = [];
    let newRelay;
    if (type === 'gpio') {
        newRelay = JSON.parse(JSON.stringify(DEFAULT_GPIO_RELAY_TEMPLATE));
    } else {
        newRelay = JSON.parse(JSON.stringify(DEFAULT_RELAY_TEMPLATE));
    }
    currentConfig.relays.push(newRelay);
    renderRelays(currentConfig.relays);
}

function removeRelay(index) {
    if (confirm('¿Eliminar este relé?')) {
        currentConfig.relays.splice(index, 1);
        renderRelays(currentConfig.relays);
    }
}

// Form submission
const form = document.getElementById('configForm');
if (form) {
    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const formData = buildConfigFromForm();

        try {
            const response = await fetch('/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(formData)
            });

            const result = await response.text();

            if (response.ok) {
                showMessage('Configuración guardada correctamente. Algunos cambios requieren reiniciar el dispositivo.', 'success');
            } else {
                showMessage('Error al guardar: ' + result, 'error');
            }
        } catch (error) {
            showMessage('Error de conexión: ' + error.message, 'error');
        }
    });
}

function buildConfigFromForm() {
    const config = { ...currentConfig };

    // WiFi
    config.ssid = document.getElementById('ssid').value;
    const passwd = document.getElementById('passwd').value;
    if (passwd) config.passwd = passwd;

    // Sistema
    config.incubator_name = document.getElementById('incubator_name').value;
    config.min_temperature = parseFloat(document.getElementById('min_temperature').value);
    config.max_temperature = parseFloat(document.getElementById('max_temperature').value);
    config.min_hum = parseInt(document.getElementById('min_hum').value);
    config.max_hum = parseInt(document.getElementById('max_hum').value);

    // RS485 (nested object structure)
    config.rs485 = {
        enabled: document.getElementById('rs485_enabled').checked,
        rx_pin: parseInt(document.getElementById('rs485_rx').value),
        tx_pin: parseInt(document.getElementById('rs485_tx').value),
        de_pin: parseInt(document.getElementById('rs485_de').value),
        baudrate: parseInt(document.getElementById('rs485_baud').value),
        raw_send_enabled: document.getElementById('rs485_raw_send').checked
    };
    // Remove old flat fields if they exist (migration cleanup)
    delete config.rs485_enabled;
    delete config.rs485_rx;
    delete config.rs485_tx;
    delete config.rs485_de;
    delete config.rs485_baud;

    // ESP-NOW
    config.espnow_enabled = document.getElementById('espnow_enabled').checked;
    config.espnow_force_mode = document.getElementById('espnow_force_mode').value;
    config.grafana_ping_url = document.getElementById('grafana_ping_url').value;
    config.espnow_channel = parseInt(document.getElementById('espnow_channel').value);
    config.send_interval_ms = parseInt(document.getElementById('send_interval_ms').value);

    // Sensors
    if (Array.isArray(config.sensors)) {
        config.sensors.forEach((sensor, index) => {
            const enabledCheckbox = document.getElementById(`sensor_${index}_enabled`);
            if (enabledCheckbox) {
                sensor.enabled = enabledCheckbox.checked;
            }

            if (sensor.type === 'onewire') {
                const pinEl  = document.getElementById(`sensor_${index}_pin`);
                const scanEl = document.getElementById(`sensor_${index}_scan`);
                if (!sensor.config) sensor.config = {};
                if (pinEl)  sensor.config.pin  = parseInt(pinEl.value);
                if (scanEl) sensor.config.scan = scanEl.checked;
            }

            if (sensor.type === 'modbus_th' || sensor.type === 'modbus_soil_7in1') {

                if (!sensor.config) sensor.config = {};
                const addrInput = document.getElementById(`sensor_${index}_addresses`);

                // Parse comma-separated addresses into array
                // RS485 bus config is now global, sensor only needs addresses
                if (addrInput) {
                    const addrStr = addrInput.value.trim();
                    const addrArray = addrStr.split(',')
                        .map(s => parseInt(s.trim()))
                        .filter(n => !isNaN(n) && n >= 1 && n <= 254);
                    sensor.config.addresses = addrArray.length > 0 ? addrArray : [1];
                }
                // Remove legacy per-sensor RS485 fields (now global)
                delete sensor.config.address;
                delete sensor.config.rx_pin;
                delete sensor.config.tx_pin;
                delete sensor.config.de_pin;
                delete sensor.config.baudrate;
            }

            if (sensor.type === 'capacitive' || sensor.type === 'hd38') {
                if (!sensor.config) sensor.config = {};
                const nameInput = document.getElementById(`sensor_${index}_name`);
                const pinInput  = document.getElementById(`sensor_${index}_pin`);
                const dryInput  = document.getElementById(`sensor_${index}_dry`);
                const wetInput  = document.getElementById(`sensor_${index}_wet`);
                if (nameInput) sensor.config.name = nameInput.value;
                if (pinInput)  sensor.config.pin  = parseInt(pinInput.value);
                if (dryInput)  sensor.config.dry  = parseInt(dryInput.value);
                if (wetInput)  sensor.config.wet  = parseInt(wetInput.value);
                if (sensor.type === 'hd38') {
                    const vdiv = document.getElementById(`sensor_${index}_voltage_divider`);
                    const inv  = document.getElementById(`sensor_${index}_invert_logic`);
                    if (vdiv) sensor.config.voltage_divider = vdiv.checked;
                    if (inv)  sensor.config.invert_logic    = inv.checked;
                    // Remove legacy multi-pin fields when saving via unified UI
                    delete sensor.config.analog_pin;
                    delete sensor.config.analog_pins;
                }
            }
        });
    }

    // Relays
    if (Array.isArray(config.relays)) {
        config.relays.forEach((relay, index) => {
            const enabledCheckbox = document.getElementById(`relay_${index}_enabled`);
            if (enabledCheckbox) relay.enabled = enabledCheckbox.checked;

            const aliasInput = document.getElementById(`relay_${index}_alias`);
            if (aliasInput) relay.config.alias = aliasInput.value;

            if (relay.type === 'gpio') {
                const pinInput = document.getElementById(`relay_${index}_pin`);
                if (pinInput) relay.config.pin = parseInt(pinInput.value);

                const activeLowCheckbox = document.getElementById(`relay_${index}_active_low`);
                if (activeLowCheckbox) relay.config.active_low = activeLowCheckbox.checked;
                const maxOnInput = document.getElementById(`relay_${index}_max_on`);
                if (maxOnInput) relay.config.max_on_ms = parseInt(maxOnInput.value) || 0;
                const minOffInput = document.getElementById(`relay_${index}_min_off`);
                if (minOffInput) relay.config.min_off_ms = parseInt(minOffInput.value) || 0;
            } else {
                const addrInput = document.getElementById(`relay_${index}_address`);
                if (addrInput) relay.config.address = parseInt(addrInput.value);
                
                relay.config.ch0 = {
                    max_on_ms: parseInt(document.getElementById(`relay_${index}_ch0_max`)?.value || 0),
                    min_off_ms: parseInt(document.getElementById(`relay_${index}_ch0_min`)?.value || 0),
                    inverted: document.getElementById(`relay_${index}_ch0_inv`)?.checked || false
                };
                relay.config.ch1 = {
                    max_on_ms: parseInt(document.getElementById(`relay_${index}_ch1_max`)?.value || 0),
                    min_off_ms: parseInt(document.getElementById(`relay_${index}_ch1_min`)?.value || 0),
                    inverted: document.getElementById(`relay_${index}_ch1_inv`)?.checked || false
                };
            }
        });
    }

    return config;
}
)=====";
const char *rules_html = R"=====(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Reglas - Monitor</title>
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <h1>⚡ Reglas de Automatización</h1>
        <div class="subtitle">Configura condiciones de sensores para activar actuadores automáticamente.</div>
        <div class="nav"><a href="/">🏠 Inicio</a><a href="/data">📊 Datos</a><a href="/settings">⚙️ Configuración</a></div>

        <div class="section">
            <h2>🔌 Estado de Actuadores</h2>
            <div id="actuatorGrid" class="cards">
                <div class="loading">Cargando...</div>
            </div>
        </div>

        <div class="section">
            <h2>📋 Reglas Activas</h2>
            <div id="ruleList">
                <div class="loading">Cargando...</div>
            </div>
        </div>

        <div class="section">
            <h2>➕ Nueva Regla</h2>
            <div class="form-group">
                <label>Tipo de expresión</label>
                <select id="exprType" onchange="toggleExprType()">
                    <option value="LEAF">Condición simple (LEAF)</option>
                    <option value="AND">AND — ambas condiciones</option>
                    <option value="OR">OR — cualquier condición</option>
                </select>
            </div>

            <!-- LEAF -->
            <div id="leafSection" style="background:#f9f9f9; padding:15px; border-radius:6px; margin-bottom:15px; border-left:3px solid var(--altermundi-blue);">
                <div class="inline-group">
                    <div class="form-group">
                        <label>Device ID (0=local)</label>
                        <input type="number" id="leafDevice" value="0" min="0" max="255">
                    </div>
                    <div class="form-group">
                        <label>Sensor ID</label>
                        <input type="number" id="leafSensor" value="0" min="0" max="65535">
                    </div>
                    <div class="form-group">
                        <label>Variable (0=Temp)</label>
                        <input type="number" id="leafVariable" value="0" min="0" max="255">
                    </div>
                </div>
                <div class="inline-group">
                    <div class="form-group">
                        <label>Operador</label>
                        <select id="leafOp">
                            <option value="GT">GT (&gt;)</option>
                            <option value="LT">LT (&lt;)</option>
                            <option value="GTE">GTE (&ge;)</option>
                            <option value="LTE">LTE (&le;)</option>
                            <option value="EQ">EQ (=)</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label>Umbral</label>
                        <input type="number" id="leafValue" value="30" step="0.1">
                    </div>
                </div>
            </div>

            <!-- AND / OR -->
            <div id="compoundSection" style="display:none; background:#f9f9f9; padding:15px; border-radius:6px; margin-bottom:15px; border-left:3px solid var(--altermundi-blue);">
                <label style="color:var(--altermundi-blue); font-weight:bold; margin-bottom:10px;">Condición IZQUIERDA</label>
                <div class="inline-group">
                    <div class="form-group"><label>Device</label><input type="number" id="leftDevice" value="0" min="0" max="255"></div>
                    <div class="form-group"><label>Sensor</label><input type="number" id="leftSensor" value="0" min="0" max="65535"></div>
                    <div class="form-group"><label>Var</label><input type="number" id="leftVariable" value="0" min="0" max="255"></div>
                </div>
                <div class="inline-group">
                    <div class="form-group"><label>Op</label><select id="leftOp"><option>GT</option><option>LT</option><option>GTE</option><option>LTE</option><option>EQ</option></select></div>
                    <div class="form-group"><label>Umbral</label><input type="number" id="leftValue" value="30" step="0.1"></div>
                </div>

                <label style="color:var(--altermundi-blue); font-weight:bold; margin-top:15px; margin-bottom:10px;">Condición DERECHA</label>
                <div class="inline-group">
                    <div class="form-group"><label>Device</label><input type="number" id="rightDevice" value="0" min="0" max="255"></div>
                    <div class="form-group"><label>Sensor</label><input type="number" id="rightSensor" value="0" min="0" max="65535"></div>
                    <div class="form-group"><label>Var</label><input type="number" id="rightVariable" value="0" min="0" max="255"></div>
                </div>
                <div class="inline-group">
                    <div class="form-group"><label>Op</label><select id="rightOp"><option>GT</option><option>LT</option><option>GTE</option><option>LTE</option><option>EQ</option></select></div>
                    <div class="form-group"><label>Umbral</label><input type="number" id="rightValue" value="30" step="0.1"></div>
                </div>
            </div>

            <div class="inline-group" style="margin-top:16px">
                <div class="form-group">
                    <label>ID Actuador</label>
                    <select id="ruleActuator">
                        <option value="0">Cargando...</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Estado al activar</label>
                    <select id="ruleState">
                        <option value="true">ON (true)</option>
                        <option value="false">OFF (false)</option>
                    </select>
                </div>
            </div>
            <div class="inline-group">
                <div class="form-group">
                    <label>Prioridad</label>
                    <select id="rulePriority">
                        <option value="1">1 — Timer</option>
                        <option value="2" selected>2 — Regla</option>
                        <option value="3">3 — Manual</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Duración (ms, 0=indefinido)</label>
                    <input type="number" id="ruleDuration" value="0" min="0">
                </div>
            </div>

            <div class="nav-buttons">
                <button class="btn" onclick="addRule()">💾 Agregar Regla</button>
                <button class="btn btn-secondary" onclick="saveRules()">☁️ Guardar en dispositivo</button>
            </div>

            <div class="message" id="toast"></div>

            <!-- REFERENCE PANEL -->
            <div style="margin-top: 25px; padding: 15px; background: #e3f2fd; border-left: 4px solid var(--altermundi-blue); border-radius: 8px;">
                <h3 style="margin-top:0; font-size:16px; color:var(--gray-dark)">🔑 Referencia de IDs (En vivo)</h3>
                <button class="btn btn-secondary" style="margin-bottom:10px; padding:6px 12px; font-size:12px;" onclick="loadReferencePanel()">🔄 Actualizar Referencias</button>
                <div id="refPanelContent" style="font-size:13px; max-height: 250px; overflow-y: auto; background:#fff; padding:12px; border-radius:6px; font-family:monospace; border:1px solid #ddd;">
                    Cargando referencias...
                </div>
            </div>
        </div>
    </div>

    <script>
        let currentRules = [];

        function showMessage(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'message ' + type;
            t.style.display = 'block';
            setTimeout(() => { t.style.display = 'none'; }, 3000);
        }

        function toggleExprType() {
            const v = document.getElementById('exprType').value;
            document.getElementById('leafSection').style.display = (v === 'LEAF') ? '' : 'none';
            document.getElementById('compoundSection').style.display = (v !== 'LEAF') ? '' : 'none';
        }

        function priLabel(p) { return ['','Timer','Regla','Manual'][p] || p; }
        function priBadge(p) {
            const colors = ['', 'var(--altermundi-green)', 'var(--altermundi-orange)', 'var(--red-error)'];
            return `<span class="status-badge" style="background:${colors[p]};color:#fff;">${priLabel(p)}</span>`;
        }

        function exprSummary(expr) {
            if (!expr) return '?';
            const op = expr.op || 'LEAF';
            if (op === 'AND' || op === 'OR') {
                return `(${exprSummary(expr.left)} ${op} ${exprSummary(expr.right)})`;
            }
            const s = expr.sensor || {};
            return `[D${s.device||0}:S${s.id||0}:V${s.var||0}] ${expr.cond||'?'} ${expr.value}`;
        }

        async function loadRules() {
            try {
                const r = await fetch('/rules');
                const d = await r.json();
                currentRules = d.rules || [];
            } catch(e) { currentRules = []; }
            renderRules();
        }

        async function loadReferencePanel() {
            const el = document.getElementById('refPanelContent');
            el.innerHTML = "Cargando...";
            try {
                const r = await fetch('/actual');
                const d = await r.json();
                let relays = [];
                try {
                    const rr = await fetch('/api/relays');
                    relays = await rr.json();
                } catch(e) {}

                let html = "<div style='display:flex; gap:20px; flex-wrap:wrap;'>";
                
                // Sensores
                html += "<div style='flex:1; min-width:200px'><b style='color:var(--altermundi-green)'>📡 Sensores</b><br>";
                if (d.sensors && d.sensors.length > 0) {
                    d.sensors.forEach(s => {
                        html += `<div style="margin-top:8px; border-left:2px solid #ddd; padding-left:8px;">`;
                        html += `<strong>${s.type || 'Sensor'}</strong> <span style="color:#888;font-size:11px;">(${s.id || 'N/A'})</span><br>`;
                        if(s.readings) {
                            s.readings.forEach(rd => {
                                if(rd.key_device !== undefined) {
                                    html += `Dev:<b style="color:var(--altermundi-green)">${rd.key_device}</b> | Sen:<b style="color:var(--altermundi-blue)">${rd.key_sensor}</b> | Var:<b style="color:var(--altermundi-orange)">${rd.key_var}</b> &rarr; ${rd.label} (${rd.value} ${rd.unit})<br>`;
                                }
                            });
                        }
                        html += `</div>`;
                    });
                } else {
                    html += "Ninguno<br>";
                }
                html += "</div>";

                // Actuadores
                html += "<div style='flex:1; min-width:200px'><b style='color:var(--altermundi-orange)'>⚡ Actuadores</b><br>";
                let hasActuators = false;
                if (d.actuators && d.actuators.length > 0) {
                    hasActuators = true;
                    d.actuators.forEach(a => {
                        html += `<div style="margin-top:6px;">ID: <b style="color:var(--altermundi-orange)">${a.id}</b> &rarr; ${a.name} (Estado: ${a.state})</div>`;
                    });
                }

                if (relays && relays.length > 0) {
                    hasActuators = true;
                    relays.forEach(r => {
                        let isGpio = r.type === 'gpio';
                        html += `<div style="margin-top:8px; font-weight:bold; color:var(--altermundi-blue)">Módulo: ${r.alias || (isGpio ? 'GPIO '+r.address : 'Relé Modbus '+r.address)}</div>`;
                        if (r.state) {
                            r.state.forEach((st, idx) => {
                                let actId = isGpio ? (r.address + 200) : ((r.address << 4) | idx);
                                let label = isGpio ? 'Estado' : ('Canal ' + (idx+1));
                                html += `<div style="padding-left:12px; font-size:12px; border-left:1px solid #ddd;">ID: <b style="color:var(--altermundi-orange)">${actId}</b> &rarr; ${label} (Estado: ${st ? 'ON' : 'OFF'})</div>`;
                            });
                        }
                    });
                }

                if (!hasActuators) {
                    html += "Ninguno<br>";
                }
                html += "</div></div>";
                el.innerHTML = html;
            } catch(e) {
                el.innerHTML = "<span style='color:red'>Error cargando referencias</span>";
            }
        }

        function renderRules() {
            const el = document.getElementById('ruleList');
            if (!currentRules.length) {
                el.innerHTML = '<div class="info-text">Sin reglas. Crea una con el formulario.</div>';
                return;
            }
            el.innerHTML = currentRules.map((r, i) => `
                <div class="sensor-item" style="border-left-color: var(--altermundi-blue);">
                    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
                        <div>
                            <span class="status-badge" style="background:#e3f2fd; color:var(--altermundi-blue); border:1px solid #b3d4fc;">Regla ${i+1}</span>
                            ${priBadge(r.priority||2)}
                        </div>
                        <button style="background:none; border:none; color:var(--red-error); cursor:pointer; font-size:16px;" onclick="deleteRule(${i})">🗑️</button>
                    </div>
                    <div style="font-size:14px; margin-bottom:6px;">Actuador <b>${r.actuator}</b> &rarr; <b style="color:${r.state ? 'var(--altermundi-green)' : 'var(--gray-medium)'}">${r.state ? 'ON':'OFF'}</b>
                        ${r.duration_ms ? `(${r.duration_ms}ms)` : '(hold)'}
                    </div>
                    <div style="font-size:12px; color:var(--gray-medium); background:#f5f5f5; padding:6px; border-radius:4px; font-family:monospace;">
                        Condición: ${exprSummary(r.expr)}
                    </div>
                </div>
            `).join('');
        }

        function deleteRule(idx) {
            currentRules.splice(idx, 1);
            renderRules();
            showMessage('Regla eliminada (guarda para aplicar)', 'warning');
        }

        function buildLeafExpr(device, sensor, variable, op, value) {
            return { sensor: { device: parseInt(device), id: parseInt(sensor), var: parseInt(variable) }, cond: op, value: parseFloat(value) };
        }

        function addRule() {
            const exprType = document.getElementById('exprType').value;
            let expr;

            if (exprType === 'LEAF') {
                expr = buildLeafExpr(
                    document.getElementById('leafDevice').value,
                    document.getElementById('leafSensor').value,
                    document.getElementById('leafVariable').value,
                    document.getElementById('leafOp').value,
                    document.getElementById('leafValue').value
                );
            } else {
                expr = {
                    op: exprType,
                    left: buildLeafExpr(
                        document.getElementById('leftDevice').value,
                        document.getElementById('leftSensor').value,
                        document.getElementById('leftVariable').value,
                        document.getElementById('leftOp').value,
                        document.getElementById('leftValue').value
                    ),
                    right: buildLeafExpr(
                        document.getElementById('rightDevice').value,
                        document.getElementById('rightSensor').value,
                        document.getElementById('rightVariable').value,
                        document.getElementById('rightOp').value,
                        document.getElementById('rightValue').value
                    )
                };
            }

            const rule = {
                expr,
                actuator: parseInt(document.getElementById('ruleActuator').value),
                state: document.getElementById('ruleState').value === 'true',
                priority: parseInt(document.getElementById('rulePriority').value),
                duration_ms: parseInt(document.getElementById('ruleDuration').value)
            };

            currentRules.push(rule);
            renderRules();
            showMessage('Regla agregada (guarda para aplicar)', 'success');
        }

        async function saveRules() {
            const payload = JSON.stringify({ rules: currentRules });
            try {
                const r = await fetch('/rules/save', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: payload
                });
                const txt = await r.text();
                if (r.ok) showMessage('Reglas guardadas y aplicadas', 'success');
                else showMessage('Error: ' + txt, 'error');
            } catch(e) { showMessage('Error de conexión', 'error'); }
        }

        async function loadActuators() {
            try {
                const [actRes, relRes] = await Promise.all([
                    fetch('/actuator/status').catch(() => ({ json: () => [] })),
                    fetch('/api/relays').catch(() => ({ json: () => [] }))
                ]);
                
                let d = [];
                try { d = await actRes.json(); } catch(e) {}
                let relays = [];
                try { relays = await relRes.json(); } catch(e) {}

                let combinedList = [];
                let selectOptions = '';

                if (relays && relays.length > 0) {
                    relays.forEach(r => {
                        let isGpio = r.type === 'gpio';
                        if (r.state) {
                            r.state.forEach((st, idx) => {
                                let actId = isGpio ? (r.address + 200) : ((r.address << 4) | idx);
                                let name = (r.alias || (isGpio ? 'GPIO ' + r.address : 'Relé ' + r.address)) + (isGpio ? '' : ' - Canal ' + (idx + 1));
                                combinedList.push({ id: actId, name: name, state: st });
                                selectOptions += `<option value="${actId}">${name} (ID: ${actId})</option>`;
                            });
                        }
                    });
                }
                
                if (d && d.length > 0) {
                    d.forEach(a => {
                        if (!combinedList.find(c => c.id === a.id)) {
                            combinedList.push({ id: a.id, name: a.name || ('Act ' + a.id), state: a.state });
                            selectOptions += `<option value="${a.id}">${a.name || ('Act ' + a.id)} (ID: ${a.id})</option>`;
                        }
                    });
                }

                const sel = document.getElementById('ruleActuator');
                if (sel) {
                    const currentVal = sel.value;
                    sel.innerHTML = selectOptions || '<option value="0">Sin actuadores</option>';
                    if (combinedList.find(c => c.id == currentVal)) {
                        sel.value = currentVal;
                    }
                }

                renderActuators(combinedList);
            } catch(e) {
                document.getElementById('actuatorGrid').innerHTML = '<div class="info-text">No disponible.</div>';
            }
        }

        function renderActuators(list) {
            const g = document.getElementById('actuatorGrid');
            if (!list || !list.length) {
                g.innerHTML = '<div class="info-text">Sin actuadores registrados.</div>';
                return;
            }
            g.innerHTML = list.map(a => `
                <div class="card ${a.state ? '' : 'warn'}" style="min-width: 150px; text-align: center; border-left-color: ${a.state ? 'var(--altermundi-green)' : 'var(--gray-medium)'};">
                    <div style="font-size:14px; font-weight:bold; color:var(--gray-dark); margin-bottom:10px;">${a.name || 'Act '+a.id}</div>
                    <div style="font-size:20px; font-weight:bold; margin-bottom:15px; color:${a.state ? 'var(--altermundi-green)' : 'var(--gray-medium)'};">
                        ${a.state ? '● ON' : '○ OFF'}
                    </div>
                    <div style="display:flex; justify-content:center; gap:5px;">
                        <button class="btn" style="padding:6px 10px; margin:0; font-size:12px; background:var(--altermundi-green);" onclick="sendCmd(${a.id},true,0,3)">ON</button>
                        <button class="btn" style="padding:6px 10px; margin:0; font-size:12px; background:var(--red-error);" onclick="sendCmd(${a.id},false,0,3)">OFF</button>
                        <button class="btn" style="padding:6px 10px; margin:0; font-size:12px; background:var(--altermundi-blue);" onclick="sendCmd(${a.id},false,0,0)">AUTO</button>
                    </div>
                </div>
            `).join('');
        }

        async function sendCmd(id, state, duration, priority = 3) {
            try {
                const r = await fetch('/actuator/command', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ id, state, duration, priority })
                });
                if (r.ok) {
                    showMessage(`Actuador ${id} → ${state?'ON':'OFF'}`, 'success');
                } else {
                    showMessage('Error al enviar comando', 'error');
                }
                setTimeout(loadActuators, 500);
            } catch(e) { showMessage('Error de conexión', 'error'); }
        }

        window.addEventListener('load', () => {
            loadRules();
            loadActuators();
            loadReferencePanel();
        });
        setInterval(loadActuators, 5000);
    </script>
</body>
</html>
)=====";

const char *favicon_svg = R"=====(
<svg xmlns="http://www.w3.org/2000/svg" width="65" height="30" fill="none" viewBox="0 0 65 30"><path fill="#000" fill-opacity=".5" d="m34 15.2.7.2q.2.1.3.4.3.2.8 1.4l.4.7q.2.9.4.9l.5-.8 1.3-2.3.3-.2.2-.2h.1l.5-.1q.9 0 .9.3v4l.2 2.7v.4l-.1-.5q0 .3.4.3.3 0 .3.3l-.1 1-.5-.2h-.8q-1.2 0-1.2-.2v-.6l.4-.2V22l.1-.7v-3.7l.2-.4-.1-.7q-.3.2-.8 1-.5 1-1 1.6l-.5.8h-.6l-.2-.2q0-.3-.4-.6l-.8-1.5-.1-.3-.2-.3.2.2q0-.5-.3-.8h-.2v.4l.2 5v.1l-.1-.2-.1.3.2.2v.2l-.3-.1v.1l.5.2h.2q.6 0 .6.2h-.1l-.3-.1h-.4l.1.2h.6v.2q0 .3-.2.3l-.5-.1h.2l.1.1-1.6-.1H33l-.3-.1-.2-.4v-.1q.2-.3.5-.3h.4v-.1l-.2-6.1v-.8q0-.3.2-.3zm4.9.6.2-.2v-.1zm-5-.2h.2zm.6 0h.2zm-.8 0 .2.6V16zm5 .3.2-.1q-.2 0-.1.1m-5.4-.1.1.6v-.3zm1 .1.5.7.5 1-.5-1.3-.3-.4zm-.6.6v.7q.2 0 .2-.3v-.4zm3.7 1.4.3-.5.2-.4q-.2 0-.5.9m2.2-.5v-.1m.3.6q0 .8.2.9v-1.2q-.2 0-.2.3m-6.5-.3v.3zm6 .4.1.1V18zm-5.4.5v.7h.1v-1zm2.6.5v.2zm-3.1.2v.4zm.4.4v.2zm-.5 1 .1-.4v-.5zm.6-.5v1.6l.2-.2-.1-.2v-.9zm5.6 1-.2.4.1.2zm.4.2v.5h.1zm-6.6.2v.2l.1-.2v-.1zm5.6.4v.2zm.8 0h.1zm-5.9.3h.1zm-1.5.4.1.2h.1v-.3zm7.1-.4h.1zm-7 1h.1zm1 0h.3zm.4 0 .2.1h.2-.3zm4.6-.4h.3-.2zm0 .3.1.1h.2-.1zM42 16l.7.1h.2l.5.1-.1.1.2.1-.2.4h-.7v1l-.1 1.1v1.2-.2h-.1V21l.1-.8v1.2l.1.3q.2 0 .6.6.2.2.7.2l.3.1h.1q.4 0 .6-.4l.5-.9q.2-.1.2-.5v-1.5.1l.1-.4h-.1v-1q0-1.1-.3-1.1-.6 0-.6-.2l-.1-.3q.2-.4.7-.4h.8l.6.1v.3q0 .3-.3.5l.1 2q-.2.4-.2 1.8l-.4 1.6-.2.3-.1.1-.2.3q-.2.2-.6.2h-1q-1.2 0-1.7-.5-.2 0-.2-.2v-.1l-.4-1.2v-.9l-.1-.2.1-.4v-.5l.2-1.5V17q0-.2-.4-.2v-.6q0-.2.7-.2m4.3.5h.2zm-1.2 0h.3zm-3.5.1h.1zm.4.3v.1h.1zm.3 0-.1.3.1-.1V17zm-.2 1 .1-.2zm3.8 0v.3zM42 18v.3-.2zm.3.5v.2l.1-.3V18zm3.7.2v.3zm-4 .9v.5zm3.9.4h.1zm0 .6v-.3zm0 .2v-.1zm-3.9 1 .1.3v.2l.1.1V22zm.6.2.3.4-.1-.3zm2.6.6q.3 0 .3-.2-.2 0-.3.3m-3 0 .2.2zm.8 0h.1zm1.9.1h.1zm-1.1.1h.1zm.2.1h.2zm.8.2h.1l.2-.1zm8.4-7.8q.2 0 .2.3 0 .4-.3.4h-.4v.1L53 22v1l-.1.5h-.7l-.3-.1q0-.3-.3-.7l-.4-1.2-.2-.2-1.1-2-.5-1-.7-1.1v1.7l-.2 2.6v1.3l.1.3v.2h-.2v.1l.2.1q0 .2-.2.2h-1q-.3 0-.3-.2l-.1-.6.5-.3v-.4l.1-.2v-1q.2-2.3.2-5.4v-.3h.7q.2 0 .6.6l.7 1 .5.8q.6.7 1 1.8.3.8.5.8l.1-.4v-3.8l-.6-.1v-.4q0-.4.3-.4zm-4.9.1q0 .2.2.2v-.1zm3.2.5h.3zm.6.4v.1h.1v-.2zm-3.8.7v-.6zm.3-.5.1.3zm.5.5.4 1-.2-.7zm.7.7q0 .2.3.5zm-.1.5q.1.6.7 1.4zm1 .6q0 .4.2.6zm.3.7q0 .4.2.6zm-.5.3q0 .5.2.5zM48 21.1v-.6zm3-.2.2.2V21zm1.3.4v.2h.1V21zm-4.5.1v.2l.1-.2zm3.7.8.1.3zm-3.7.3v.1h.1zm-.6.3h.2zm5.6.1v.1zm-5.8.2.2.1h.1-.3m.2.4h.2zm8.1-7.8h.4q.6 0 1.6.3.9.4 1 .6l.5.4.1.1-.4-.2q0 .2.3.4l.5.5.3.9.2.2v-.2q.4 1 .4 1.4v.8l-.3.6-.5.9.3-.4h.1l-.3.4-1.4 1q-.3.3-.8.4l-.8.1h-1.4l-.6-.2h-.4v.1H54l-.1-.2V23h1.2v-1.4l.1-3.7v-.5l-.1-.3h-1.5v-.9q.2-.2.6-.2zm-.2.1h.5zm-.6 0 .4.2h.1l-.2-.2zm1.7.2.2.1zm-2.3.2h.3-.2zm2.7 0h.1zm.5 0 .9.4h.1l-.8-.4zm-1.8.1q0 .2.6.2l.1-.2h.3zm1.7 0 .5.3zm-2.9.1h.1zm3.6.2v.1l.3.2.1.1h.3-.2V17zm-3.7 0 .2.1zm.3.1h.2zm1.7.4v1.3-.2.5h.1V21q0 1.3-.2 2l.9.2q.6 0 1.4-.4l.7-.8q.2-.3.3-1.2v-.1l-.1-1.3-.5-1.1-.7-.8-1.5-.5q-.3 0-.3.2m1.2-.2.3.2zm1 .3.2.1zm.8.1.4.6.1.2-.3-.3zm-.7.1q0 .2.3.4v-.1zm-3 .4v.2zv-.4zm4.2.4.2.4zm-.6 0v.2l.3.8q-.1-.7-.3-1m-3.7.1v.1zm-.1 1.8v.6h.1v-2zm4.4-1v.6h.1zm-.4.8v.3l-.1.8.2-.3zm.9 1.2-.3.7zm-1 .5.2-.1v-.1zm-3.5 0v.2zm3 .9q.6-.5.7-.8-.5.2-.7.7zm-3.4 0V22zm3 0v-.1zm-1.1.9q.5 0 1.5-.7l-1 .4zM54 23v.4-.1l.2-.1h.1V23l-.2-.1zm.8.4.5.3h.5v-.1zm1.3.1.4.1h.1-.5zm0 .3.3.1h.2-.1z"/><path fill="#000" fill-opacity=".5" d="M61.2 15.7h.2l1.7-.1q.1.2.3.2.2.2.2.7 0 .3-.6.4-.5 0-.5.3v1.7l-.1.2v1.1l.2 2-.1.6.1.1 1.2-.1q.3 0 .3.3v.1q0 .7-.2.7h-3.3q-.2 0-.2-.5V23h.2v-.2h1v-1.3l-.3-4.6h-1.1l-.2-.4v-.9zm.1.4h.1zm-1 .7h.2zm1.6.4.1-.2zm.5 6.1h.1zm1.1.2h.2zm-28-17.2q.7.1.7.3l.6 1.8.4 1.5.8 3 .3.5v.5h.6l.2.1-.1.2v.2l-.1.3-.2-.1h-.2l.2.1-.1.1h-1.1q-.3 0-.7-.3l.1-.1V14q0-.2.4-.2 0-.6-.5-1.3l-3.1.3h-.3l-.3 1h.4q.4 0 .7.4l-.3.3q0 .2-2 .2l-.2-.1v-.4l.1-.2v-.2l.2-.1v.2l.2-.3v-.2l.1-.6.7-1.8.2-.6.2-.3V10l.1-.2.2-.3.5-1.7.4-1 .2-.2zm.2.2h.1zM33.6 12l2.6-.2h.5v-.1q0-.5-.2-.5L35.8 9l-.2-.9-.2-.2v.3L34 10.5zm.2-2.2v.3zm2.9 1.7h-.1zm-1 .5h.3zm-1 .2h.1zm3.2.4-.6.2v-.1zm-5.6.5h.1zm5 0h.5zm1 1.3.2-.1zm-1.1.4h.1z"/><path fill="#000" fill-opacity=".5" d="M39.6 6q1 0 1.7.3h.4v.8h-.1l-.2-.1H41h.2v.1h-.3l-.1 2.5v3.9h3.1l.4.1.1-.7q0-.2.2-.2.7 0 .7.2H45v.1l.2.2-.1.8v.5l-.2.2h-6v-.3q0-.7.3-.7l.5-.1V10l.1-2.7q0-.3-.3-.3l.3-.2h-.5V7h-.1l-.1-.3.1-.4.1-.1zm1.6.5h-.3v.1zl.2-.1zm-1 0q.1.2.3.2h.1v-.1h-.3m.2.2h.1zm0 .3v.3h.1v-.1zm-.1 2.5v.7zm4.6 3.6v.3l-.2.5v.1h.1q.2 0 .2-.7zm-4.7 0v.6l.1-.4zm3.4.5h.2q0 .2.2.2h.3l-.6-.2zm-.5.1h.3-.1zm-4 .2h.1v-.1zm1.2 0 2.3.2zM39 14h.1zm4.3.1h.3zm.5.2.3-.1zm.2.1.3.2.1-.1h.4-.6zm-3.5.1q0 .2.6.2h.6v-.1h-1zm1.7 0h.1z"/><path fill="#000" fill-opacity=".5" d="m45.7 5.7 2.4.2h3q.2.6.2 1.4 0 .4-.2.4l.1.2-.4.2q-.3 0-.3-.2v-1h-2q-.2 0-.2 2l-.1 2.5v.4l-.1.3v1.5l1.3.2q.3.1.3.3l-.1.6v.1h-4l-.1-.8h.2l-.1-.1.6-.2h.9l.2-.9V12l-.1-1.4L47 8V6.9q0-.2-1-.2l-.7.1-.2 1h-.8V6.3q0-.2.2-.2l.2-.1.1.1q.3-.3.7-.3zm2.8.6h.6zm1.6.3h.2zm-2.4.3q.2 0 .3-.2zm-2.9 0h.1v-.1zm2.5 0v.2q.2 0 .2-.2v-.2zm.5.5v.3zm.1 1.2v1.1h.1V8.5zm-.4 1.4v1.4zm.1 1.8v.2l.1.2zm-1.3 2.1h.4l-.2-.1zm-.3 0h.1zm2.9.2h.5zm-.3.2.1.1h.2l-.2-.1zm-1.3 0h.3-.2zm-1 0h.5-.3zm-.4.3.1-.1zm3.4 0"/><path fill="#000" fill-opacity=".5" d="M53.2 6.9h2.7V7l.5-.1h.1V7l.1 1.2-.2.5H56q-.4 0-.4-.2l.2-.2q-.2 0-.2-.2l-.1-.2v-.1h-.4.1V8H53l-.7.1.1 2.2h.8l1 .2.1-.4.3-.2q.2 0 .2 1.2v.6l-.5-.1-.1-.4-1-.1h-.8v.2l.1 1.2-.1 1.5 1.4-.1h2v-.2h-.2l-.1-.2.4-.2.3.1.1.3V15l-.6-.1-4.2-.2q-.3 0-.3-.2v-.8h.3l.2-.1V11l-.2-.3.1-2.2v-.3q0-.3-.3-.3l-.3-.1h-.2q-.2 0-.2-.5t.3-.4zm-2.3.1.1.2h.2V7zm.6.2h.1zm.2 0 .1.1h.2zm.6 0h.1zm2.7.1.7.1-.3-.1zm-2.2 0h.2zm-.6.3a.1.1 0 0 0 .2.1h.4l-.3-.1zm1 0h.1zm-1.9 0h.3zm.5 3.8v.2zm.3 1.7v.3zm3.8.4h.1m-4 .7h.3-.2zm.9 0h.1zm2.6 0h.2zm-2.3 0h1-.7zm-1.6.1.1.1h.1zm3 0 .6.1H55zm1.2.2.2.1zm-1 .2h.4-.3m-1 0h.2zm-1 0h.1zm5.8-8.9 1 .2q1 0 2.7.5.7.6 1 1.3l.2.9q0 .6-.5 1.3-.3.4-.9.7l-.6.4-.2.2 1.5 2.2q.4.6.7.6 1 0 1 .3l.1.6h-.2l-.7.2H63q-.7 0-.7-.2l-.1-.5v-.1l-.4-.7-.1-.3-1-1.7q0-.2-.2-.2l-.8.2q-.6 0-.6.4v.2q0 1.7.3 1.7h.6l.1.5v.3q0 .2-1 .3h-1.8l-.1-.4v-.2q0-.5.2-.5l.6-.1v-4L57.8 7l-1 .1h-.2v-.6l.1-.1v-.1l1-.3zm0 .3h.3-.1zm.2.4h.5zm1 .1h.4-.3m1.6 0h.1m-5 .2h.2v-.1h-.1zm5.3 0 .7.4q0-.2-.7-.4m-3 .1v3.4l.1.5 1-.3 1-.4q.6 0 1.4-.8.3-.4.3-.8v-.2l-.4-.7q-.7-.7-1.3-.7zM58 7v.3l.1.7.1-1zm.8.7V8h.1v-.2zm-.3.4-.1.9h.1zm4.6.3h.1zm-5 .3v.1zm5.1.4-.1.3v.1q.2 0 .2-.2zm-.3.7.1.1.2-.2zm-.3.2h.1zm-.9.3h.1l.3-.1V10zm-3.3 0V10zm2.7.4q.4 0 .5-.3-.4.1-.5.3m1-.2h.1zm-.2.1h.2zm-1.4.3h.3v-.1zm-.5.2h.3V11zm1 .2.3-.1zm.3.9v.2h.1zm-2.8 1.7v.2l.1-.2zm.4.7.1-.2V14zm-.3-.2v.2zm4.3.4.1.1v-.1zm1.4 0h.1zm-4.5.2h.1v-.1m3.3 0h.2zm1.1.2h.4-.3m-1.9 0h.1z"/><path fill="#55d400" fill-opacity=".5" d="m22.6 4.7-.3 1q.3 0-.2 1l-.2.3-.3.5-.3.3-.2.2v.3h.1l.5-.4-.4 1.3-.3.1-.2-.1h-.2v.3l-.1.1-.1.2-3.5 6.2-3.2 4.2h-.1l.3.2.5-.4-.2.3-.1.2h.2l.5-.3h.1l.1.1q.2 0 .8-.8v-.2h-.2l-.2.2h-.1l.2-.3q.3-.7.7-.9v.3h.1l.2-.3.7-1v.2q-.7 1.6-2.1 3.2l-.1.2.2.1.4-.4a25 25 0 0 0 3.2-5.2l1.6-2.6.7-1 .2-.3.7-1.8.6-1.4L23.9 5l.1-.4zm.1 0 .1.2v.2h-.1V5zM21 8.6q-.2.3 0 .4zm-1 4.1-.1.1-.8 1.2.1-.5q.5-.8.7-.8m-1.5 1.8v.2h-.1zm-.4.8q-.3.9-.8 1.3.6-1.3.8-1.3m-2.5 3.5q-.2.4-.5.5v-.1q.3-.5.5-.5m-2 1.3-.3.1.4.2zm1.2.8"/><path fill="#55d400" fill-opacity=".5" d="M21.1 2.7q-.4 0-.3.6-.1.7-.3 1h.3v.1l-.1.1h-.2l1.2.4L24 7.5q1.4 2.7 1.8 4.1v.1h.1v.2l.2.1h.5v.9l-.4-.3q0 .2-.2.2v.1h.1l.3.2v.3l.2.2q.2.7-.2.8-.1 1.6-.4 2l-.3 1.5q-.5 1.3-1 2l.1.1h1.4q.1-.2.6-.3.2-.1.5-1.4.4-1 .6-3.6l-.2-2.2-.2-.9-.1-1.2-.1-.2q0-.3-.5-.6-.1-.7-1-1.7-1.6-3.4-1.9-3.3l-1.2-1q-.9-.8-1.6-.9m3.6 3.8-.4-.6-.3.1-.2.2q-.4-.2-.5-.6l-.1-.3h.1l.2.2.2-.1q-.4-.6-.6-.6l-.1.1h-.2q-.3 0-.7-.7l.5.2-.3-.3V4h.1q.8.2.9.6 1 1 1.3 2m-3.2-2.3h.3l.5.6q.2.1.1.2l-.5-.4q-.6-.3-.7-.5h.1zm.1.7h-.2q-.3 0-.4-.2 0-.2.2-.3t.4.4m1.3.4h-.2zm.3.5V6q-.2 0-.3-.3v-.1q.3 0 .3.3M25 8.1q-.2 0-.6-.8.4.3.6.8m.2.4H25zm1 1.2q-.2 0-.5-.4V9q.4.3.6.7m-.2 2.5q-.1 0 0 .2zm.6 2.5-.1.2zm.3.8.2.3q0 .7-.2.7h-.1l-.3-.1q.1-.9.4-1m-.3 1.2.1.1q0 .5-.3.6v-.1zm-.7 1.9v.2l.2.1q0 .2-.5.4 0-.4.3-.7m0 1.1-.1.2h-.1l-.2-.1z"/><path fill="#ffb600" fill-opacity=".5" d="M3.7 5.8q0 .4.5.5l1 .5.2-.3.1.2V7l.7-1.2L9.5 4q3-1 4.7-1l.1-.1h.2l.2-.1v-.5l1 .1-.4.4q.2 0 .2.2h.2q0-.3.2-.3h.6q.8-.1.7.3 1.7.4 2.1.8l1.5.6 2 1.4.1-.1V5l.3-.6v-.2Q23 4 23 3.5q0-.3-1.4-.8-1-.6-3.7-1.4L15.6 1H13q-.3 0-.7.3-.8 0-2 .7-4 1-4 1.3L5 4.4q-.9.7-1.2 1.4m.9-.6v.2h-.1zm.4-.1v.1H5zm3.6-2.2h.1l-.8.4-.2.1.2.3q-.2.3-.8.4H7V4l.2-.2v-.2q-.7.3-.7.5v.3q0 .3-1 .7 0-.2.4-.6l-.5.3v-.2q.4-.7.8-.8 1.4-.9 2.5-1m-3.2 3v-.3q0-.2.8-.5.2-.2.2 0l-.5.4q-.5.6-.7.6zm.8 0 .1.2q-.2.4-.4.4T5.7 6zm.5-1.3v.2zm.6-.2q0 .2-.3.3H7q.1-.3.4-.3m2.8-1.4q0 .3-1 .5.4-.4 1-.5m1.9-1q0 .4-.6.6l-.3-.2q.4-.3.8-.3m2.7.7.2.1zm2.8 0h.1v.1zm.8-.3h.4q.7 0 .7.2v.2l-.2.3q-.9-.3-.9-.7m1.1.6h.2q.5 0 .5.3-.4 0-.7-.3m1.9 1.2h.3l.1-.2q.2 0 .3.6-.4 0-.7-.4m1.2.1.1.3-.1.2v-.3z"/><path fill="#55d400" fill-opacity=".5" d="M4.8 4.6q.4 0 .4.4.3.7.6 1l-.3.1q0 .2.2.1h.2l-1.2.7-1.5 3.2q-.8 3-.8 4.5v.4l-.2.1H2l-.3.1q.1.9.3.9l.3-.4q0 .2.2.2v.1l-.3.3v.5q0 .8.3.7.6 1.6 1 2l.6 1.3 1.5 1.9-.2.1H5l-.6.3h-.2l-.7-.1q-.3-.1-.8-1.3-.7-1-1.6-3.5L.8 16l-.1-1q-.2-.3-.1-1.2v-.2l.2-.7q0-.7.6-2 .7-3.7 1-3.8.4-.7 1-1.3.7-1 1.4-1.2m-.6.8h.2v-.1zm-2 3.9v.1l.3-.7.1-.2.3.1q.3-.2.3-.7v-.2l-.2.2h-.2q.2-.7.4-.8h.3q.3 0 .6-.8v-.1l-.5.3.2-.4h-.1q-.7.4-.7.8-.8 1.3-.9 2.4m2.7-3.1h-.3q-.2 0-.4.7-.2.2 0 .3l.4-.5Q5 6.2 5 6zm0 .7L5 7h.2q.3-.2.3-.4t-.3-.2zm-1.1.7H4zm-.2.6q.2 0 .3-.3v-.1q-.3.1-.3.4m-1.2 2.7q.2 0 .3-1-.3.4-.3 1m-.1.4v.1h.1zm-.7 1.5q.2 0 .4-.5l-.1-.3q-.3.4-.3.8m.7 2.5q.2 0 .2.2zm.1 2.6v.2h.2v-.2zm-.1.9v.3q0 .7.2.7l.2-.1.2-.1q-.3-.9-.6-.8m.6 1v.2q0 .5.3.4 0-.4-.3-.6M4 21.5v.3l-.2.1q0 .2.6.2 0-.3-.4-.6m.2 1.1q0 .2.2.2h.1l.2-.2z"/><path fill="#c78338" fill-opacity=".5" d="M27 18.4q-1 .5-3 1.1l-2 .5q-.6.4-3 .7 0 .3-1 .3h-1l-.4-.2H16v.1l.7.3-1.4.2q-.2 0-.2-.2V21l-.2-.1h-.5L7 20.5l-5.2-1v.2l.5.3q0 .1-.4 0h-.1v.2q.4 0 .5.3v.2q0 .2 1.1.3l.1-.1-.3-.2H3h.4q.8-.1 1.1.1l-.1.2.2.2 1.3.1-.1.1h-.1q-1.7 0-3.9-.5h-.1v.3l.5.1q0 .3 6.2.6 1.9.3 3 .3l1.1.2h.5q1.3 0 1.9-.2l1.5-.1 3.5-.3q4-.6 5.4-1.1 2-.4 2-.7 0-.4.3-.5v-.2q-.5-.3-.5-.5v-.4zM1.7 19v.4H2v-.2zm25 0 .3.2-.1.2h-.2zm-.7.2q0 .5-.3.5l-.2-.1H25zM3.5 20.3l.6.1v.1h-.2q-.4 0-.6-.2zm19.7 0h.2q0 .2-.7.4h-.1l-.1-.1q.2-.3.7-.3m-1.2.3.3.2h.2q0 .2-1 .3h-.7q0-.3 1.2-.5m-2.3.1h.2v.2h-.3zm-4 .1-.3.1.2.1zm-8.8.2q1.4 0 1.6.2-1 0-1.6-.2m2.3.1v.1H9zm1.4.3q1 .1 1 .4h-.1l-1.4-.2z"/><path fill="#0198fe" fill-opacity=".5" d="m1.7 18.6 1 .8q.5.3.7-.1l1.4-2.6 2.2-4 2.2-3.4.6-.9 4.6-5.7-1.8-1.1L11.1 4 7.8 8.6a82 82 0 0 0-5 8zm.3-.3.2.1v.2zm1-.5.2-.1.2.6v.3h-.1q-.2 0-.2-.4-.1-.2 0-.4m.7-1.9.2-.1v.1l-.1.1zm.8-1.4v.1zm1.6-1.3h.2zm.3-.3.2-.5.2-.3-.1.4zM6 11.7v.4zm1 .2v-.2h.1zm1.1-3-.5 1zm1.1-.7 1-1.4 1.4-1.7.6-.8h.2l-.1.3zm1.6-3.5.3-.2v.2zq-.2.3-.3.2z"/><path fill="#c78338" fill-opacity=".5" d="M19.3 26.4q.1-.3-.3-.5-.5-.6-.6-1l-.3.3q-.1-.1 0-.2l.1-.2-1.1.8-3.6.2q-3-.5-4.5-1l-.1-.1h-.3v.2q-.1.2-.3.2l-.7-.5.5-.1v-.3h-.2q-.2.1-.4 0H7V24q-.7-.2-.5-.6-1.3-1-1.5-1.5-.1-.3-1-1.2l-1.1-2h-.2l-.2.4-.5.5v.1l-.2.7q0 .3.9 1.3.6 1 2.6 2.7l2 1.2.8.5q.3.3 1 .6h.3q.2.1.8 0 .6.3 2 .3 3.7.8 4 .5l1.5-.4q1.1-.3 1.7-.8m-1 .2h.1zm-.4 0h.2zm-4 .4q.1-.1.8 0 0-.1.2 0v-.4q.3-.2.8 0h.2v.1l-.2.1h-.1v.3q.7 0 .8-.2v-.1l.1-.2q.2-.2 1-.2h.1l-.5.4h.5v.1q-.7.4-1.1.3-1.5.2-2.5-.2m3.9-1.2-.1.3H17q-.2.1-.2 0l.6-.2q.7-.3.9-.2v.1zm-.6-.3H17v-.3q.3-.2.5-.1t.1.3zm-1.2.8h.1zm-.6 0q0-.2.4-.1h.1q-.3.3-.5.2m-3 0q.2-.2 1 0-.4.2-1 0m-.4 0H12V26h.1zm-1.6 0q.1-.3.7-.2.3.2.2.3zm-2-1.8-.2-.2v.2zm-2.5-1-.1-.2zm-.8-.2-.4-.1q-.5-.4-.5-.6h.2l.2-.2q.7.6.5.9m-.7-1h-.2q-.4-.3-.3-.5.4.1.5.5m-1.1-1.7-.3-.2-.2.1q-.2 0 0-.6.3.2.5.6zm-1-.7q-.1 0 0-.2v-.1l.2-.1v.3z"/><path fill="#55d400" fill-opacity=".5" d="m27.1 10.4-.4.2q-.6.5-1 .5v.1l.2.2v.1h-.2l-.1-.2.3.6h.2v.5l.1.1-.3 3.6q-.7 2.4-1.3 3.7h2v-.2q.3-.6.5-1.9 1.2-3.7 1-3.9l-.2-1.6q-.2-1.1-.6-1.7zm-1 2-.2-.5h-.1v.5l.2.1zm.2-.8h.2v.3l.2.1q.2.1 0 .9v.2q-.2-.1-.2-.7-.2-.7-.1-.8m.8 0 .1.2h-.1zm-.1.6.3.6v-.5h.1q.4.8.2 1.1 0 1.5-.5 2.5-.1 0 0-.8V15h-.3q-.2-.4 0-.8l.2-.2.1.3q.2.1.2 0v-.7l-.1-.1H27q-.2-.3 0-1.1zm-.5 1.3v.2zm0 .2q.2.3 0 .6-.2 0 0-.5zm-.1 2.5-.2 1q-.2-.2.2-1m-.3 2.2-.1.9q-.3-.2-.2-.7z"/><path fill="#0198fe" fill-opacity=".5" d="m15.4 1.2-1.5.8q-.5.4-.3.7L15.5 5l2.8 3.6 2.2 3.4.5 1 3.1 6.5 2.3-1.3q-1.4-1.8-1.6-2.4l-2.8-5q-.6-1.4-5.2-7.7zm.1.4v.1l-.3.2h-.1zm0 1.3V3h-.8l-.3-.1v-.1q.2-.2.6 0 .3-.1.4 0M17 4.4v.2h-.1v-.2zm1 1.3h-.1zm.3 2v.2-.1zm.2.4.4.5.2.2q-.1.1-.4-.2zm1.5 0h-.2V8q0-.1-.2 0v-.1zM19 9l.1.2H19zm2.4 2.2q-.1.2-.7-.8zm0 1.4h.2l.7 1.4 1 2 .4 1h-.2q0 .1-.1 0zm2.7 2.9v.3H24l-.1-.2q-.1-.3 0-.4t.2.3"/><path fill="#0198fe" fill-opacity=".5" d="M2 12.3q1-.5 3-1 1.7-.3 2.1-.5.6-.2 3.1-.5 0-.3 1-.2h1l.4.2h.5l-.6-.4H14l.2.1v.2l.1.1h.6l7.2.7 5.1 1.2h.1V12l-.5-.3q0-.1.4 0h.1v-.2q-.4 0-.4-.3V11l-1.2-.3-.1.1.1.1.3.1v.1h-.3q-.8 0-1.2-.3l.2-.2-.3-.1-1.2-.2h.2q1.7-.2 3.8.6h.2v-.3l-.6-.2q0-.2-6-.8-2-.4-3-.4l-1.2-.3h-.4L14 9h-1.5L9 9.2l-5.5.8q-1.9.4-2 .6 0 .4-.3.6v.1q.5.4.5.6v.3zm25.2.4v-.4H27v.2zm-25-1.2-.3-.1v-.1l.1-.2h.1l.2.1q0 .3-.2.3m.8-.1q0-.5.3-.5l.3.1h.4zm22.7.1q-.6-.1-.7-.3h.1q.5 0 .7.2zm-19.7-1h-.2q-.1-.2.6-.3h.3q-.3.3-.7.3m1.1-.2-.2-.2-.2-.1q0-.2 1-.2h.6q0 .4-1.2.5m2.4 0h-.2v-.1l.2-.1h.2zm4 0h.3v-.1zm8.8.3q-1.4-.1-1.5-.3 1 0 1.5.2zm-2.2-.3h-.2zm-1.4-.3q-1-.1-1-.4h.1q.7 0 1.3.3z"/><path fill="#c78338" fill-opacity=".5" d="m13.8 21.6.5.7a25 25 0 0 0 3.9 4.8l.4.4.2-.2-.1-.1c-1-1-2-2-2.5-3V24l.8 1 .2.2.1-.3q.4.1.8.8l.2.3-.3-.1h-.1q-.1 0 0 .1l.8.8.1-.1h.1l.5.2.2-.1v-.1l-.3-.3.5.3.3-.2-3.8-3.8-.8-1zm1.3.3q.2 0 1 1.2-.6-.4-1-1.2m3 3.2.4.4.1.1q-.3 0-.5-.4zm.6.7h.1v.1zm1.5.4H20v.2l.3-.1zM19 27"/><path fill="#55d400" fill-opacity=".5" d="M7.5 4.7h-.2l-.3.1h-.8q0 .4-.4.5-.1.2.6 2 .3 1.5 2 5.2l1.6 3 .8 1.4q.2.6.9 1.7l.2.3q.2.4.8.9l.5.8h.8l-.8-1v-.1q.2 0 .7.6l.2.5h1l-2.7-4h-.2v-.2l-.1-.2h-.3v.1l-.4-.1-.6-1.2.6.3V15l-.2-.2-.4-.2-.2-.5-.3-.2v-.1q-.6-.9-.3-1-1-2.3-1-3l-.8-2zm-.8.4.3.2-.2.2h-.2v-.3zm.5.9q.3.5.3 1l-.3-.4-.2-.1q-.2-.2.2-.5m.6 2.8L8 9q.2.3.1.7l-.2-.2q-.3-.7 0-.7m0 1 .2.2h.3q.5 1.2.2 1.3l-.4-.4q-.5-1-.3-1zM9.4 12v.2zm2.2 3.5v.3h.1z"/><path fill="#c78338" fill-opacity=".5" d="m13.3 21.6-.1.2-3.7 3.7.2.2.5-.3-.2.3v.1l.1.2.5-.3h.1l.1.1q.2 0 1-.7l-.1-.2h-.2l-.2.2h-.1l.2-.3q.4-.7.8-.9v.3h.1l.2-.2.9-.9v.1l-.1.1q-.8 1.4-2.5 3l.2.2.4-.4q.2.2 3.6-4.5h-1l-.5.6.3-.6zm-1.8 2.5v.1q-.2.4-.5.5v-.1q.3-.5.5-.5m-.7.8-.1.1h-.1zm-1.4.3-.3.1.3.2.1-.1zm14.9-4.8v.2h-.1v.3h.2q.2.1.2.3-.5.7-.6.6v-.4h-.3l-.1.1q.1.3 0 .4l-.1.3v.2h-.1q-.3.6-.6.4-1.3 1-1.8 1.2l-1.3.9-2.2.9.1.1.4.3.4.5.1.1.7.2 1.3-.7q1-.4 3.1-2.2l1.4-1.7.6-.8.7-1 .1-.2zm-.3.6-.1.1h.2zm-1.4 2.2v.2h-.2v-.2zm-.4.8h.1v.4q-.6.5-.7.4v-.1l-.2-.3q.5-.4.8-.4m-.9.7v.1q-.4.4-.6.2.2-.3.6-.3m-2 .8h.1l-.2.3.1.2-.6-.1q.2-.3.7-.4m-1.1.6.3.1v.2h-.3z"/><path fill="#fff" fill-opacity=".5" d="M7.4 3.8q0 .6-.4 1a1 1 0 0 1-1 .5q-.6 0-1-.5l-.4-1q0-.6.4-1.1t1-.4q.7.1 1 .4.4.5.4 1"/><path fill="#55d400" fill-opacity=".5" d="m6.4 2 .8.5q.4.3.4.5l-.1-.1.1.3.1.2.1.1v.9l-.8 1q-.3.4-.6.4h-.8L5 5.6q-.8-.6-.8-1l-.1-.4.2-.6q0-.4.2-.5.3-.8.9-1l.8-.2zm-.1 0q0 .2.2.2V2zM5 2.6l.5-.3.6-.1H6a1 1 0 0 0-1 .4m1.8-.2H7zm-.3 0q0 .2.2.2zm-.9 0v.1h.6v-.1zm.8 0v.1q-.1 0 0 0zm-.8.3h.7l-.1-.1zm1.4 0 .2.2zm-2 0h.2zm1.6 0 .1.2zm-2 1.6q0 .3.2.5l.3.2.6.3H6l.5-.2.3-.2q.5-.4.4-.8v-.8L7 3.1l-.4-.4h-.1l-1 .1-.3.4-.2.4zM7 2.8l.3.2zM4.8 3H5m2.6 1.1v.3zm-3.5 0v.2zm3.4.5v-.2q.1 0 0 .2m-.1.1.1-.1zm-3.1-.1.1.3zm.4.7.4.1zm.6.1h.2zm.7.2h.2z"/><path fill="#fff" fill-opacity=".5" d="M15.9 1.8q0 .6-.5 1.1a1 1 0 0 1-1 .5q-.5 0-1-.5L13 2q0-.7.4-1.1.6-.5 1-.5.7 0 1 .5.5.6.5 1"/><path fill="#0198fe" fill-opacity=".5" d="m14.9 0 .9.6q.4.3.4.5L16 1v.3h.1l.1.1v1.1l-.8 1q-.3.3-.6.3H14l-.7-.1q-.7-.7-.7-1l-.2-.4.2-.6q0-.4.2-.5.4-.8 1-1l.8-.2zm-.1.2h.2zm-1.4.4.5-.3h.1l.6-.1h-.1a1 1 0 0 0-1 .4zm1.9-.3q0 .2.2.2zm-.4.1.2.1zm-.8 0 .6-.1zm.7 0v.1zm-.7.3h.7V.5h-.2zm1.3 0 .3.2zm-1.9 0h.2zm1.7 0v.1m-2 1.6.3.5.3.2q.1.2.5.3h.2l.5-.2.3-.2q.4-.5.4-.8v-.8l-.3-.3-.5-.3-1 .1-.3.4-.3.4zM15.5.9l.2.2q0-.2-.2-.2m.6 1.3v.3zm-3.1 0V2zm-.5 0v.2zm.1.6q0 .2.2.3zm.4.7.4.1h.1zm.6 0h.3zm1.2.1h.2zm-.4 0h.3z"/><path fill="#fff" fill-opacity=".5" d="M24.8 4.9q0 .7-.4 1a1 1 0 0 1-1 .5q-.6 0-1-.5L22 5q0-.7.4-1 .4-.5 1-.5t1 .4q.6.6.4 1"/><path fill="#55d400" fill-opacity=".5" d="m23.8 3 1 .6q.3.3.3.5H25l.1.2.1.2v.2l.1.3-.1.5-.7 1q-.4.4-.7.4H23l-.7-.2q-.7-.6-.7-1l-.2-.3.3-.7.1-.5q.4-.8 1-1l.8-.2zm-.1.2.1.1h.1zm-1.3.4.5-.3h.6l-.1-.1a1 1 0 0 0-1 .4m1.8-.2h.2zm-.4 0 .2.2q0-.2-.2-.2m-.8 0v.1h.6zm.7 0h.1zm-.7.2h.7zm1.3 0 .3.2zm-2.2 1.7.3.5.3.2q.1.2.5.3h.2l.5-.1.3-.3q.4-.4.4-.7v-.8l-.3-.3q-.2-.3-.5-.4-1 0-1 .2l-.3.4-.2.3zM24.5 4l.2.2q0-.2-.2-.2m-2.2.2.1-.1zm2.8 1.3v.2zM25 6v-.2zm-3.3 0q0 .3.2.3zm.5.7q0 .2.3.2h.1zm.6.1h.2zm.7.3h.3z"/><path fill="#fff" fill-opacity=".5" d="M28.6 11.6q0 .7-.4 1a1 1 0 0 1-1 .5q-.6 0-1-.4l-.4-1.1q0-.7.4-1 .4-.6 1-.5t1 .4q.5.6.4 1"/><path fill="#55d400" fill-opacity=".5" d="m27.6 9.8 1 .5.3.5h-.1l.1.2.1.2v.2l.1.3-.1.5q0 .2-.7 1.1-.3.3-.7.3H27l-.8-.2q-.7-.6-.7-1l-.2-.3.3-.7.1-.5q.4-.8 1-1zm-.1.1.2.1zm-1.3.4.5-.3h.6l-.1-.1a1 1 0 0 0-1 .4m1.8-.2.2.1zm-.4 0 .3.2zm-.8 0h.6zm.8 0h-.1zm-.8.3h.2l.5-.1zm1.4 0 .2.2zm-2 0h.2zm1.6.1h.1zm-2 1.6.3.5.3.3.5.2h.2l.5-.1.3-.3q.4-.4.4-.7v-.8l-.3-.3q-.2-.3-.5-.4-1 0-1 .2l-.3.4-.2.3zm2.3-1.5.2.2q0-.2-.2-.2m-2 0v.1zm-.2.3.1-.1zm2.9.7v.1zm-.1.6v.2zm-.1.5v-.2zm-3.3 0q0 .3.2.3zm.5.7q0 .2.4.2zm.6.1.2.1zm-.2 0h.1m1.3.2.1-.1zm-.5 0h.3z"/><path fill="#fff" fill-opacity=".5" d="M27 20q-.1 1.4-1.4 1.5-1.2 0-1.4-1.5c-.2-1.5.7-1.5 1.5-1.5q1.2.1 1.4 1.5"/><path fill="#0198fe" fill-opacity=".5" d="M26 18.3h.2q.4.1.8.5.4.3.4.5l-.1-.1v.3h.1l.1.1v1.1l-.8 1q-.3.3-.6.3h-.8l-.7-.1q-.7-.7-.7-1l-.2-.4.2-.6q0-.4.2-.5.4-.8 1-1l.8-.2zm0 0 .1.2h.1zm-1.3.5.4-.3h.1l.6-.1h-.1a1 1 0 0 0-1 .3zm1.8-.3.2.1zm-.4.1.2.1zm-.8 0h.6zm.7 0h.1zm-.7.3h.7v-.1h-.2zm1.3 0 .3.2zm-1.9 0h.2v-.1zm1.7 0v.1h.1zm-2 1.6.3.5.3.2q.1.2.5.2h.7l.3-.3q.4-.5.4-.8v-.8l-.3-.3q-.3-.3-.5-.3l-1 .1-.3.4-.3.4zm2.3-1.5.2.2q0-.2-.2-.2m-2.2.2h.1zm2.8 1.1v.3zm-3.6 0v.1zm3.3.6.2-.1zm-3 0v.3zm.2.4v.1h.1zm.2.3.3.1h.1zm.5 0h.3zm-.1 0h.1zm1.4.2h.1zm-.5 0h.3v-.1z"/><path fill="#fff" fill-opacity=".5" d="M19.6 26q0 .6-.5 1a1 1 0 0 1-1 .5q-.5 0-1-.4l-.4-1.1q0-.7.4-1 .6-.5 1-.5.6 0 1 .4t.5 1.1"/><path fill="#c78338" fill-opacity=".5" d="m18.6 24.2.9.5.4.5h-.1v.2h.1l.1.2v1q0 .2-.8 1.1-.3.3-.6.3h-.8l-.7-.2q-.7-.6-.7-1l-.2-.3.2-.7q0-.3.2-.5.4-.8 1-1l.8-.2zm-.1.1.1.1h.1zm-1.4.4.5-.3h.7l-.1-.1a1 1 0 0 0-1 .4zm1.9-.2.2.1zm-.4 0 .2.2zm-.8 0h.6zm0 .3h.1l.6-.1h-.7m1.3 0 .3.2zm-1.9 0h.2zm-.3 1.7.3.4.3.3.5.2h.2l.5-.1.3-.3q.4-.4.4-.7v-.8l-.3-.3-.5-.4q-1 0-1 .2l-.3.4-.3.3zm2.3-1.5q0 .2.2.2 0-.2-.2-.2m-2 0v.1zm-.2.3v-.1m2.8.6v.1zm0 .6v.2zv-.1m-.1.5v-.1zm-.2.1h.1v-.1zm-3.2-.1.1.3zm.4.7.4.2h.1zm.6.1.3.1zm1.2.2.2-.1zm-.4 0h.3-.2"/><path fill="#fff" fill-opacity=".5" d="M11.2 25.6q0 .6-.4 1a1 1 0 0 1-1 .5q-.7 0-1-.5l-.4-1q0-.6.4-1.1.3-.5 1-.5.5 0 1 .5l.4 1"/><path fill="#c78338" fill-opacity=".5" d="m10.2 23.8 1 .5q.3.3.3.5l-.1-.1v.3h.1l.1.2v1l-.8 1q-.3.4-.6.4h-.8l-.7-.2q-.7-.7-.7-1l-.2-.4.3-.6.1-.5q.4-.8 1-1l.8-.2zm-.1 0 .1.2h.1zm-1.3.5.5-.3.6-.1h-.1a1 1 0 0 0-1 .4m1.8-.2h.2zm-.4 0 .2.1zm-.8 0h.6V24zm0 .3h.7l-.1-.1zm1.3 0 .3.2zm-.2 0 .1.2zm-2 1.6.3.5.3.2q.1.2.5.3h.2q.3 0 .5-.2l.3-.2q.4-.4.4-.8V25l-.3-.2q-.2-.3-.5-.4l-1 .1-.3.4-.2.4zm2.4-1.5.2.2q0-.2-.2-.2m.5 1.3.1.3zm-3.6 0v.2zm3.5.5v-.2zm-.1 0v.1zm-3.2 0q0 .3.2.3zm.5.7.3.1H9zm.6.1h.2zm.7.2h.2z"/><path fill="#fff" fill-opacity=".5" d="M4.3 19.7q0 .6-.4 1a1 1 0 0 1-1 .5q-.6 0-1-.4l-.4-1q0-.7.4-1.2.4-.3 1-.4.6 0 1 .4t.4 1.1"/><path fill="#0198fe" fill-opacity=".5" d="m3.4 18 .9.4.4.5h-.2l.1.3V19h.1v.2l.1.2v.8L4 21.4q-.3.3-.6.3h-.8l-.7-.2q-.7-.6-.7-1l-.2-.3.2-.7q0-.3.2-.5.3-.7 1-1H3zm-.2 0 .2.1V18zm-1.3.5.5-.3h.1L3 18a1 1 0 0 0-1 .4zm1.9-.3.1.1zm-.4 0 .2.2zm-.9 0h.7zm.1.3.6-.1h-.6m1.3 0 .2.2zm-.3.1.1.1m-2 1.6.2.5.3.2.6.2h.1l.5-.1.3-.2q.5-.5.5-.8v-.8l-.4-.3-.4-.4h-.1q-1 0-1 .2l-.2.4-.3.3zM4 18.8l.3.1zm-2.2.2H2zm2.8 1.2v.2zM1 20v.1zm3.4.4v-.1zm-3.2 0 .1.3zm.4.8H2zm.6 0 .2.1zm-.2 0h.1m1.3.2h.2v-.1zm-.4 0h.2z"/><path fill="#fff" fill-opacity=".5" d="M3.3 11q0 .5-.4 1a1 1 0 0 1-1 .5q-.5 0-1-.5l-.4-1q0-.7.4-1.1.6-.6 1-.5.6 0 1 .5t.4 1"/><path fill="#55d400" fill-opacity=".5" d="M2.4 9.2q.5.1.9.5.4.3.4.5l-.2-.1.2.3v.1l.1.2v.9l-.8 1q-.3.3-.6.3h-.8l-.7-.1q-.7-.7-.7-1l-.2-.4.2-.6q0-.4.2-.5.4-.8 1-1L2 9zm-.2 0q0 .2.2.2h.1q0-.2-.2-.2m-1.4.5.5-.3h.1l.6-.1h-.2a1 1 0 0 0-1 .3zm1.9-.3.1.2zm-.4.1.2.1zm-.8 0h.6zm0 .3h.7v-.1H2zm1.3 0 .2.2zM1 9.8h.2v-.1zm1.6 0 .2.1zM.7 11.5l.2.5.4.2q.1.2.5.2h.7l.2-.3q.5-.5.5-.8v-.8l-.3-.3q-.3-.3-.5-.3h-.1l-1 .1-.2.4-.3.4zM3 10l.2.2q0-.2-.2-.2m.6 1.3v.3zm-.3.6.1-.1m-3.2 0 .1.3zm.3.4v.1zm.1.3.4.1zm.6 0v.1h.3zm1.2.1h.2zm-.4 0h.3z"/><path fill="#fff" fill-opacity=".5" d="M9.6 9.6q0 .7-.5 1a1 1 0 0 1-1 .5q-.5 0-1-.4l-.4-1.1q0-.7.5-1 .3-.5 1-.5.5 0 1 .4t.4 1"/><path fill="#0198fe" fill-opacity=".5" d="m8.6 7.8.9.5q.4.3.4.5h-.1V9h.1l.1.2v1q0 .2-.8 1.1-.3.3-.6.3h-.8l-.7-.2q-.7-.6-.7-1l-.2-.3.2-.7q0-.3.2-.5.4-.8 1-1l.8-.2zm-1.4.5.4-.3h.7l-.1-.1a1 1 0 0 0-1 .4M9 8.1l.2.1zm-.4 0 .2.2q0-.2-.2-.2m-.8 0h.6zm0 .3H8l.6-.1zm1.3 0 .3.2zm-1.9 0h.2zM7 10.2l.3.5.3.3.5.2h.2l.5-.1.3-.3q.4-.4.4-.7v-.8L9 8.9l-.5-.4q-1 0-1 .2l-.3.4-.3.3zm2.3-1.5.2.2q0-.2-.2-.2m.5 1.6v.2zm-.1.5v-.2zm-.2.1.2-.1zm-3-.1v.3zm.2.5H7zm.2.2q0 .2.3.2h.1zm.5.1.3.1zm-.1 0h.1zm1.4.2.1-.1zm-.5 0h.3z"/><path fill="#fff" fill-opacity=".5" d="M21.9 10.2q0 .7-.4 1a1 1 0 0 1-1 .5q-.6 0-1-.4l-.5-1.1q0-.7.5-1 .4-.5 1-.5t1 .4q.6.5.4 1.1"/><path fill="#0c9cfe" fill-opacity=".5" d="m20.9 8.4 1 .5.3.5h-.1v.2h.1l.1.2v1q0 .2-.8 1.1-.3.3-.6.3h-.8l-.7-.2q-.7-.6-.7-1l-.2-.3.2-.7q0-.3.2-.5.4-.7 1-1l.8-.1zm-1.4.5.5-.2.6-.2h-.1a1 1 0 0 0-1 .4m1.8-.2.2.1zm-.4 0 .2.2zm-.8 0h.6zm0 .3.6-.1zm1.3 0 .3.2zm-1.9 0h.2zm1.7.1h.1zm-2 1.7.3.4.3.3.5.2h.2l.5-.1.3-.3q.4-.4.4-.7v-.8l-.3-.3q-.2-.3-.5-.4-1 0-1 .2l-.3.4-.2.3zm2.4-1.5.2.1zm-2.2.2.1-.1zm2.7 1.2v.2h.1zm-3.4.5q0 .3.2.3 0-.3-.2-.3m.5.7q0 .2.3.2h.1zm.6.1.2.1zm.7.2h.3z"/><path fill="#fff" fill-opacity=".5" d="M15.4 21q0 .8-.4 1.1a1 1 0 0 1-1 .5q-.6 0-1-.5l-.4-1q0-.6.4-1.1t1-.4 1 .4q.5.6.4 1"/><path fill="#55d400" fill-opacity=".5" d="m14.4 19.3.8.5q.5.3.5.5l-.2-.1.1.3.1.2.1.1v.9l-.8 1q-.3.4-.6.4h-.8L13 23q-.8-.6-.8-1l-.1-.3.2-.7q0-.4.2-.5.3-.8.9-1l.8-.2zm-.1 0q0 .2.2.2v-.1zm-1.3.4.5-.3h.6v-.1a1 1 0 0 0-1 .4zm1.8-.2h.2zm-.3 0q0 .2.2.2zm-.9 0v.1h.6v-.1zm.8 0v.1q-.1 0 0 0zm-.8.3h.7zm1.4 0 .2.2zm-2 0h.2zm-.4 1.7q0 .3.2.5l.3.2.6.3h.1l.5-.1.3-.3q.5-.4.4-.7v-.9l-.3-.2-.4-.4h-.1q-1 0-1 .2l-.3.3-.2.4zM15 20l.3.2zm-2.8.6.1-.1m3.2.8v.3zm-.1.5v-.2zm-3.2 0 .1.3zm.4.7.4.1zm.6.1h.2zm.7.2h.2z"/></svg>)=====";
