/* Heltec WiFi LoRa 32 V3 — Nodo 1 LoRaWAN (TTN US915 FSB2)
 *
 * Sensors: DHT22, MQ135, GY-91 (BMP280 + MPU)
 * TTN: heltec-wifi-lora-32-v3 / ChipID E4B4B743CA48
 * Pins: DHT=GPIO4, MQ=GPIO2, GY-91 SDA=47 SCL=48
 * Tools: WiFi LoRa 32(V3), LORAWAN_REGION = REGION_US915
 */

#include "LoRaWan_APP.h"
#include <Wire.h>
#include <DHT.h>
#include <BMP280.h>
#include "HT_SSD1306Wire.h"

extern SSD1306Wire display;

#include "secrets.h"  // generado desde .env (gitignored)

uint16_t userChannelsMask[6] = { 0xFF00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t loraWanClass = CLASS_A;
uint32_t appTxDutyCycle = 60000;
bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = false;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;

static const int PIN_I2C_SDA = 47;
static const int PIN_I2C_SCL = 48;
static const int PIN_DHT = 4;
static const int PIN_MQ135 = 2;

#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);
BMP280 bmp(&Wire1);

static uint8_t mpuAddr = 0x68;
bool bmpOk = false;
bool mpuOk = false;
bool oledReady = false;

enum LinkUiState : uint8_t {
  UI_DESCONECTADO = 0,
  UI_CONECTANDO = 1,
  UI_CONECTADO = 2
};

struct SensorReading {
  float dhtTempC;
  float dhtHum;
  uint16_t mqAdc;
  float bmpTempC;
  float pressureHpa;
  float pitchDeg;
  float rollDeg;
  bool dhtOk;
};

static SensorReading lastReading = {};
static LinkUiState linkUi = UI_DESCONECTADO;
static bool hasSentData = false;

static const char *linkUiText(LinkUiState s) {
  switch (s) {
    case UI_CONECTANDO: return "Conectando";
    case UI_CONECTADO: return "Conectado";
    default: return "Desconectado";
  }
}

static bool networkJoined() {
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_NETWORK_JOINED;
  if (LoRaMacMibGetRequestConfirm(&mibReq) != LORAMAC_STATUS_OK) {
    return false;
  }
  return mibReq.Param.IsNetworkJoined == true;
}

static void refreshOled();

static void vextOn() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

static void putI16(uint8_t *buf, int16_t v) {
  buf[0] = (uint8_t)((v >> 8) & 0xFF);
  buf[1] = (uint8_t)(v & 0xFF);
}

static void putU16(uint8_t *buf, uint16_t v) {
  buf[0] = (uint8_t)((v >> 8) & 0xFF);
  buf[1] = (uint8_t)(v & 0xFF);
}

static bool mpuWrite(uint8_t reg, uint8_t val) {
  Wire1.beginTransmission(mpuAddr);
  Wire1.write(reg);
  Wire1.write(val);
  return Wire1.endTransmission() == 0;
}

static bool mpuReadBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire1.beginTransmission(mpuAddr);
  Wire1.write(reg);
  if (Wire1.endTransmission(false) != 0) {
    return false;
  }
  if (Wire1.requestFrom(mpuAddr, len) != len) {
    return false;
  }
  for (uint8_t i = 0; i < len; i++) {
    buf[i] = Wire1.read();
  }
  return true;
}

static bool mpuProbe(uint8_t addr) {
  mpuAddr = addr;
  uint8_t id = 0;
  if (!mpuReadBytes(0x75, &id, 1)) {
    return false;
  }
  if (id != 0x71 && id != 0x70 && id != 0x73 && id != 0x68) {
    Serial.printf("MPU WHO_AM_I=0x%02X @0x%02X (no reconocido)\n", id, addr);
    return false;
  }
  Serial.printf("MPU OK WHO_AM_I=0x%02X @0x%02X\n", id, addr);
  mpuWrite(0x6B, 0x00);
  delay(10);
  mpuWrite(0x1C, 0x00);
  return true;
}

static bool mpuBegin() {
  if (mpuProbe(0x68)) {
    return true;
  }
  return mpuProbe(0x69);
}

static bool mpuReadTilt(float &pitchDeg, float &rollDeg) {
  uint8_t raw[6];
  if (!mpuReadBytes(0x3B, raw, 6)) {
    return false;
  }
  int16_t ax = (int16_t)((raw[0] << 8) | raw[1]);
  int16_t ay = (int16_t)((raw[2] << 8) | raw[3]);
  int16_t az = (int16_t)((raw[4] << 8) | raw[5]);
  float x = ax / 16384.0f;
  float y = ay / 16384.0f;
  float z = az / 16384.0f;
  pitchDeg = atan2f(-x, sqrtf(y * y + z * z)) * 180.0f / PI;
  rollDeg = atan2f(y, z) * 180.0f / PI;
  return true;
}

