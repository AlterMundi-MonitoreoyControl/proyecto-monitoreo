#!/bin/bash
# Utilidad Modbus RS485 - Wrapper
set -x

VENV_DIR="$HOME/.local/share/modbus-util/venv"
SCRIPT="$HOME/.local/share/modbus-util/modbus_util.py"

# Crear venv si no existe
if [ ! -d "$VENV_DIR" ]; then
    echo "Instalando dependencias (primera ejecución)..."
    python3 -m venv "$VENV_DIR"
    "$VENV_DIR/bin/pip" install --quiet pymodbus pyserial
    cp modbus_util.py "$SCRIPT"
    echo "Listo!"
    echo
fi

# Ejecutar
exec "$VENV_DIR/bin/python" "$SCRIPT" "$@"
