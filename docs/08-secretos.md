# 08 — Secretos y variables de entorno

## Archivos

| Archivo | Rol |
|---------|-----|
| `.env.example` | Plantilla pública |
| `.env` | Valores reales (gitignored) |
| `firmware/nodo-X/secrets.example.h` | Plantilla Arduino |
| `firmware/nodo-X/secrets.h` | Generado desde `.env` (gitignored) |

## Qué va en `.env`

- **TTN:** AppKeys OTAA, webhook secret
- **Heltec:** `NODO*_LICENSE_0..3` por ChipID
- **Supabase:** URL, anon key, service role
- **Vite (Vue):** `VITE_SUPABASE_URL`, `VITE_SUPABASE_ANON_KEY`

DevEUI / AppEUI pueden estar en docs públicas; **AppKey** y **license** no.

## Flujo

```text
.env  --(sync-secrets-from-env.ps1)-->  firmware/nodo-X/secrets.h
                                              ↑
                                         #include en nodo-X.ino
```

```powershell
powershell -ExecutionPolicy Bypass -File scripts/sync-secrets-from-env.ps1
```

## Vue / Edge Function

Copia las mismas variables Supabase al `.env` del frontend o a secrets de Supabase CLI:

- Front: solo `VITE_SUPABASE_URL` + `VITE_SUPABASE_ANON_KEY`
- Edge Function: `SUPABASE_SERVICE_ROLE_KEY` + `TTN_WEBHOOK_SECRET`
