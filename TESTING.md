# Testing in Proyecto Monitoreo

This project uses [PlatformIO's Unit Testing framework](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html) combined with [Unity](http://www.throwtheswitch.org/unity) to ensure the reliability of the control mediator, rules engine, sensor identity logic, and more.

## Test Environments

We have two primary test environments configured in `platformio.ini`:

1. **`native_test`**: Runs directly on your computer (Windows/Linux/Mac). It is very fast and does not require an ESP32 to be connected. It uses mocks to simulate hardware-specific functions.
2. **`esp32dev_multi`**: Runs directly on the ESP32 hardware. It uploads the test firmware to the board, executes the tests using the actual ESP32 framework (and flash memory for SPIFFS), and returns the results via serial.

## How to Run Tests

### 1. Running Native Tests (Local PC)
To run the tests quickly without uploading them to the board, use the native environment:

```bash
pio test -e native_test
```

*Note: Native tests use the `ArduinoFake` library to mock Arduino core functions like `millis()` and `digitalRead()`. They also use custom mocks located in `test/mocks/` to simulate the `ESP` object (for MAC addresses) and `SPIFFS` (for file system operations).*

### 2. Running Hardware Tests (ESP32)
To run the integration tests on the actual hardware, connect your ESP32 via USB and run:

```bash
pio test -e esp32dev_multi
```

*PlatformIO will automatically compile, upload the test firmware, open the serial monitor, and print the test results.*

### 3. API Integration Tests (Python)

We use `pytest` and `requests` to validate the live HTTP API endpoints directly on the ESP32.

**Setup Virtual Environment:**
It is recommended to use a Python virtual environment:

```bash
# Create the virtual environment
python3 -m venv venv

# Activate it 
source venv/bin/activate
# Install dependencies
pip install pytest requests
```

**Run the Tests:**
To run the API invariant checks against your device (change the IP as needed):
```bash
# This will exercise all endpoints, test state changes, and run the Pytest suite
python tools/testendpoints.py --ip 192.168.1.110
```

## Test Architecture

- **`test/testAll.cpp`**: The central entry point for all tests. It contains the `main()` (for native) and `setup()`/`loop()` (for Arduino) functions, and orchestrates the execution of all test suites.
- **`test/testRuleConditions.cpp`**: Tests the logic of the rule engine (`ControlMediator` and `RuleExpr`), including LEAF, AND, and OR evaluations.
- **`test/testSensorIdentity.cpp`**: Tests the generation of `SensorKey`, verifying that device IDs and sensor IDs are calculated and encoded correctly.
- **`test/testAutomationFlow.cpp`**: Simulates a complete automation scenario with mock actuators and sensors.
- **`test/testRuleLoader.cpp`**: Validates the loading and parsing of rules from a JSON file using SPIFFS.
- **`test/mocks/`**: Contains custom mock objects (like `MockESP.h` and `SPIFFS.h`) that replace hardware-specific implementations during native testing.

## Continuous Integration (CI)

Native tests are automatically run on every `push` and `pull_request` to the `main` branch via GitHub Actions. If any test fails, the build pipeline will fail, preventing broken firmware from being released.

## Writing New Tests

1. Create a new file (e.g., `test/testFeature.cpp`).
2. Write your test functions using `TEST_ASSERT...` macros.
3. Create a runner function (e.g., `void runFeatureTests() { RUN_TEST(test_my_feature); }`).
4. Add the runner function definition and call it inside `test/testAll.cpp`.
5. Use `#ifndef ARDUINO` blocks to include mocks when compiling for the native environment.