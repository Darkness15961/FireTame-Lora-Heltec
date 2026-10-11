# 03 — Gateway HT-M7603 y The Things Network

## Gateway

1. Enciende el HT-M7603 y conéctalo a internet (WiFi/Ethernet según modelo).
2. Entra al panel local del gateway (ver docs Heltec HT-M7603).
3. Configura:
   - **Region:** `US915`
   - **Channels:** `8-15,65` (FSB 2 de TTN)
   - **Server:** `nam1.cloud.thethings.network` (puerto Semtech UDP típico `1700`)
4. Registra el gateway en TTN nam1 con su Gateway EUI.

Consola: https://nam1.cloud.thethings.network/console/

## Application TTN

- Application ID: `heltec-v3-peru` (o el nombre que uses)
- Frequency plan en dispositivos: **United States 902-928 MHz, FSB 2 (used by TTN)**
- LoRaWAN: **1.0.3**
- Regional Parameters: **1.0.3 revision A**
- JoinEUI: `0000000000000000`

## End devices

| Nombre | End device ID | DevEUI |
|--------|---------------|--------|
| Nodo 1 | `heltec-wifi-lora-32-v3` | `70B3D57ED0079581` |
| Nodo 2 | `heltec-nodo-2` | `70B3D57ED0079582` |
| Nodo 3 | `heltec-nodo-3` | `70B3D57ED0079584` |

Cada dispositivo tiene su propio **AppKey** (está en el firmware correspondiente).

## Licencia Heltec (obligatoria)

1. Cargar ejemplo `GetChipID` en la placa.
2. Copiar `ESP32ChipID=...`
3. Consultar: https://resource.heltec.cn/search
4. Pegar los 4 valores `0x...` en `license[4]` del sketch.

| Nodo | ChipID |
|------|--------|
| 1 | `E4B4B743CA48` |
| 2 | `60BCB743CA48` |
| 3 | `60ACB743CA48` |

## Payload Formatter

En la Application → **Payload formatters → Uplink**, pegar el contenido de:

`config/ttn-uplink-decoder.js`

## Verificación

Live data de la app o del device: uplinks cada ~60 s, FPort **2**, RSSI/SNR visibles.
