# Firmware — fireTame-heltec

Sketches Arduino IDE para Heltec WiFi LoRa 32 V3.

| Carpeta | OLED | Device TTN |
|---------|------|------------|
| [nodo-1/](nodo-1/) | Nodo 1 | `heltec-wifi-lora-32-v3` |
| [nodo-2/](nodo-2/) | Nodo 2 | `heltec-nodo-2` |
| [nodo-3/](nodo-3/) | Nodo 3 | `heltec-nodo-3` |

Instrucciones: [docs/04-firmware.md](../docs/04-firmware.md)

**Credenciales:** no están en el `.ino`. Genera `secrets.h` desde la raíz:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/sync-secrets-from-env.ps1
```

**Tools → LORAWAN_REGION → REGION_US915** (obligatorio).
