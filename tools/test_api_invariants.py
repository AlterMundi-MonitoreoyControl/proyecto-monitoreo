import os
import pytest
import requests

# ── Configuración ─────────────────────────────────────────────────────────────
# Puedes ejecutar las pruebas en otra IP usando: ESP32_IP="192.168.1.x" pytest test_api_invariants.py
ESP32_IP = os.getenv("ESP32_IP", "192.168.1.110")
BASE_URL = f"http://{ESP32_IP}"
TIMEOUT = 5

@pytest.fixture(scope="session")
def base_url():
    return BASE_URL

# ── Pruebas de Invariantes ───────────────────────────────────────────────────

def test_api_status_invariants(base_url):
    """Verifica que el estado general tenga las métricas vitales y tipos correctos."""
    r = requests.get(f"{base_url}/api/status", timeout=TIMEOUT)
    assert r.status_code == 200, "El endpoint debe devolver 200 OK"
    
    data = r.json()
    
    # Campos que SIEMPRE deben existir, independientemente del estado del hardware
    expected_keys = {
        "device_name": str,
        "device_id": str,
        "firmware_version": str,
        "wifi_connected": bool,
        "active_sensors": int,
        "total_sensors": int,
        "uptime": str
    }
    
    for key, expected_type in expected_keys.items():
        assert key in data, f"Falta la clave invariante: '{key}'"
        assert isinstance(data[key], expected_type), f"El campo '{key}' debe ser {expected_type.__name__}"
        
    # Si el WiFi está conectado, deben existir las métricas de red
    if data["wifi_connected"]:
        assert "wifi_ssid" in data
        assert "wifi_ip" in data
        assert "wifi_rssi" in data
        assert isinstance(data["wifi_rssi"], int)
        assert "signal_class" in data

def test_actual_data_invariants(base_url):
    """Verifica la estructura de datos en vivo (Live Data)."""
    r = requests.get(f"{base_url}/actual", timeout=TIMEOUT)
    assert r.status_code == 200
    data = r.json()
    
    assert "sensors" in data, "La respuesta debe contener la lista de sensores"
    assert isinstance(data["sensors"], list)
    assert "actuators" in data
    assert isinstance(data["actuators"], list)
    assert "uptime" in data
    
    # Validar el esquema de cada sensor activo
    for sensor in data["sensors"]:
        assert "type" in sensor
        assert "id" in sensor
        assert "active" in sensor
        assert isinstance(sensor["active"], bool)
        assert "error" in sensor
        assert "readings" in sensor
        assert isinstance(sensor["readings"], list)
        
        # Validar el esquema de las lecturas dentro del sensor (si tiene)
        for reading in sensor["readings"]:
            assert "label" in reading
            assert "value" in reading
            assert "unit" in reading
            assert "status" in reading
            assert "key_var" in reading  # Crucial para la decodificación en reglas
            
        # Validaciones de los bloques opcionales (ej: sensores capacitivos/resistivos)
        if "diagnostics" in sensor:
            assert "raw" in sensor["diagnostics"]
            assert "pin" in sensor["diagnostics"]
            
        if "calibration" in sensor:
            assert "dry" in sensor["calibration"]
            assert "wet" in sensor["calibration"]
            
    for actuator in data["actuators"]:
        assert "id" in actuator
        assert isinstance(actuator["id"], int)
        assert "name" in actuator
        assert "state" in actuator
        assert isinstance(actuator["state"], bool)
        assert "key_actuator" in actuator

def test_config_invariants(base_url):
    """Verifica que la configuración cargada exponga todos los módulos configurables."""
    r = requests.get(f"{base_url}/config", timeout=TIMEOUT)
    assert r.status_code == 200
    data = r.json()
    
    # Nuevas variables de configuración estandarizadas del sistema
    expected_keys = [
        "incubator_name", "hash", "ssid",
        "min_temperature", "max_temperature", "min_hum", "max_hum",
        "sensors", "relays", "rs485", "current_wifi_channel"
    ]
    
    for key in expected_keys:
        assert key in data, f"Falta la clave de configuración: '{key}'"
        
    assert isinstance(data["sensors"], list)
    assert isinstance(data["relays"], list)
    assert isinstance(data["rs485"], dict)

def test_rules_schema_invariants(base_url):
    """Verifica el formato del motor de reglas automáticas."""
    r = requests.get(f"{base_url}/rules", timeout=TIMEOUT)
    assert r.status_code == 200
    data = r.json()
    
    assert "rules" in data
    assert isinstance(data["rules"], list)
    
    for rule in data["rules"]:
        assert "expr" in rule
        assert isinstance(rule["expr"], dict), "La expresión debe ser un objeto"
        assert "actuator" in rule
        assert "state" in rule
        assert isinstance(rule["state"], bool)
        assert "priority" in rule
        assert "duration_ms" in rule

def test_actuator_status_invariants(base_url):
    """Verifica el estado del pool de actuadores en el mediator."""
    r = requests.get(f"{base_url}/actuator/status", timeout=TIMEOUT)
    assert r.status_code == 200
    data = r.json()
    
    assert isinstance(data, list)
    for actuator in data:
        assert "id" in actuator
        assert isinstance(actuator["id"], int)
        assert "name" in actuator
        assert "state" in actuator
        assert isinstance(actuator["state"], bool)

def test_api_relays_invariants(base_url):
    """Verifica la estructura del listado de relés heredado (/api/relays)."""
    r = requests.get(f"{base_url}/api/relays", timeout=TIMEOUT)
    assert r.status_code == 200
    data = r.json()
    
    assert isinstance(data, list)
    for relay in data:
        assert "address" in relay
        assert "alias" in relay
        assert "state" in relay
        assert isinstance(relay["state"], list)
        if relay.get("type") != "gpio":
            assert "input_state" in relay
            assert isinstance(relay["input_state"], list)

def test_espnow_status_invariants(base_url):
    """Verifica la conectividad Mesh/ESP-NOW."""
    r = requests.get(f"{base_url}/espnow/status", timeout=TIMEOUT)
    if r.status_code == 404:
        pytest.skip("ESP-NOW no está habilitado en este build de firmware")
        
    assert r.status_code == 200
    data = r.json()
    
    assert "enabled" in data
    assert isinstance(data["enabled"], bool)
    assert "mode" in data
    assert "mac_address" in data
    assert "paired" in data
    assert "peer_count" in data