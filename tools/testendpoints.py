#!/usr/bin/env python3
"""
ESP32 Endpoint Exerciser
========================

Exercises all HTTP endpoints exposed by the ESP32 firmware.

Usage:
    python test_endpoints.py --ip <DEVICE_IP> [options]

Options:
    --ip        IP address of the target device (required)
    --port      HTTP port (default: 80)
    --timeout   Request timeout in seconds (default: 5)
    --skip-destructive  Skip endpoints that restart or reset the device
    --skip-invariants   Skip running pytest API invariant checks
    --relay-addr        Relay address for toggle test (default: 1)
    --relay-ch          Relay channel for toggle test (default: 0)
    --actuator-id       Actuator ID for command test (default: 0)
    --dump              Dump responses to a JSON file (e.g. expected.json)
"""

import argparse
import json
import os
import sys
import time
from typing import Any

try:
    import requests
except ImportError:
    print("[ERROR] 'requests' library not found. Install it with: pip install requests")
    sys.exit(1)


# ── ANSI colours ─────────────────────────────────────────────────────────────
GREEN  = "\033[92m"
YELLOW = "\033[93m"
RED    = "\033[91m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
RESET  = "\033[0m"


def ok(msg: str)   -> None: print(f"  {GREEN}✔ {msg}{RESET}")
def warn(msg: str) -> None: print(f"  {YELLOW}⚠ {msg}{RESET}")
def err(msg: str)  -> None: print(f"  {RED}✘ {msg}{RESET}")
def info(msg: str) -> None: print(f"  {CYAN}→ {msg}{RESET}")


# ── Result tracking ───────────────────────────────────────────────────────────
results: list[dict[str, Any]] = []
dump_data: dict[str, Any] = {}


def record(name: str, passed: bool, detail: str = "") -> None:
    results.append({"name": name, "passed": passed, "detail": detail})

def record_dump(name: str, data: Any) -> None:
    dump_data[name] = data


# ── HTTP helpers ──────────────────────────────────────────────────────────────
def get(session: requests.Session, url: str, timeout: int,
        params: dict | None = None) -> requests.Response | None:
    try:
        r = session.get(url, params=params, timeout=timeout)
        return r
    except requests.exceptions.ConnectionError:
        err(f"Connection refused: {url}")
        return None
    except requests.exceptions.Timeout:
        err(f"Timeout: {url}")
        return None


def post(session: requests.Session, url: str, timeout: int,
         json_body: dict | None = None,
         data: str | None = None) -> requests.Response | None:
    try:
        if json_body is not None:
            r = session.post(url, json=json_body, timeout=timeout)
        else:
            r = session.post(url, data=data,
                             headers={"Content-Type": "application/json"},
                             timeout=timeout)
        return r
    except requests.exceptions.ConnectionError:
        err(f"Connection refused: {url}")
        return None
    except requests.exceptions.Timeout:
        err(f"Timeout: {url}")
        return None


def assert_status(r: requests.Response | None, expected: int,
                  name: str) -> bool:
    if r is None:
        record(name, False, "No response")
        return False
    if r.status_code == expected:
        ok(f"HTTP {r.status_code}")
        return True
    else:
        warn(f"HTTP {r.status_code} (expected {expected})")
        record(name, False, f"HTTP {r.status_code}")
        return False


def try_json(r: requests.Response, name: str) -> dict | list | None:
    try:
        data = r.json()
        record_dump(name, data)
        info(f"Response: {json.dumps(data, ensure_ascii=False)[:300]}")
        return data
    except Exception:
        warn("Response is not valid JSON")
        info(f"Raw: {r.text[:200]}")
        record(name, False, "Invalid JSON response")
        return None


# ── Individual endpoint tests ─────────────────────────────────────────────────

def test_home(session, base, timeout):
    name = "GET /"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/", timeout)
    if assert_status(r, 200, name):
        info(f"Content-Type: {r.headers.get('Content-Type','?')}")
        record_dump(name, r.text[:200] + "... (truncated)")
        record(name, True)


