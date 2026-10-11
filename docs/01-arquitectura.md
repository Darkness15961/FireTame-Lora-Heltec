# 01 — Arquitectura

## Objetivo

Telemetría ambiental para **prevención de incendios**: temperatura, humedad, índice de gas/humo (MQ135), presión e inclinación del nodo, vía LoRaWAN hacia un dashboard en tiempo casi real.

## Componentes

| Capa | Tecnología |
|------|------------|
| Nodos | Heltec WiFi LoRa 32 V3 + DHT22 + MQ135 + GY-91 |
| Gateway | Heltec HT-M7603 |
| Red | The Things Network (cluster **nam1**, plan **US915 FSB2**) |
| Backend | Supabase (Postgres + Edge Functions + Realtime) |
| Frontend | Vue (proyecto web) |

## Flujo de datos

```text
[DHT22 / MQ135 / GY-91]
        │
        ▼
[Heltec V3] ──LoRaWAN OTAA──► [HT-M7603] ──UDP/TTN──► [TTN nam1]
                                                         │
                                              Payload Formatter
                                                         │
                                                    Webhook HTTP
                                                         │
                                                         ▼
                                              [Supabase Edge Function]
                                                         │
                                              nodes + sensor_readings
                                                         │
                                                         ▼
                                                   [Vue Dashboard]
```

## Reglas importantes

- El **firmware Heltec** es la fuente de verdad del payload (`schema_version: 1`).
- Vue **no** se conecta al nodo; solo a Supabase.
- No se envían imágenes por LoRaWAN.
- Intervalo de uplink: **~1 minuto** por nodo.
