
#include <WiFi.h>
#include <time.h>

const char* WIFI_SSID = "приховано";
const char* WIFI_PASSWORD = "приховано";

const char* TZ_INFO = "EET-2EEST,M3.5.0/3,M10.5.0/4";

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Connected to access point");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.print("Disconnected. Reason code: ");
      Serial.println(info.wifi_sta_disconnected.reason);
      break;

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  delay(1000);

  WiFi.mode(WIFI_STA);
  Serial.print("ESP32 MAC: ");
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();
  delay(500);

  Serial.println("\n=== Wi-Fi scan ===");

  int count = WiFi.scanNetworks();

  if (count == 0) {
    Serial.println("No networks found!");
  } else {
    Serial.printf("Found %d networks:\n", count);

    for (int i = 0; i < count; i++) {
      Serial.printf(
  "%d: %s | RSSI: %d dBm | CH: %d | BSSID: %s\n",
  i + 1,
  WiFi.SSID(i).c_str(),
  WiFi.RSSI(i),
  WiFi.channel(i),
  WiFi.BSSIDstr(i).c_str()
);
    }
  }

  WiFi.scanDelete();

  Serial.println("\n=== Connecting ===");
  uint8_t bssid[] = {0x50, 0xD4, 0xF7, 0x3E, 0x9E, 0x7A};

WiFi.begin(
  WIFI_SSID,
  WIFI_PASSWORD//,
 // 1,       // канал роутера
  //bssid,   // конкретна точка доступу
  //true
);

  for (int i = 0; i < 30; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }

    Serial.printf("Status: %d\n", WiFi.status());
    delay(1000);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nCONNECTED!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("\nConnection failed.");
    Serial.printf("Final status: %d\n", WiFi.status());
  }

   configTzTime(
    TZ_INFO,
    "pool.ntp.org",
    "time.google.com"
  );

  Serial.println("Waiting for NTP time...");

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo, 20000)) {
    Serial.println("NTP synchronization failed!");
    return;
  }

  Serial.println("NTP synchronized!");
}

void loop() {
  struct tm timeInfo;

  if (getLocalTime(&timeInfo, 1000)) {
    Serial.printf(
      "%04d-%02d-%02d %02d:%02d:%02d\n",
      timeInfo.tm_year + 1900,
      timeInfo.tm_mon + 1,
      timeInfo.tm_mday,
      timeInfo.tm_hour,
      timeInfo.tm_min,
      timeInfo.tm_sec
    );
  } else {
    Serial.println("Time unavailable");
  }

  delay(1000);
}
