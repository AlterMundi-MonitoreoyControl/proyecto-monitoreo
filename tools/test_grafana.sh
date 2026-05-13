#!/bin/bash

# Script de prueba para verificar conectividad y datos en Grafana/InfluxDB
# AlterMundi - Proyecto Monitoreo

set -e

GRAFANA_URL="http://grafana.altermundi.net:8086"
DATABASE="cto"

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  AlterMundi - Test de Conectividad Grafana/InfluxDB       ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Colores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para test con resultado
test_step() {
    local description="$1"
    local command="$2"

    echo -n "[→ TEST] $description... "

    if eval "$command" > /tmp/grafana_test_output 2>&1; then
        echo -e "${GREEN}✓ OK${NC}"
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        cat /tmp/grafana_test_output
        return 1
    fi
}

echo -e "${BLUE}[1/5] Verificando conectividad a InfluxDB...${NC}"
test_step "Ping a servidor InfluxDB" "curl -s -o /dev/null -w '%{http_code}' $GRAFANA_URL/ping | grep -q 204"
echo ""

echo -e "${BLUE}[2/5] Verificando base de datos...${NC}"
test_step "Database 'cto' existe" "curl -s '$GRAFANA_URL/query?q=SHOW+DATABASES' | grep -q '\"cto\"'"
echo ""

echo -e "${BLUE}[3/5] Verificando measurements...${NC}"
MEASUREMENTS=$(curl -s "$GRAFANA_URL/query?db=$DATABASE&q=SHOW+MEASUREMENTS" | grep -o '"medicionesCO2"' | wc -l)
if [ "$MEASUREMENTS" -gt 0 ]; then
    echo -e "${GREEN}✓${NC} Measurement 'medicionesCO2' encontrado"
else
    echo -e "${RED}✗${NC} Measurement 'medicionesCO2' NO encontrado"
fi
echo ""

echo -e "${BLUE}[4/5] Verificando datos recientes...${NC}"
RECENT_DATA=$(curl -s "$GRAFANA_URL/query?db=$DATABASE&q=SELECT+COUNT(*)+FROM+medicionesCO2+WHERE+time+>+now()-1h" | grep -o '"count_temp":[0-9]*' | cut -d':' -f2)

if [ -n "$RECENT_DATA" ] && [ "$RECENT_DATA" -gt 0 ]; then
    echo -e "${GREEN}✓${NC} Datos recientes encontrados: $RECENT_DATA lecturas en la última hora"
else
    echo -e "${YELLOW}⚠${NC} No hay datos recientes (última hora)"
fi
echo ""

echo -e "${BLUE}[5/5] Estadísticas de dispositivos...${NC}"
echo ""

# Listar dispositivos únicos
echo "Dispositivos activos:"
DEVICES=$(curl -s "$GRAFANA_URL/query?db=$DATABASE&q=SHOW+TAG+VALUES+FROM+medicionesCO2+WITH+KEY=device" 2>/dev/null)
echo "$DEVICES" | grep -o '"value":"[^"]*"' | cut -d'"' -f4 | while read device; do
    echo "  • $device"
done
echo ""

# Listar tipos de sensores
echo "Tipos de sensores:"
SENSORS=$(curl -s "$GRAFANA_URL/query?db=$DATABASE&q=SHOW+TAG+VALUES+FROM+medicionesCO2+WITH+KEY=sensor" 2>/dev/null)
echo "$SENSORS" | grep -o '"value":"[^"]*"' | cut -d'"' -f4 | while read sensor; do
    echo "  • $sensor"
done
echo ""

# Obtener últimas lecturas
echo -e "${BLUE}Últimas lecturas (últimos 5 minutos):${NC}"
LAST_READINGS=$(curl -s "$GRAFANA_URL/query?db=$DATABASE&q=SELECT+*+FROM+medicionesCO2+WHERE+time+>+now()-5m+ORDER+BY+time+DESC+LIMIT+5" 2>/dev/null)

if echo "$LAST_READINGS" | grep -q '"series"'; then
    echo "$LAST_READINGS" | python3 -c "
import sys, json
try:
    data = json.load(sys.stdin)
    if 'results' in data and len(data['results']) > 0:
        if 'series' in data['results'][0]:
            for serie in data['results'][0]['series']:
                device = serie['tags']['device']
                sensor = serie['tags']['sensor']
                print(f'\n  📡 {device} ({sensor}):')
                columns = serie['columns']
                for value in serie['values'][:5]:
                    row = dict(zip(columns, value))
                    print(f\"     Temp: {row.get('temp', 'N/A')}°C, Hum: {row.get('hum', 'N/A')}%, CO₂: {row.get('co2', 'N/A')} ppm\")
        else:
            print('  Sin datos recientes')
    else:
        print('  Sin datos recientes')
except Exception as e:
    print(f'  Error procesando datos: {e}')
" 2>/dev/null || echo "  (Instalar python3 para ver lecturas formateadas)"
else
    echo -e "  ${YELLOW}Sin datos en los últimos 5 minutos${NC}"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Resumen final
echo ""
echo -e "${BLUE}Resumen:${NC}"
echo "  📊 Base de datos: $DATABASE @ $GRAFANA_URL"
echo "  🔗 Dashboard URL: https://grafana.altermundi.net"
echo "  📁 Config JSON: grafana-dashboard.json"
echo ""

# Sugerencias si no hay datos
if [ -z "$RECENT_DATA" ] || [ "$RECENT_DATA" -eq 0 ]; then
    echo -e "${YELLOW}⚠ Sugerencias:${NC}"
    echo "  1. Verifica que los sensores ESP32 estén encendidos"
    echo "  2. Confirma conectividad WiFi de los dispositivos"
    echo "  3. Revisa logs del serial monitor"
    echo "  4. Verifica URL en constants_private.h:"
    echo "     const char* URL = \"$GRAFANA_URL/write?db=$DATABASE\";"
    echo ""
fi

echo "✓ Test completado"
