#!/usr/bin/env python3
"""
Utilidad Modbus RS485 - Interfaz interactiva para diagnosticar sensores
"""

import glob
import sys
import time
from datetime import datetime

try:
    from pymodbus.client import ModbusSerialClient
except ImportError:
    print("Error: pymodbus no instalado. Ejecuta: pip install pymodbus pyserial")
    sys.exit(1)

# Colores ANSI
class C:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    CYAN = '\033[96m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    DIM = '\033[2m'
    RESET = '\033[0m'

class ModbusUtil:
    """
    Registros Modbus para sensores Slicetex TH-MB-XXX-DS
    Ref: https://github.com/mspivak/esp-idf-thmbxxx

    Lectura (Function 03):
      0x0000 (0): Humedad × 10
      0x0001 (1): Temperatura × 10

    Calibración (Function 03 leer, 06 escribir):
      0x0050 (80): Corrección humedad (-100 a +100, ÷10 = -10.0 a +10.0%)
      0x0051 (81): Corrección temperatura (-100 a +100, ÷10 = -10.0 a +10.0°C)

    Configuración (Function 03 leer, 06 escribir):
      0x07D0 (2000): Dirección del dispositivo (1-247)
      0x07D1 (2001): Baudrate (0=2400, 1=4800, 2=9600)
    """

    # Registros de calibración (Slicetex)
    REG_HUM_OFFSET = 0x0050   # 80
    REG_TEMP_OFFSET = 0x0051  # 81

    # Registros de configuración (Slicetex)
    REG_SLAVE_ADDR = 0x07D0   # 2000
    REG_BAUDRATE = 0x07D1     # 2001

    # Mapeo de baudrates Slicetex (ref: BAUDRATE_MAP[] = {2400, 4800, 9600})
    BAUD_MAP = {
        0: 2400,
        1: 4800,   # default Slicetex
        2: 9600
    }
    BAUD_MAP_REV = {v: k for k, v in BAUD_MAP.items()}

    def __init__(self):
        self.port = None
        self.baudrate = 9600  # Tu sensor está a 9600
        self.timeout = 1.0    # Timeout 1s como mbpoll
        self.client = None

    def detect_ports(self):
        """Detecta puertos serial disponibles"""
        ports = sorted(glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*'))
        return ports

    def connect(self):
        """Conecta al puerto configurado"""
        if not self.port:
            return False
        try:
            self.client = ModbusSerialClient(
                port=self.port,
                baudrate=self.baudrate,
                parity='N',
                stopbits=1,
                bytesize=8,
                timeout=self.timeout
            )
            return self.client.connect()
        except Exception as e:
            print(f"{C.RED}Error de conexión: {e}{C.RESET}")
            return False

    def disconnect(self):
        """Desconecta del puerto"""
        if self.client:
            self.client.close()
            self.client = None

    def read_sensor(self, addr):
        """Lee temperatura y humedad de un sensor"""
        if not self.client:
            return None, None, None
        try:
            # pymodbus 3.x usa 'device_id' en lugar de 'slave' o 'unit'
            result = self.client.read_holding_registers(address=0, count=2, device_id=addr)
            if not result.isError():
                hum = result.registers[0] / 10.0
                temp = result.registers[1] / 10.0
                return True, temp, hum
        except Exception:
            pass
        return False, None, None

    def read_register(self, addr, register):
        """Lee un registro de configuración"""
        if not self.client:
            return None
        try:
            result = self.client.read_holding_registers(address=register, count=1, device_id=addr)
            if not result.isError():
                return result.registers[0]
        except Exception:
            pass
        return None

    def write_register(self, addr, register, value):
        """Escribe un registro de configuración (Function 06)"""
        if not self.client:
            return False
        try:
            result = self.client.write_register(address=register, value=value, device_id=addr)
            return not result.isError()
        except Exception as e:
            print(f"{C.RED}Error escribiendo registro: {e}{C.RESET}")
            return False

    def read_sensor_config(self, addr):
        """Lee la configuración actual del sensor"""
        config = {}

        # Leer dirección configurada
        val = self.read_register(addr, self.REG_SLAVE_ADDR)
        config['address'] = val

        # Leer baudrate
        val = self.read_register(addr, self.REG_BAUDRATE)
        config['baudrate_code'] = val
        config['baudrate'] = self.BAUD_MAP.get(val, f"desconocido ({val})")

        # Leer offsets
        val = self.read_register(addr, self.REG_TEMP_OFFSET)
        if val is not None:
            # Manejar valores negativos (complemento a 2)
            if val > 32767:
                val = val - 65536
            config['temp_offset'] = val / 10.0
        else:
            config['temp_offset'] = None

        val = self.read_register(addr, self.REG_HUM_OFFSET)
        if val is not None:
            if val > 32767:
                val = val - 65536
            config['hum_offset'] = val / 10.0
        else:
            config['hum_offset'] = None

        return config

    def clear_screen(self):
        """Limpia la pantalla"""
        print('\033[2J\033[H', end='')

    def print_header(self):
        """Imprime el encabezado"""
        self.clear_screen()
        print(f"{C.CYAN}{C.BOLD}")
        print("╔══════════════════════════════════════════╗")
        print("║       Utilidad Modbus RS485              ║")
        print("╚══════════════════════════════════════════╝")
        print(f"{C.RESET}")

        # Estado actual
        port_str = self.port if self.port else "(no seleccionado)"
        print(f"{C.DIM}Puerto: {C.RESET}{port_str}")
        print(f"{C.DIM}Baudrate: {C.RESET}{self.baudrate}")
        print()

    def menu_select_port(self):
        """Menú para seleccionar puerto"""
        ports = self.detect_ports()

        if not ports:
            print(f"\n{C.RED}No se detectaron puertos USB serial{C.RESET}")
            print("Verificar que el adaptador RS485 esté conectado")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        print(f"\n{C.BOLD}Puertos disponibles:{C.RESET}")
        for i, port in enumerate(ports, 1):
            marker = " <--" if port == self.port else ""
            print(f"  {i}. {port}{C.GREEN}{marker}{C.RESET}")
        print(f"  0. Cancelar")

        try:
            choice = input(f"\n{C.CYAN}Selecciona puerto [1-{len(ports)}]: {C.RESET}")
            idx = int(choice)
            if 1 <= idx <= len(ports):
                self.port = ports[idx - 1]
                print(f"{C.GREEN}Puerto seleccionado: {self.port}{C.RESET}")
        except (ValueError, IndexError):
            pass

    def menu_select_baudrate(self):
        """Menú para seleccionar baudrate"""
        baudrates = [4800, 9600,2400, 19200, 38400, 115200]

        print(f"\n{C.BOLD}Baudrates disponibles:{C.RESET}")
        for i, baud in enumerate(baudrates, 1):
            marker = " <--" if baud == self.baudrate else ""
            print(f"  {i}. {baud}{C.GREEN}{marker}{C.RESET}")
        print(f"  0. Cancelar")

        try:
            choice = input(f"\n{C.CYAN}Selecciona baudrate [1-{len(baudrates)}]: {C.RESET}")
            idx = int(choice)
            if 1 <= idx <= len(baudrates):
                self.baudrate = baudrates[idx - 1]
                print(f"{C.GREEN}Baudrate seleccionado: {self.baudrate}{C.RESET}")
        except (ValueError, IndexError):
            pass

    def scan_bus(self):
        """Escanea el bus buscando sensores"""
        if not self.port:
            print(f"\n{C.RED}Primero selecciona un puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        # Pedir rango de direcciones
        addr_input = input(f"\n{C.CYAN}Rango de direcciones [1-32]: {C.RESET}").strip()
        if not addr_input:
            addr_input = "1-32"

        # Parsear rango
        addresses = []
        try:
            for part in addr_input.split(','):
                if '-' in part:
                    start, end = map(int, part.split('-'))
                    addresses.extend(range(start, end + 1))
                else:
                    addresses.append(int(part))
            addresses = sorted(set(addresses))
        except ValueError:
            print(f"{C.RED}Formato inválido. Usar: 1-10 o 1,2,5{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        print(f"\n{C.BOLD}Escaneando {len(addresses)} direcciones en {self.port} @ {self.baudrate}...{C.RESET}\n")

        if not self.connect():
            print(f"{C.RED}No se pudo conectar al puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        found = []
        for addr in addresses:
            success, temp, hum = self.read_sensor(addr)
            if success:
                found.append((addr, temp, hum))
                print(f"  {C.GREEN}✓ Addr {addr:3d}: Temp={temp:5.1f}°C  Hum={hum:5.1f}%{C.RESET}")
            else:
                print(f"  {C.DIM}· Addr {addr:3d}: sin respuesta{C.RESET}", end='\r')
            time.sleep(0.05)

        self.disconnect()

        print("\n" + "="*45)
        if found:
            print(f"{C.GREEN}{C.BOLD}ENCONTRADOS: {len(found)} sensor(es){C.RESET}")
            for addr, temp, hum in found:
                print(f"  - Dirección {addr}: T={temp:.1f}°C, H={hum:.1f}%")
        else:
            print(f"{C.YELLOW}No se encontraron sensores{C.RESET}")
            print(f"\n{C.DIM}Verificar:")
            print("  - Alimentación del sensor (5-30V DC)")
            print("  - Cableado A/B (probar invertir)")
            print(f"  - Baudrate (actual: {self.baudrate}){C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def monitor_sensor(self):
        """Monitorea un sensor en tiempo real"""
        if not self.port:
            print(f"\n{C.RED}Primero selecciona un puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        # Pedir dirección
        try:
            addr_input = input(f"\n{C.CYAN}Dirección del sensor [1]: {C.RESET}").strip()
            addr = int(addr_input) if addr_input else 1
        except ValueError:
            print(f"{C.RED}Dirección inválida{C.RESET}")
            return

        print(f"\n{C.BOLD}Monitoreando sensor addr={addr} en {self.port} @ {self.baudrate}{C.RESET}")
        print(f"{C.YELLOW}Presiona Ctrl+C para volver al menú{C.RESET}\n")

        if not self.connect():
            print(f"{C.RED}No se pudo conectar al puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        try:
            while True:
                timestamp = datetime.now().strftime('%H:%M:%S')
                success, temp, hum = self.read_sensor(addr)

                if success:
                    print(f"  [{timestamp}] {C.GREEN}Temp={temp:5.1f}°C  Hum={hum:5.1f}%{C.RESET}")
                else:
                    print(f"  [{timestamp}] {C.RED}Sin respuesta{C.RESET}")

                time.sleep(1)

        except KeyboardInterrupt:
            print(f"\n\n{C.CYAN}Monitoreo detenido{C.RESET}")
        finally:
            self.disconnect()

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def auto_scan_all_bauds(self):
        """Escaneo automático probando todos los baudrates"""
        if not self.port:
            print(f"\n{C.RED}Primero selecciona un puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        baudrates = [9600, 4800,2400, 19200, 38400]
        addresses = range(1, 33)
        all_found = []

        print(f"\n{C.BOLD}Escaneo automático en {self.port}{C.RESET}")
        print(f"Probando baudrates: {baudrates}")
        print(f"Direcciones: 1-32\n")

        for baud in baudrates:
            print(f"\n{C.CYAN}--- Baudrate {baud} ---{C.RESET}")
            self.baudrate = baud

            if not self.connect():
                print(f"{C.RED}  Error de conexión{C.RESET}")
                continue

            found_at_baud = []
            for addr in addresses:
                success, temp, hum = self.read_sensor(addr)
                if success:
                    found_at_baud.append((addr, temp, hum))
                    all_found.append((baud, addr, temp, hum))
                    print(f"  {C.GREEN}✓ Addr {addr}: T={temp:.1f}°C H={hum:.1f}%{C.RESET}")
                time.sleep(0.03)

            self.disconnect()

            if not found_at_baud:
                print(f"  {C.DIM}Sin respuesta{C.RESET}")

        print("\n" + "="*50)
        if all_found:
            print(f"{C.GREEN}{C.BOLD}RESUMEN - Sensores encontrados:{C.RESET}")
            for baud, addr, temp, hum in all_found:
                print(f"  Baud {baud}, Addr {addr}: T={temp:.1f}°C, H={hum:.1f}%")
        else:
            print(f"{C.YELLOW}No se encontraron sensores en ningún baudrate{C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def sensor_config_menu(self):
        """Submenú de configuración de sensores"""
        if not self.port:
            print(f"\n{C.RED}Primero selecciona un puerto{C.RESET}")
            input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
            return

        # Pedir dirección actual del sensor
        try:
            addr_input = input(f"\n{C.CYAN}Dirección actual del sensor [1]: {C.RESET}").strip()
            addr = int(addr_input) if addr_input else 1
        except ValueError:
            print(f"{C.RED}Dirección inválida{C.RESET}")
            return

        while True:
            self.print_header()
            print(f"{C.BOLD}Configuración del sensor (addr={addr}){C.RESET}\n")

            # Conectar y leer config actual
            if not self.connect():
                print(f"{C.RED}No se pudo conectar al puerto{C.RESET}")
                input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
                return

            # Verificar que el sensor responde
            success, temp, hum = self.read_sensor(addr)
            if success:
                print(f"{C.GREEN}Sensor detectado: T={temp:.1f}°C, H={hum:.1f}%{C.RESET}\n")

                # Leer configuración
                config = self.read_sensor_config(addr)
                print(f"{C.DIM}Configuración actual:{C.RESET}")
                print(f"  Dirección guardada: {config.get('address', '?')}")
                print(f"  Baudrate: {config.get('baudrate', '?')}")
                if config.get('temp_offset') is not None:
                    print(f"  Offset temperatura: {config['temp_offset']:+.1f}°C")
                if config.get('hum_offset') is not None:
                    print(f"  Offset humedad: {config['hum_offset']:+.1f}%")
            else:
                print(f"{C.RED}Sensor no responde en addr={addr}{C.RESET}")

            self.disconnect()

            print(f"\n{C.BOLD}Opciones:{C.RESET}")
            print(f"  1. Cambiar dirección Modbus")
            print(f"  2. Cambiar baudrate")
            print(f"  3. Ajustar offset de temperatura")
            print(f"  4. Ajustar offset de humedad")
            print(f"  5. Cambiar sensor (otra dirección)")
            print(f"  0. Volver al menú principal")

            choice = input(f"\n{C.CYAN}Opción: {C.RESET}").strip()

            if choice == '1':
                self.change_sensor_address(addr)
            elif choice == '2':
                self.change_sensor_baudrate(addr)
            elif choice == '3':
                self.change_temp_offset(addr)
            elif choice == '4':
                self.change_hum_offset(addr)
            elif choice == '5':
                try:
                    new_addr = input(f"\n{C.CYAN}Nueva dirección a consultar [1-247]: {C.RESET}").strip()
                    addr = int(new_addr)
                except ValueError:
                    pass
            elif choice == '0':
                break

    def change_sensor_address(self, current_addr):
        """Cambia la dirección Modbus del sensor"""
        print(f"\n{C.YELLOW}⚠ ADVERTENCIA: Cambiar la dirección requiere reiniciar el sensor{C.RESET}")
        print(f"{C.DIM}Después de cambiar, deberás usar la nueva dirección para comunicarte{C.RESET}\n")

        try:
            new_addr = input(f"{C.CYAN}Nueva dirección [1-247]: {C.RESET}").strip()
            new_addr = int(new_addr)
            if not (1 <= new_addr <= 247):
                print(f"{C.RED}Dirección debe estar entre 1 y 247{C.RESET}")
                input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")
                return
        except ValueError:
            print(f"{C.RED}Valor inválido{C.RESET}")
            return

        confirm = input(f"\n¿Cambiar dirección de {current_addr} a {new_addr}? [s/N]: ").strip().lower()
        if confirm != 's':
            print("Cancelado")
            return

        if not self.connect():
            print(f"{C.RED}No se pudo conectar{C.RESET}")
            return

        success = self.write_register(current_addr, self.REG_SLAVE_ADDR, new_addr)
        self.disconnect()

        if success:
            print(f"\n{C.GREEN}✓ Dirección cambiada a {new_addr}{C.RESET}")
            print(f"{C.YELLOW}Reinicia el sensor para aplicar el cambio{C.RESET}")
        else:
            print(f"\n{C.RED}✗ Error al cambiar dirección{C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def change_sensor_baudrate(self, addr):
        """Cambia el baudrate del sensor"""
        print(f"\n{C.YELLOW}⚠ ADVERTENCIA: Cambiar baudrate requiere reiniciar el sensor{C.RESET}")
        print(f"{C.DIM}Después deberás cambiar también el baudrate de esta utilidad{C.RESET}\n")

        print(f"{C.BOLD}Baudrates disponibles:{C.RESET}")
        print(f"  0. 2400")
        print(f"  1. 4800 (default Slicetex)")
        print(f"  2. 9600")

        try:
            choice = input(f"\n{C.CYAN}Selección [0-2]: {C.RESET}").strip()
            baud_code = int(choice)
            if baud_code not in self.BAUD_MAP:
                print(f"{C.RED}Opción inválida{C.RESET}")
                return
        except ValueError:
            print(f"{C.RED}Valor inválido{C.RESET}")
            return

        new_baud = self.BAUD_MAP[baud_code]
        confirm = input(f"\n¿Cambiar baudrate a {new_baud}? [s/N]: ").strip().lower()
        if confirm != 's':
            print("Cancelado")
            return

        if not self.connect():
            print(f"{C.RED}No se pudo conectar{C.RESET}")
            return

        success = self.write_register(addr, self.REG_BAUDRATE, baud_code)
        self.disconnect()

        if success:
            print(f"\n{C.GREEN}✓ Baudrate cambiado a {new_baud}{C.RESET}")
            print(f"{C.YELLOW}Reinicia el sensor y cambia el baudrate de esta utilidad{C.RESET}")
        else:
            print(f"\n{C.RED}✗ Error al cambiar baudrate{C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def change_temp_offset(self, addr):
        """Ajusta el offset de temperatura para calibración"""
        print(f"\n{C.BOLD}Ajuste de offset de temperatura{C.RESET}")
        print(f"{C.DIM}Rango válido: -10.0 a +10.0 °C{C.RESET}\n")

        try:
            offset_str = input(f"{C.CYAN}Offset en °C (ej: -1.5 o +2.0): {C.RESET}").strip()
            offset = float(offset_str)
            if not (-10.0 <= offset <= 10.0):
                print(f"{C.RED}Offset debe estar entre -10.0 y +10.0{C.RESET}")
                return
        except ValueError:
            print(f"{C.RED}Valor inválido{C.RESET}")
            return

        # Convertir a valor de registro (×10, complemento a 2 para negativos)
        reg_value = int(offset * 10)
        if reg_value < 0:
            reg_value = 65536 + reg_value

        confirm = input(f"\n¿Aplicar offset de {offset:+.1f}°C? [s/N]: ").strip().lower()
        if confirm != 's':
            print("Cancelado")
            return

        if not self.connect():
            print(f"{C.RED}No se pudo conectar{C.RESET}")
            return

        success = self.write_register(addr, self.REG_TEMP_OFFSET, reg_value)
        self.disconnect()

        if success:
            print(f"\n{C.GREEN}✓ Offset de temperatura: {offset:+.1f}°C{C.RESET}")
        else:
            print(f"\n{C.RED}✗ Error al cambiar offset{C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def change_hum_offset(self, addr):
        """Ajusta el offset de humedad para calibración"""
        print(f"\n{C.BOLD}Ajuste de offset de humedad{C.RESET}")
        print(f"{C.DIM}Rango válido: -10.0 a +10.0 %{C.RESET}\n")

        try:
            offset_str = input(f"{C.CYAN}Offset en % (ej: -2.0 o +3.5): {C.RESET}").strip()
            offset = float(offset_str)
            if not (-10.0 <= offset <= 10.0):
                print(f"{C.RED}Offset debe estar entre -10.0 y +10.0{C.RESET}")
                return
        except ValueError:
            print(f"{C.RED}Valor inválido{C.RESET}")
            return

        # Convertir a valor de registro
        reg_value = int(offset * 10)
        if reg_value < 0:
            reg_value = 65536 + reg_value

        confirm = input(f"\n¿Aplicar offset de {offset:+.1f}%? [s/N]: ").strip().lower()
        if confirm != 's':
            print("Cancelado")
            return

        if not self.connect():
            print(f"{C.RED}No se pudo conectar{C.RESET}")
            return

        success = self.write_register(addr, self.REG_HUM_OFFSET, reg_value)
        self.disconnect()

        if success:
            print(f"\n{C.GREEN}✓ Offset de humedad: {offset:+.1f}%{C.RESET}")
        else:
            print(f"\n{C.RED}✗ Error al cambiar offset{C.RESET}")

        input(f"\n{C.DIM}Presiona Enter para continuar...{C.RESET}")

    def main_menu(self):
        """Menú principal"""
        while True:
            self.print_header()

            print(f"{C.BOLD}Opciones:{C.RESET}")
            print(f"  1. Seleccionar puerto")
            print(f"  2. Seleccionar baudrate")
            print(f"  3. Escanear bus (buscar sensores)")
            print(f"  4. Monitorear sensor")
            print(f"  5. Auto-escaneo (todos los baudrates)")
            print(f"  6. {C.YELLOW}Configurar sensor (addr/baud/calibración){C.RESET}")
            print(f"  0. Salir")

            choice = input(f"\n{C.CYAN}Opción: {C.RESET}").strip()

            if choice == '1':
                self.menu_select_port()
            elif choice == '2':
                self.menu_select_baudrate()
            elif choice == '3':
                self.scan_bus()
            elif choice == '4':
                self.monitor_sensor()
            elif choice == '5':
                self.auto_scan_all_bauds()
            elif choice == '6':
                self.sensor_config_menu()
            elif choice == '0' or choice.lower() == 'q':
                print(f"\n{C.CYAN}Hasta luego!{C.RESET}\n")
                break

def main():
    util = ModbusUtil()

    # Auto-detectar primer puerto si existe
    ports = util.detect_ports()
    if ports:
        util.port = ports[0]

    util.main_menu()

if __name__ == '__main__':
    main()
