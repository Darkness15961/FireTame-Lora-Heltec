/**
 * fireTame-heltec — TTN Application Payload Formatter (Uplink)
 * Pegar en: Application → Payload formatters → Uplink
 */
function decodeUplink(input) {
  var b = input.bytes;

  // Beacon legacy: BE AC nodeId counter
  if (b.length >= 4 && b[0] === 0xBE && b[1] === 0xAC) {
    return {
      data: {
        mode: "beacon",
        node_id: b[2],
        counter: b[3],
        schema_version: 0
      }
    };
  }

  if (b.length < 14) {
    return { errors: ["payload too short"] };
  }

  function i16(i) {
    var v = (b[i] << 8) | b[i + 1];
    if (v & 0x8000) v = v - 0x10000;
    return v;
  }

  function u16(i) {
    return (b[i] << 8) | b[i + 1];
  }

  return {
    data: {
      mode: "sensors",
      temp_dht_c: i16(0) / 10,
      humidity_pct: u16(2) / 10,
      mq135_adc: u16(4),
      temp_bmp_c: i16(6) / 10,
      pressure_hpa: u16(8) / 10,
      pitch_deg: i16(10) / 10,
      roll_deg: i16(12) / 10,
      interval_s: 60,
      source: "heltec_v3_gy91",
      schema_version: 1
    }
  };
}