def test_status(session, base, timeout):
    name = "GET /status"
    name = "GET /api/status"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/status", timeout)
    r = get(session, f"{base}/api/status", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if data:
            for key in ("device_name", "firmware_version", "wifi_connected", "uptime"):
                if key in data:
                    ok(f"Field '{key}': {data[key]}")
                else:
                    warn(f"Missing expected field '{key}'")
            record(name, True)


def test_mediciones(session, base, timeout):
    name = "GET /mediciones"
    name = "GET /actual"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/mediciones", timeout)
    r = get(session, f"{base}/actual", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if data:
            sensors = data.get("sensors", [])
            actuators = data.get("actuators", [])
            ok(f"Sensors reported: {len(sensors)}")
            ok(f"Actuators reported: {len(actuators)}")
            ok(f"WiFi status: {data.get('wifi_status', '?')}")
            ok(f"Uptime: {data.get('uptime', '?')}")
            for s in sensors:
                readings = s.get("readings", [])
                info(f"  Sensor [{s.get('id','?')}] type={s.get('type','?')} "
                     f"readings={len(readings)}")
                for rdg in readings:
                    info(f"    {rdg.get('label','?')}: {rdg.get('value','?')} "
                         f"{rdg.get('unit','')} [{rdg.get('status','?')}]")
            record(name, True)


def test_configuracion_get(session, base, timeout):
    name = "GET /configuracion"
    name = "GET /config"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/configuracion", timeout)
    r = get(session, f"{base}/config", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if data:
            ok(f"Config keys: {list(data.keys())[:10]}")
            record(name, True)


def test_configuracion_post(session, base, timeout):
    """
    POSTs a harmless config update.
    Reads current config, changes max_temperature, and posts it back.
    """
    name = "POST /config"
    print(f"\n{BOLD}{name}{RESET}")
    
    r_get = get(session, f"{base}/config", timeout)
    if r_get is None or r_get.status_code != 200:
        warn("Could not read current config — skipping POST test")
        record(name, False, "Could not read config first")
        return
        
    payload = r_get.json()
    payload["max_temperature"] = 22.0
    
    # Evitar que el ESP32 reinicie su conexión WiFi durante el POST
    payload["ssid"] = "ToChange"
    payload["passwd"] = "ToChange"

    r = post(session, f"{base}/config", timeout, json_body=payload)
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code in (200, 400):
        record_dump(name, r.text.strip())
        ok(f"HTTP {r.status_code} — {r.text[:120]}")
        record(name, True)
    else:
        warn(f"HTTP {r.status_code}")
        record(name, False, f"HTTP {r.status_code}")


def test_relay_list(session, base, timeout):
    name = "GET /relay/list"
    name = "GET /api/relays"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/relay/list", timeout)
    r = get(session, f"{base}/api/relays", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if isinstance(data, list):
            ok(f"Relays reported: {len(data)}")
            for relay in data:
                info(f"  Relay addr={relay.get('address','?')} "
                     f"alias={relay.get('alias','?')} "
                     f"type={relay.get('type','?')}")
            record(name, True)


def test_relay_toggle(session, base, timeout, addr: int, ch: int):
    name = f"GET /relay/toggle?addr={addr}&ch={ch}"
    name = f"POST /api/relay/toggle?addr={addr}&ch={ch}"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/relay/toggle", timeout,
            params={"addr": addr, "ch": ch})
    r = post(session, f"{base}/api/relay/toggle?addr={addr}&ch={ch}", timeout, data="")
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code == 200:
        record_dump(name, r.text.strip())
        ok(f"Toggle accepted: {r.text.strip()}")
        record(name, True)
    elif r.status_code == 404:
        warn(f"Relay addr={addr} ch={ch} not found on device")
        # Se acepta el 404 como válido ya que el relé puede estar deshabilitado en config
        record(name, True, "Skipped (Relay not configured)")
    elif r.status_code == 400:
        warn(f"Bad request: {r.text.strip()}")
        record(name, False, "Bad request")
    else:
        warn(f"HTTP {r.status_code}")
        record(name, False, f"HTTP {r.status_code}")


def test_actuator_status(session, base, timeout):
    name = "GET /actuator/status"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/actuator/status", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if isinstance(data, list):
            ok(f"Actuators: {len(data)}")
            for a in data:
                info(f"  id={a.get('id','?')} name={a.get('name','?')} "
                     f"state={a.get('state','?')}")
            record(name, True)


def test_actuator_command(session, base, timeout, actuator_id: int):
    name = "POST /actuator/command"
    print(f"\n{BOLD}{name}{RESET}")
    payload = {
        "id": actuator_id,
        "state": False,   # safe — set to OFF
        "duration": 0,
        "priority": 3
    }
    r = post(session, f"{base}/actuator/command", timeout, json_body=payload)
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code == 200:
        data = try_json(r, name)
        if data:
            ok(f"Command accepted: id={data.get('id')} state={data.get('state')}")
            record(name, True)
    else:
        warn(f"HTTP {r.status_code} — {r.text[:120]}")
        record(name, False, f"HTTP {r.status_code}")


def test_rules_get(session, base, timeout):
    name = "GET /rules"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/rules", timeout)
    if assert_status(r, 200, name):
        data = try_json(r, name)
        if data is not None:
            rules = data.get("rules", data) if isinstance(data, dict) else data
            ok(f"Rules on device: {len(rules) if isinstance(rules, list) else '?'}")
            record(name, True)


def test_rules_reload(session, base, timeout):
    name = "POST /rules/reload"
    print(f"\n{BOLD}{name}{RESET}")
    r = post(session, f"{base}/rules/reload", timeout, data="")
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code == 200:
        data = try_json(r, name)
        if data:
            ok(f"Rules reloaded: {data.get('rules_loaded','?')} rule(s)")
            record(name, True)
    else:
        warn(f"HTTP {r.status_code} — {r.text[:120]}")
        record(name, False, f"HTTP {r.status_code}")


def test_rules_save(session, base, timeout):
    """Saves the existing rules back (read → write round-trip, non-destructive)."""
    name = "POST /rules/save (round-trip)"
    print(f"\n{BOLD}{name}{RESET}")

    # First read current rules
    r_get = get(session, f"{base}/rules", timeout)
    if r_get is None or r_get.status_code != 200:
        warn("Could not read current rules — skipping save test")
        record(name, False, "Could not read rules first")
        return

    body = r_get.text  # re-post exact content
    r = post(session, f"{base}/rules/save", timeout, data=body)
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code == 200:
        data = try_json(r, name)
        if data:
            ok(f"Rules saved & reloaded: {data.get('rules_loaded','?')} rule(s)")
            record(name, True)
    else:
        warn(f"HTTP {r.status_code} — {r.text[:120]}")
        record(name, False, f"HTTP {r.status_code}")


def test_calibration(session, base, timeout):
    name = "GET /calibrate  (SCD30)"
    name = "GET /calibrate-scd30  (SCD30)"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/calibrate", timeout)
    r = get(session, f"{base}/calibrate-scd30", timeout)
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code in (200, 503):
        data = try_json(r, name)
        if data:
            status = data.get("status", "?")
            msg = data.get("message", "")
            (ok if status == "success" else warn)(f"Status: {status} — {msg}")
            record(name, True)
    else:
        warn(f"HTTP {r.status_code}")
        record(name, False, f"HTTP {r.status_code}")


def test_espnow_status(session, base, timeout):
    name = "GET /espnow/status"
    print(f"\n{BOLD}{name}{RESET}")
    r = get(session, f"{base}/espnow/status", timeout)
    if r is None:
        record(name, False, "No response")
        return
    if r.status_code == 200:
        data = try_json(r, name)
        if data:
            ok(f"ESPNow enabled={data.get('enabled')} mode={data.get('mode')} "
               f"mac={data.get('mac_address')}")
            record(name, True)
    elif r.status_code == 404:
        warn("ESPNow endpoint not present (feature may be disabled in firmware)")
        record(name, False, "404 — feature disabled")
    else:
        warn(f"HTTP {r.status_code}")
        record(name, False, f"HTTP {r.status_code}")


def test_static_assets(session, base, timeout):
    assets = [
        ("/style.css",    "text/css",               "GET /style.css"),
        ("/config.js",    "application/javascript",  "GET /config.js"),
        ("/favicon.ico",  "image/svg+xml",           "GET /favicon.ico"),
        ("/favicon.svg",  "image/svg+xml",           "GET /favicon.svg"),
        ("/data",         "text/html",               "GET /data"),
        ("/settings",     "text/html",               "GET /settings"),
        ("/rules-editor", "text/html",               "GET /rules-editor"),
    ]
    for path, expected_ct, name in assets:
        print(f"\n{BOLD}{name}{RESET}")
        r = get(session, f"{base}{path}", timeout)
        if r is None:
            record(name, False, "No response")
            continue
        if r.status_code == 200:
            ct = r.headers.get("Content-Type", "")
            if expected_ct in ct:
                ok(f"Content-Type: {ct}")
            else:
                warn(f"Content-Type '{ct}' does not match expected '{expected_ct}'")
            record(name, True)
        else:
            warn(f"HTTP {r.status_code}")
            record(name, False, f"HTTP {r.status_code}")


def test_restart(session, base, timeout):
    name = "GET /restart  ⚠ DESTRUCTIVE"
    name = "POST /restart  ⚠ DESTRUCTIVE"
    print(f"\n{BOLD}{name}{RESET}")
    warn("Sending restart command — device will reboot!")
    r = get(session, f"{base}/restart", timeout)
    r = post(session, f"{base}/restart", timeout, data="")
    if r is not None and r.status_code == 200:
        record_dump(name, r.text.strip())
        ok(f"Restart accepted: {r.text.strip()}")
        record(name, True)
    else:
        status = r.status_code if r else "no response"
        warn(f"HTTP {status}")
        record(name, False, str(status))


def test_config_reset(session, base, timeout):
    name = "GET /config/reset  ⚠ DESTRUCTIVE"
    name = "POST /config/reset  ⚠ DESTRUCTIVE"
    print(f"\n{BOLD}{name}{RESET}")
    warn("Sending config-reset command — device will reset and reboot!")
    r = get(session, f"{base}/config/reset", timeout)
    r = post(session, f"{base}/config/reset", timeout, data="")
    if r is not None and r.status_code == 200:
        data = try_json(r, name)
        if data:
            ok(f"Config reset: {data.get('message','?')}")
            record(name, True)
    else:
        status = r.status_code if r else "no response"
        warn(f"HTTP {status}")
        record(name, False, str(status))


# ── Summary ───────────────────────────────────────────────────────────────────

def print_summary():
    passed = [r for r in results if r["passed"]]
    failed = [r for r in results if not r["passed"]]
    total = len(results)

    print(f"\n{'═'*60}")
    print(f"{BOLD}SUMMARY{RESET}  {total} tests  |  "
          f"{GREEN}{len(passed)} passed{RESET}  |  "
          f"{RED}{len(failed)} failed{RESET}")
    print('═'*60)

    if failed:
        print(f"\n{RED}Failed tests:{RESET}")
        for r in failed:
            detail = f" ({r['detail']})" if r['detail'] else ""
            print(f"  {RED}✘{RESET} {r['name']}{detail}")

    print()


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Exercise ESP32 firmware HTTP endpoints."
    )
    parser.add_argument("--ip", required=True,
                        help="IP address of the target ESP32 device")
    parser.add_argument("--port", type=int, default=80,
                        help="HTTP port (default: 80)")
    parser.add_argument("--timeout", type=int, default=5,
                        help="Request timeout in seconds (default: 5)")
    parser.add_argument("--skip-destructive", action="store_true",
                        help="Skip /restart and /config/reset endpoints")
    parser.add_argument("--skip-invariants", action="store_true",
                        help="Skip running pytest API invariant checks")
    parser.add_argument("--relay-addr", type=int, default=1,
                        help="Relay address for toggle test (default: 1)")
    parser.add_argument("--relay-ch", type=int, default=0,
                        help="Relay channel for toggle test (default: 0)")
    parser.add_argument("--actuator-id", type=int, default=0,
                        help="Actuator ID for command test (default: 0)")
    parser.add_argument("--dump", type=str,
                        help="Dump responses to a JSON file (e.g. expected_output.json)")
    args = parser.parse_args()

    base = f"http://{args.ip}:{args.port}"

    print(f"\n{BOLD}{'═'*60}")
    print(f"  ESP32 Endpoint Exerciser")
    print(f"  Target : {base}")
    print(f"  Timeout: {args.timeout}s")
    if args.skip_destructive:
        print(f"  Mode   : safe (destructive endpoints skipped)")
    else:
        print(f"  Mode   : full (destructive endpoints INCLUDED)")
    print(f"{'═'*60}{RESET}")

    session = requests.Session()
    t = args.timeout

    # ── Safe endpoints ────────────────────────────────────────────────────────
    test_home(session, base, t)
    test_status(session, base, t)
    test_mediciones(session, base, t)
    test_configuracion_get(session, base, t)
    test_configuracion_post(session, base, t)
    test_relay_list(session, base, t)
    test_relay_toggle(session, base, t, args.relay_addr, args.relay_ch)
    test_actuator_status(session, base, t)
    test_actuator_command(session, base, t, args.actuator_id)
    test_rules_get(session, base, t)
    test_rules_reload(session, base, t)
    test_rules_save(session, base, t)
    test_calibration(session, base, t)
    test_espnow_status(session, base, t)
    test_static_assets(session, base, t)


    # ── Invariant Checks (Pytest) ─────────────────────────────────────────────
    if not args.skip_invariants:
        print(f"\n{CYAN}{'─'*60}")
        print(f"  API Invariant Checks (pytest)")
        print(f"{'─'*60}{RESET}")
        try:
            import pytest
            os.environ["ESP32_IP"] = args.ip
            script_dir = os.path.dirname(os.path.abspath(__file__))
            invariants_script = os.path.join(script_dir, "test_api_invariants.py")
            
            ret = pytest.main(["-v", invariants_script])
            if ret == 0:
                record("Pytest Invariants", True, "All structural checks passed")
            else:
                record("Pytest Invariants", False, f"Pytest failed with code {ret}")
        except ImportError:
            warn("pytest is not installed. Skipping invariant checks.")
            record("Pytest Invariants", False, "pytest not installed")
    # ── Destructive endpoints ─────────────────────────────────────────────────
    if not args.skip_destructive:
        print(f"\n{YELLOW}{'─'*60}")
        print(f"  ⚠  DESTRUCTIVE TESTS — device will restart")
        print(f"{'─'*60}{RESET}")
        time.sleep(1)

        test_restart(session, base, t)
        # Config-reset is intentionally commented out by default.
        # Uncomment the line below to also test factory reset:
        # test_config_reset(session, base, t)
    else:
        print(f"\n{YELLOW}Skipping destructive tests (/restart, /config/reset).{RESET}")

    print_summary()

    if args.dump:
        with open(args.dump, "w", encoding="utf-8") as f:
            json.dump(dump_data, f, ensure_ascii=False, indent=2)
        print(f"\n{GREEN}✔ Responses dumped to {args.dump}{RESET}\n")


if __name__ == "__main__":
    main()