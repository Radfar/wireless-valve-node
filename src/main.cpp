
/*
  ESP8266 capacitive soil moisture node — battery-powered, Zone 4 interim WiFi path
  Water Process project

  Power strategy:
  - Deep sleep between reads — D0 (GPIO16) MUST be wired to RST externally for the
    timed wake-up to work. This is a hardware jumper, not something code can do.
  - Sensor VCC switched via GPIO (not tied to 3V3) — sensor only draws current
    for the ~1s it's actually being read, extends sensor life too.
  - Fast WiFi reconnect using a cached channel + BSSID stored in RTC memory —
    the WiFi association handshake is the dominant power cost per wake, this
    cuts it from ~2-5s down to roughly 1s on most access points.
  - Plain MQTT (no TLS) to keep the connect+publish window short. Fine on a
    private LAN; add TLS before this is ever exposed beyond that.

  NOT included in this version: battery voltage monitoring. The ESP8266 has
  only one ADC pin (A0) and it's already committed to the soil sensor reading.
  Monitoring battery voltage too would need an analog mux or a second board —
  left as a known gap, not solved here.
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ---- User config — fill in before flashing ----
const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
const char* MQTT_HOST     = "192.168.x.x";
const uint16_t MQTT_PORT  = 1883;
const char* MQTT_TOPIC    = "zone4/soil01/data";
const char* NODE_ID       = "soil01";

// ---- Pins ----
const uint8_t SENSOR_POWER_PIN = D5;   // GPIO14 — switches sensor VCC on/off
const uint8_t SENSOR_ADC_PIN   = A0;   // only analog pin on the ESP8266

// ---- Calibration — replace with YOUR sensor's own dry-air / water-immersion readings ----
const int ADC_DRY = 780;   // raw analogRead() in open dry air
const int ADC_WET = 360;   // raw analogRead() fully submerged in water

// ---- Sleep interval — trade battery life for data freshness here ----
const uint64_t SLEEP_SECONDS = 600; // 10 minutes

// ---- RTC memory block for fast WiFi reconnect across deep-sleep cycles ----
struct RtcData {
  uint32_t magic;
  uint8_t channel;
  uint8_t bssid[6];
};
RtcData rtcData;

WiFiClient espClient;
PubSubClient mqtt(espClient);

float readMoisturePercent() {
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(50); // let the sensor's output settle after power-up

  long total = 0;
  const int SAMPLES = 8;
  for (int i = 0; i < SAMPLES; i++) {
    total += analogRead(SENSOR_ADC_PIN);
    delay(10);
  }
  int raw = total / SAMPLES;

  digitalWrite(SENSOR_POWER_PIN, LOW); // cut sensor power the moment we're done reading

  float pct = (float)(ADC_DRY - raw) / (float)(ADC_DRY - ADC_WET) * 100.0;
  if (pct < 0)   pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}

bool connectWiFiFast() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false); // avoid wearing out flash with repeated config writes

  ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
  bool haveCache = (rtcData.magic == 0xA5A5A5A5);

  if (haveCache) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, rtcData.channel, rtcData.bssid, true);
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    rtcData.magic = 0xA5A5A5A5;
    rtcData.channel = WiFi.channel();
    memcpy(rtcData.bssid, WiFi.BSSID(), 6);
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
    return true;
  }
  return false; // no cache saved on failure — next boot falls back to a full scan
}

void goToSleep() {
  WiFi.disconnect(true);
  delay(10);
  ESP.deepSleep(SLEEP_SECONDS * 1000000ULL); // deepSleep takes microseconds
}

void setup() {
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, LOW);

  float moisture = readMoisturePercent();

  if (connectWiFiFast()) {
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    if (mqtt.connect(NODE_ID)) {
      char payload[64];
      snprintf(payload, sizeof(payload), "{\"moisture\":%.1f}", moisture);
      mqtt.publish(MQTT_TOPIC, payload);
      mqtt.disconnect();
    }
  }
  // If WiFi or MQTT connect failed, we just skip publishing this cycle and
  // sleep anyway — don't burn battery retrying every cycle.

  goToSleep();
}

void loop() {
  // never reached — deep sleep resets the chip, execution restarts at setup()
}