# Genera firmware/nodo-X/secrets.h desde el archivo .env raíz
# Uso: powershell -ExecutionPolicy Bypass -File scripts/sync-secrets-from-env.ps1

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$envFile = Join-Path $root ".env"

if (-not (Test-Path $envFile)) {
  Write-Error "No existe .env. Copia .env.example a .env y rellena valores."
}

$vars = @{}
Get-Content $envFile | ForEach-Object {
  $line = $_.Trim()
  if ($line -eq "" -or $line.StartsWith("#")) { return }
  $i = $line.IndexOf("=")
  if ($i -lt 1) { return }
  $k = $line.Substring(0, $i).Trim()
  $v = $line.Substring($i + 1).Trim()
  $vars[$k] = $v
}

function HexPairs([string]$hex) {
  $h = $hex -replace "[^0-9A-Fa-f]", ""
  if ($h.Length % 2 -ne 0) { throw "Hex inválido: $hex" }
  $bytes = @()
  for ($i = 0; $i -lt $h.Length; $i += 2) {
    $bytes += ("0x{0}" -f $h.Substring($i, 2).ToUpper())
  }
  return ($bytes -join ", ")
}

function Write-Secrets($nodeNum, $name, $ttnId) {
  $prefix = "NODO$nodeNum"
  $deveui = $vars["${prefix}_DEVEUI"]
  $appeui = $vars["${prefix}_APPEUI"]
  $appkey = $vars["${prefix}_APPKEY"]
  if ([string]::IsNullOrWhiteSpace($deveui) -or [string]::IsNullOrWhiteSpace($appkey)) {
    throw "Faltan ${prefix}_DEVEUI o ${prefix}_APPKEY en .env"
  }

  $nwks = $vars["${prefix}_NWKSKEY"]
  $apps = $vars["${prefix}_APPSKEY"]
  $devaddr = $vars["${prefix}_DEVADDR"]
  if ([string]::IsNullOrWhiteSpace($nwks)) { $nwks = "00000000000000000000000000000000" }
  if ([string]::IsNullOrWhiteSpace($apps)) { $apps = "00000000000000000000000000000000" }
  if ([string]::IsNullOrWhiteSpace($devaddr)) { $devaddr = "00000000" }
  if ([string]::IsNullOrWhiteSpace($appeui)) { $appeui = "0000000000000000" }

  $lic0 = $vars["${prefix}_LICENSE_0"]
  $lic1 = $vars["${prefix}_LICENSE_1"]
  $lic2 = $vars["${prefix}_LICENSE_2"]
  $lic3 = $vars["${prefix}_LICENSE_3"]

  $dir = Join-Path $root "firmware\nodo-$nodeNum"
  $out = Join-Path $dir "secrets.h"

  $content = @"
#pragma once
/* AUTO-GENERADO desde .env - no subir a git */
/* TTN: $ttnId */

static const uint8_t NODE_ID = $nodeNum;
static const char *DEVICE_NAME = "$name";

uint8_t devEui[] = { $(HexPairs $deveui) };
uint8_t appEui[] = { $(HexPairs $appeui) };
uint8_t appKey[] = { $(HexPairs $appkey) };

uint8_t nwkSKey[] = { $(HexPairs $nwks) };
uint8_t appSKey[] = { $(HexPairs $apps) };
uint32_t devAddr = (uint32_t)0x$($devaddr.ToUpper());

uint32_t license[4] = { $lic0, $lic1, $lic2, $lic3 };
"@

  Set-Content -Path $out -Value $content -Encoding UTF8
  Write-Host "OK  $out"
}

Write-Secrets 1 "Nodo 1" "heltec-wifi-lora-32-v3"
Write-Secrets 2 "Nodo 2" "heltec-nodo-2"
Write-Secrets 3 "Nodo 3" "heltec-nodo-3"
Write-Host "`nListo. Compila/carga los sketches con secrets.h generado."
