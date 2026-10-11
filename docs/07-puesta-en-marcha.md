# 07 — Puesta en marcha (checklist)

## A. Red LoRaWAN

- [ ] Gateway HT-M7603 online en TTN nam1
- [ ] Región US915, canales FSB2 (8–15)
- [ ] Application `heltec-v3-peru` creada
- [ ] 3 end devices registrados (Nodo 1/2/3)
- [ ] Decoder pegado desde `config/ttn-uplink-decoder.js`

## B. Hardware de un nodo

- [ ] Antena LoRa conectada
- [ ] Cableado según `docs/02-hardware-y-cableado.md`
- [ ] License Heltec correcta para el ChipID de esa placa
- [ ] Firmware de `firmware/nodo-X` con `REGION_US915`
- [ ] Serial: `joined` + lecturas DHT/MQ/BMP
- [ ] TTN Live data: FPort 2, payload sensores (no solo `BEAC…`)

## C. Supabase

- [ ] Ejecutar `config/supabase-schema.sql`
- [ ] Verificar filas en `nodes` (3 dispositivos)
- [ ] Desplegar Edge Function `ttn-uplink`
- [ ] Configurar webhook TTN → function
- [ ] Confirmar inserts en `sensor_readings` cada ~60 s

## D. Vue

- [ ] Variables `VITE_SUPABASE_URL` / `VITE_SUPABASE_ANON_KEY`
- [ ] Dashboard muestra los 7 campos de sensores
- [ ] Realtime o refresh ≤ 15 s
- [ ] Sin dependencia de CO₂ / PM2.5 / cámara

## E. Secretos locales

- [ ] Existe `.env` (copiado de `.env.example`) con AppKeys + licenses + Supabase
- [ ] Ejecutado `scripts/sync-secrets-from-env.ps1`
- [ ] Existen `firmware/nodo-*/secrets.h` locales
- [ ] `.env` y `secrets.h` **no** aparecen en `git status`

## F. GitHub

```bash
cd fireTame-heltec
git init
git add .
git status   # verificar que NO liste .env ni secrets.h
git commit -m "chore: initial fireTame-heltec project structure"
git branch -M main
git remote add origin https://github.com/<usuario>/fireTame-heltec.git
git push -u origin main
```

Sí se suben: `.env.example`, `secrets.example.h`.  
No se suben: `.env`, `secrets.h`.
