# 05 — Payload y decoder

## Contrato `schema_version: 1` (14 bytes, big-endian)

| Bytes | Tipo | Campo JSON | Escala |
|-------|------|------------|--------|
| 0–1 | int16 | `temp_dht_c` | /10 → °C |
| 2–3 | uint16 | `humidity_pct` | /10 → % |
| 4–5 | uint16 | `mq135_adc` | 0–4095 |
| 6–7 | int16 | `temp_bmp_c` | /10 → °C (BMP280) |
| 8–9 | uint16 | `pressure_hpa` | /10 → hPa |
| 10–11 | int16 | `pitch_deg` | /10 → ° |
| 12–13 | int16 | `roll_deg` | /10 → ° |

- **FPort:** 2  
- **Intervalo:** ~60 s  

## JSON tras decode

```json
{
  "mode": "sensors",
  "temp_dht_c": 28.3,
  "humidity_pct": 43.6,
  "mq135_adc": 718,
  "temp_bmp_c": 29.1,
  "pressure_hpa": 697.3,
  "pitch_deg": 7.3,
  "roll_deg": 3.4,
  "interval_s": 60,
  "source": "heltec_v3_gy91",
  "schema_version": 1
}
```

## Beacon legacy (4 bytes)

`BE AC <nodeId> <counter>` → `mode: "beacon"`. No insertar como lectura de sensores.

## Archivo listo para TTN

Copia/pega: [`config/ttn-uplink-decoder.js`](../config/ttn-uplink-decoder.js)

En TTN: Application → Payload formatters → Uplink → Custom JavaScript formatter.