static void initSensors() {
  vextOn();
  delay(300);

  display.init();
  oledReady = true;

  Wire1.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
  dht.begin();
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_MQ135, ADC_11db);

  bmpOk = bmp.begin(0x76);
  if (!bmpOk) {
    bmpOk = bmp.begin(0x77);
  }
  if (bmpOk) {
    bmp.setSampling(BMP280::MODE_NORMAL,
                    BMP280::SAMPLING_X2,
                    BMP280::SAMPLING_X16,
                    BMP280::FILTER_X16,
                    BMP280::STANDBY_MS_500);
    Serial.println("BMP280 OK (GY-91)");
  } else {
    Serial.println("BMP280 not found on Wire1 0x76/0x77");
  }

  mpuOk = mpuBegin();
  if (!mpuOk) {
    Serial.println("MPU not found on Wire1 0x68/0x69");
  }

  linkUi = UI_DESCONECTADO;
  hasSentData = false;
  refreshOled();
}

static SensorReading readSensors() {
  SensorReading r = {};
  r.dhtTempC = dht.readTemperature();
  r.dhtHum = dht.readHumidity();
  r.dhtOk = !(isnan(r.dhtTempC) || isnan(r.dhtHum));
  if (!r.dhtOk) {
    r.dhtTempC = 0;
    r.dhtHum = 0;
  }

  r.mqAdc = (uint16_t)analogRead(PIN_MQ135);

  if (bmpOk) {
    r.bmpTempC = bmp.readTemperature();
    r.pressureHpa = bmp.readPressure() / 100.0f;
  }

  if (mpuOk) {
    if (!mpuReadTilt(r.pitchDeg, r.rollDeg)) {
      r.pitchDeg = 0;
      r.rollDeg = 0;
    }
  }

  return r;
}

static void refreshOled() {
  if (!oledReady) {
    return;
  }
  if (networkJoined()) {
    linkUi = UI_CONECTADO;
  }

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, DEVICE_NAME);
  display.drawString(0, 12, String("Estado: ") + linkUiText(linkUi));

  if (!hasSentData) {
    display.drawString(0, 28, bmpOk ? "GY91 BMP OK" : "GY91 BMP FAIL");
    display.drawString(0, 42, mpuOk ? "GY91 MPU OK" : "GY91 MPU FAIL");
  } else {
    String line2;
    if (lastReading.dhtOk) {
      line2 = "T:" + String(lastReading.dhtTempC, 1) + "C H:" + String(lastReading.dhtHum, 0) + "%";
    } else {
      line2 = "DHT: error";
    }
    line2 += " MQ:" + String(lastReading.mqAdc);
    display.drawString(0, 28, line2);

    String line3 = bmpOk ? ("P:" + String(lastReading.pressureHpa, 0) + "hPa") : "P:n/a";
    if (mpuOk) {
      line3 += " Pi:" + String(lastReading.pitchDeg, 0) + " Ro:" + String(lastReading.rollDeg, 0);
    }
    display.drawString(0, 42, line3);
  }
  display.display();
}

static void prepareTxFrame(uint8_t port) {
  (void)port;
  lastReading = readSensors();
  hasSentData = true;
  linkUi = UI_CONECTADO;
  refreshOled();

  Serial.printf("[%s] DHT T=%.1f H=%.1f MQ=%u BMP T=%.1f P=%.1f Pitch=%.1f Roll=%.1f\n",
                DEVICE_NAME,
                lastReading.dhtTempC, lastReading.dhtHum, lastReading.mqAdc,
                lastReading.bmpTempC, lastReading.pressureHpa,
                lastReading.pitchDeg, lastReading.rollDeg);

  int16_t tDht = (int16_t)lroundf(lastReading.dhtTempC * 10.0f);
  uint16_t hum = (uint16_t)lroundf(constrain(lastReading.dhtHum, 0.0f, 100.0f) * 10.0f);
  int16_t tBmp = (int16_t)lroundf(lastReading.bmpTempC * 10.0f);
  uint16_t pHpa = (uint16_t)lroundf(constrain(lastReading.pressureHpa, 0.0f, 1200.0f) * 10.0f);
  int16_t pitch = (int16_t)lroundf(lastReading.pitchDeg * 10.0f);
  int16_t roll = (int16_t)lroundf(lastReading.rollDeg * 10.0f);

  appDataSize = 14;
  putI16(&appData[0], tDht);
  putU16(&appData[2], hum);
  putU16(&appData[4], lastReading.mqAdc);
  putI16(&appData[6], tBmp);
  putU16(&appData[8], pHpa);
  putI16(&appData[10], pitch);
  putI16(&appData[12], roll);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("%s (id=%u) sensores GY-91\n", DEVICE_NAME, NODE_ID);

  Mcu.setlicense(license, HELTEC_BOARD);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  initSensors();
}

void loop() {
  switch (deviceState) {
    case DEVICE_STATE_INIT: {
#if (LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass, loraWanRegion);
      LoRaWAN.setDefaultDR(3);
      linkUi = UI_DESCONECTADO;
      refreshOled();
      break;
    }
    case DEVICE_STATE_JOIN: {
      linkUi = UI_CONECTANDO;
      refreshOled();
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND: {
      prepareTxFrame(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE: {
      txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP: {
      LoRaWAN.sleep(loraWanClass);
      break;
    }
    default: {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
