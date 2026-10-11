# 02 — Hardware y cableado

## Material por nodo

- Heltec WiFi LoRa 32 V3 + antena LoRa
- DHT22 (temp / humedad)
- MQ135 (gas / humo relativo, ADC)
- GY-91 (BMP280 + MPU6500/9250)

## Pinout Heltec (USB hacia abajo)

### Alimentación

| Heltec | Sensores |
|--------|----------|
| **3V3** | DHT22 VCC, GY-91 **VIN** |
| **5V** | MQ135 VCC (calentador) |
| **GND** | GND de todos |

### Señales

| Sensor | Pin sensor | Heltec |
|--------|------------|--------|
| DHT22 | DATA | **GPIO4** |
| MQ135 | AO | **GPIO2** |
| MQ135 | DO | no conectar |
| GY-91 | SDA | **GPIO47** |
| GY-91 | SCL | **GPIO48** |

### GY-91 — pines que no se usan (modo I2C)

`SDO/SAO`, `NCS`, `CSB` → **sin cable**.

## Resumen

```text
3V3  → DHT22 + GY-91 VIN
5V   → MQ135
GND  → todos

GPIO4  → DHT DATA
GPIO2  → MQ AO
GPIO47 → GY-91 SDA
GPIO48 → GY-91 SCL
```

## Notas

- GPIO17/18 son del OLED interno: **no salen al header**.
- DHT22: si no trae pull-up, 4.7k–10k entre DATA y 3V3.
- MQ135 AO debe ser ≤ 3.3 V hacia el ESP32.
- Antena LoRa siempre conectada antes de transmitir.
