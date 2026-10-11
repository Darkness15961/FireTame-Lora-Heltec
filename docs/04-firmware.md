# 04 — Firmware (Arduino)

## Requisitos

- Arduino IDE 2.x
- Board package Heltec: **WiFi LoRa 32(V3)**
- Librerías:
  - Heltec ESP32 Dev-Boards (incluye LoRaWAN + BMP280)
  - DHT sensor library
  - Adafruit Unified Sensor (dependencia DHT)

## Carpeta por nodo

| Carpeta | Archivo | TTN device |
|---------|---------|------------|
| `firmware/nodo-1/` | `nodo-1.ino` | `heltec-wifi-lora-32-v3` |
| `firmware/nodo-2/` | `nodo-2.ino` | `heltec-nodo-2` |
| `firmware/nodo-3/` | `nodo-3.ino` | `heltec-nodo-3` |

Cada sketch incluye: OTAA keys, license Heltec, sensores DHT22 + MQ135 + GY-91, OLED (nombre + estado + datos).

## Credenciales (antes de compilar)

Las claves OTAA y licenses Heltec **no** están en el `.ino`. Vienen de `.env`:

```bash
# en la raíz fireTame-heltec
cp .env.example .env   # si aún no existe
# rellena NODO1_APPKEY, NODO*_LICENSE_*, etc.

powershell -ExecutionPolicy Bypass -File scripts/sync-secrets-from-env.ps1
```

Eso crea `firmware/nodo-X/secrets.h` (ignorado por git).  
Sin ese archivo, el sketch no compila.

## Cómo cargar

1. Genera `secrets.h` (paso anterior).
2. Conecta **solo** la placa del nodo correspondiente.
3. Abre el `.ino` de esa carpeta.
4. Tools:
   - Board: **WiFi LoRa 32(V3)**
   - **LORAWAN_REGION: REGION_US915**
   - Port: COM de esa placa
5. Antena conectada → **Upload**.
6. Serial Monitor **115200**.

## Serial esperado (éxito)

```text
Nodo 2 (id=2) sensores GY-91
BMP280 OK (GY-91)
MPU OK WHO_AM_I=0x70 @0x68
joining...
joined
[Nodo 2] DHT T=28.3 H=43.6 MQ=718 BMP T=29.1 P=697.3 Pitch=7.3 Roll=3.4
unconfirmed uplink sending ...
```

## OLED

```text
Nodo X
Estado: Conectado | Conectando | Desconectado
T:.. H:.. MQ:..
P:.. Pi:.. Ro:..
```

## Importante

- No mezclar sketches entre placas (cada una tiene DevEUI/AppKey/license propios).
- Tras deep sleep es normal que el Serial muestre de nuevo el banner de arranque.
