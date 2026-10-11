# 06 — Backend Supabase + frontend Vue

## Cadena

```text
TTN uplink → Webhook → Edge Function `ttn-uplink` → Postgres → Realtime → Vue
```

## Schema SQL

Ejecutar en Supabase SQL Editor:

[`config/supabase-schema.sql`](../config/supabase-schema.sql)

Tablas:

- `nodes` — registro de dispositivos
- `sensor_readings` — histórico (~1 fila/min/nodo)
- `risk_events` — alertas por umbrales (opcional)

## Campos de lectura (fuente de verdad)

| Columna | UI sugerida |
|---------|-------------|
| `temp_dht_c` | Temperatura (°C) |
| `humidity_pct` | Humedad (%) |
| `mq135_adc` | Índice gas/humo (ADC) |
| `temp_bmp_c` | Temp. barómetro (°C) |
| `pressure_hpa` | Presión (hPa) |
| `pitch_deg` / `roll_deg` | Inclinación |

**No usar como fuente:** CO₂, PM2.5, acc/gyro crudos, cámara.

## Edge Function (resumen)

1. Validar `TTN_WEBHOOK_SECRET`.
2. Leer `decoded_payload` del uplink TTN v3.
3. Si `mode === "beacon"` → 200 sin insertar lectura.
4. Resolver `nodes` por `ttn_device_id` o `deveui`.
5. Insertar en `sensor_readings` (+ riesgo opcional).

URL ejemplo:

`https://<PROJECT_REF>.supabase.co/functions/v1/ttn-uplink`

## Webhook TTN

Application → Integrations → Webhooks → **Uplink message** → URL de la function + header secreto.

## Vue

- Listar `nodes`
- Mostrar última lectura + series temporales
- Subscribe Realtime a `INSERT` en `sensor_readings`
- Badge de riesgo ambiental (umbrales)

### Variables front

- `VITE_SUPABASE_URL`
- `VITE_SUPABASE_ANON_KEY`

Nunca exponer `SERVICE_ROLE` en el navegador.

## Tipos TypeScript (referencia)

```ts
export type SensorReading = {
  id: number;
  node_id: string;
  received_at: string;
  temp_dht_c: number | null;
  humidity_pct: number | null;
  mq135_adc: number | null;
  temp_bmp_c: number | null;
  pressure_hpa: number | null;
  pitch_deg: number | null;
  roll_deg: number | null;
  rssi?: number | null;
  snr?: number | null;
  schema_version: number;
};
```
