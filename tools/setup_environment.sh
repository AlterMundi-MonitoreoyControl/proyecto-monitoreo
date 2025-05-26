# Install dependencies
sudo apt update && sudo apt install -y python3 pip git unzip curl

# Install Arduino CLI
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Configure Arduino CLI
arduino-cli config init
arduino-cli config set board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32

# Install required libraries
arduino-cli lib install "WiFiManager"
arduino-cli lib install "HTTPClient"
arduino-cli lib install "Adafruit SCD30"
arduino-cli lib install "ArduinoJson"

# Compile and upload firmware
#
# arduino-cli compile --fqbn esp32:esp32:esp32 SendToGrafana.ino --build-path ./bins
# arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 --input-dir ./bins

## PLATFORMIO
pio test -e native_test -vvv
pio run -e esp32dev
pio run -e esp32dev -t upload
