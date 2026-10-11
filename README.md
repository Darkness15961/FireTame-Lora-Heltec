# fireTame-heltec

Sistema de **prevención de incendios** con nodos **Heltec WiFi LoRa 32 V3**, gateway LoRaWAN (**HT-M7603**), **The Things Network** y backend/dashboard **Supabase + Vue**.

```text
Sensores → Heltec V3 → LoRaWAN → HT-M7603 → TTN (nam1 / US915)
                                              ↓
                                    Webhook → Supabase
                                              ↓
                                         Dashboard Vue
```

## Conexiones Heltec WiFi LoRa 32 V3 ↔ sensores

Placa: **Heltec WiFi LoRa 32 V3** (USB hacia abajo). Sensores: **DHT22**, **MQ135**, **GY-91** (BMP280 + MPU).

### Alimentación

| Pin Heltec | Conectar a |
|------------|------------|
| **3V3** | VCC DHT22 + **VIN** GY-91 |
| **5V** | VCC MQ135 (calentador) |
| **GND** | GND de DHT22, MQ135 y GY-91 |

### Señales

| Sensor | Pin del sensor | Pin Heltec (GPIO) |
|--------|----------------|-------------------|
| **DHT22** | DATA | **GPIO4** |
| **MQ135** | AO (analógico) | **GPIO2** |
| **MQ135** | DO | no conectar |
| **GY-91** | SDA | **GPIO47** |
| **GY-91** | SCL | **GPIO48** |

### GY-91 — pines que no se usan (modo I2C)

| Pin GY-91 | Acción |
|-----------|--------|
| SDO / SAO | sin cable |
| NCS | sin cable |
| CSB | sin cable |

### Resumen rápido

```text
Heltec 3V3  ──►  DHT22 VCC, GY-91 VIN
Heltec 5V   ──►  MQ135 VCC
Heltec GND  ──►  GND (todos)

GPIO4   ──►  DHT22 DATA
GPIO2   ──►  MQ135 AO
GPIO47  ──►  GY-91 SDA
GPIO48  ──►  GY-91 SCL
```

Notas:

- GPIO17/18 son del OLED interno y **no salen** al header.
- Si el DHT22 no trae pull-up: 4.7k–10k entre DATA y 3V3.
- AO del MQ135 debe ser ≤ 3.3 V hacia el ESP32.
- Detalle ampliado: [docs/02-hardware-y-cableado.md](docs/02-hardware-y-cableado.md)

## Estructura del repositorio

```text
fireTame-heltec/
├── README.md                 ← este archivo
├── docs/                     ← guías paso a paso
├── firmware/                 ← sketches Arduino por nodo
│   ├── nodo-1/
│   ├── nodo-2/
│   └── nodo-3/
├── config/                   ← decoder TTN + schema SQL
└── assets/                   ← imágenes / diagramas (opcionales)
```

## Documentación

| Guía | Contenido |
|------|-----------|
| [docs/01-arquitectura.md](docs/01-arquitectura.md) | Visión del sistema y componentes |
| [docs/02-hardware-y-cableado.md](docs/02-hardware-y-cableado.md) | Pines y conexiones de sensores |
| [docs/03-gateway-y-ttn.md](docs/03-gateway-y-ttn.md) | HT-M7603 + registro en TTN |
| [docs/04-firmware.md](docs/04-firmware.md) | Compilar y cargar nodos |
| [docs/05-payload-decoder.md](docs/05-payload-decoder.md) | Formato de datos + decoder |
| [docs/06-backend-supabase-vue.md](docs/06-backend-supabase-vue.md) | BD, webhook, dashboard |
| [docs/07-puesta-en-marcha.md](docs/07-puesta-en-marcha.md) | Checklist para levantar todo |
| [docs/08-secretos.md](docs/08-secretos.md) | `.env` y `secrets.h` |

## Arranque rápido

1. Configurar gateway y TTN → `docs/03-gateway-y-ttn.md`
2. Cablear sensores → `docs/02-hardware-y-cableado.md`
3. Cargar firmware del nodo → `firmware/nodo-X` + `docs/04-firmware.md`
4. Pegar decoder en TTN → `config/ttn-uplink-decoder.js`
5. Aplicar SQL en Supabase → `config/supabase-schema.sql`
6. Conectar webhook TTN → Edge Function (ver doc 06)
7. Levantar dashboard Vue (repo front aparte o carpeta `web/` si se añade)

## Nodos TTN

| Nodo | TTN device id | DevEUI |
|------|---------------|--------|
| Nodo 1 | `heltec-wifi-lora-32-v3` | `70B3D57ED0079581` |
| Nodo 2 | `heltec-nodo-2` | `70B3D57ED0079582` |
| Nodo 3 | `heltec-nodo-3` | `70B3D57ED0079584` |

App TTN: `heltec-v3-peru` · Cluster: [nam1 console](https://nam1.cloud.thethings.network/console/)

## Datos que envía cada nodo (cada ~60 s)

`temp_dht_c`, `humidity_pct`, `mq135_adc`, `temp_bmp_c`, `pressure_hpa`, `pitch_deg`, `roll_deg`

## Secretos (no van a GitHub)

```bash
cp .env.example .env
# edita .env con AppKeys, licenses Heltec, Supabase, webhook

powershell -ExecutionPolicy Bypass -File scripts/sync-secrets-from-env.ps1
# genera firmware/nodo-X/secrets.h (gitignored)
```

| Archivo | ¿Se sube? |
|---------|-----------|
| `.env.example` | Sí (plantilla) |
| `.env` | **No** |
| `firmware/**/secrets.example.h` | Sí |
| `firmware/**/secrets.h` | **No** |

No subas `SERVICE_ROLE` de Supabase ni `TTN_WEBHOOK_SECRET` al frontend.

## Licencia

Proyecto académico / prototipo — define la licencia al publicar en GitHub.
