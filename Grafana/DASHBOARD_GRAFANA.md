# Dashboard de Grafana - Monitoreo de Sensores

Dashboard de Grafana para visualizar datos de sensores de temperatura, humedad y CO2.

## Características

**Variables dinámicas:**
- Dispositivo: filtra por dispositivo específico o todos
- Tipo de sensor: SCD30, BME280, DHT22, mesh_XXX, etc.

**Paneles:**
- Stats de resumen (dispositivos activos, promedios)
- Gráficas temporales por dispositivo (temp, hum, CO2)
- Tabla de estado actual con colores según rangos
- Mapa de calor de CO2

## Umbrales

| Métrica | Verde | Amarillo | Naranja | Rojo |
|---------|-------|----------|---------|------|
| Temp    | 18-25°C | 25-30°C | - | <18 o >30°C |
| Hum     | 40-70% | 30-40% o 70-80% | - | <30 o >80% |
| CO2     | <800 ppm | 800-1000 | 1000-1500 | >1500 |

## Instalación

1. Acceder a `https://grafana.altermundi.net`
2. Ir a **Dashboards** → **Import**
3. Subir `grafana-dashboard.json`
4. Seleccionar fuente de datos **InfluxDB**
5. Click en **Import**

## Configuración de Fuente de Datos

| Parámetro | Valor |
|-----------|-------|
| Tipo | InfluxDB |
| URL | `http://localhost:8086` |
| Database | `cto` |
| HTTP Method | GET |

Query de prueba:
```sql
SELECT * FROM "medicionesCO2" WHERE time > now() - 1h
```

## Estructura de Datos

**Measurement:** `medicionesCO2`

**Tags:**
- `device`: ID del dispositivo (ej: `moni-80F3DAAD`)
- `sensor`: Tipo de sensor (ej: `SCD30`, `BME280`)

**Fields:**
- `temp`: Temperatura en °C
- `hum`: Humedad en %
- `co2`: CO2 en ppm

Ejemplo:
```
medicionesCO2,device=moni-80F3DAAD,sensor=SCD30 temp=23.5,hum=60.2,co2=450 1699876543000000000
```

## Alertas

Alerta pre-configurada de CO2:
- Condición: CO2 promedio > 1000 ppm durante 5 minutos
- Frecuencia: 1 minuto

Configurar notificaciones en **Alerting** → **Notification channels**.

## Personalización

Modificar umbrales de CO2 en el JSON:
```json
"thresholds": {
  "steps": [
    {"value": null, "color": "green"},
    {"value": 800, "color": "yellow"},
    {"value": 1000, "color": "orange"},
    {"value": 1500, "color": "red"}
  ]
}
```

Variables adicionales (ej: ubicación):
```json
{
  "name": "ubicacion",
  "type": "query",
  "query": "SHOW TAG VALUES FROM \"medicionesCO2\" WITH KEY = \"ubicacion\""
}
```

## Troubleshooting

**No aparecen datos:**

1. Verificar fuente de datos:
   ```bash
   curl "http://grafana.altermundi.net:8086/query?db=cto&q=SELECT * FROM medicionesCO2 LIMIT 10"
   ```

2. Verificar sensores:
   - Logs del ESP32
   - Conectividad WiFi
   - URL en `constants_private.h`

3. Query vacío: ajustar rango de tiempo, verificar filtros y nombres de tags/fields.

## Backup

```bash
curl -H "Authorization: Bearer YOUR_API_KEY" \
  https://grafana.altermundi.net/api/dashboards/uid/DASHBOARD_UID \
  > backup-dashboard-$(date +%Y%m%d).json
```

---

**Versión**: 1.0 | **Grafana**: 7.4+ | **DB**: InfluxDB 1.x
