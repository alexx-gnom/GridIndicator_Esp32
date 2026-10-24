#include <WiFi.h>

#include "config.h"
#include "Settings.h"
#include "Clock.h"
#include "PowerMonitor.h"
#include "History.h"
#include "Telegram.h"
#include "SetupPortal.h"
#include "WebServer.h"
#include "WifiManager.h"
#include "ButtonManager.h"
#include "LedManager.h"

SettingsManager settings;
SetupPortalManager setupPortal;
WebServerManager webServer;
DeviceConfig config;

enum class DeviceMode : uint8_t { STARTING, WORKING, SETUP };
DeviceMode mode = DeviceMode::STARTING;

constexpr uint32_t WIFI_RETRY_MS = 10000;
uint32_t lastWifiAttempt = 0;
bool wifiAttemptStarted = false;

void startSetupMode() {
  webServer.stop();
  WifiManager::stop();
  mode = DeviceMode::SETUP;
  LedManager::setState(LedManager::State::BLINK);

  if (!setupPortal.begin()) {
    LedManager::setState(LedManager::State::BLINK);
    return;
  }
}

void startWorkingMode() {
  setupPortal.stop();
  mode = DeviceMode::WORKING;
  LedManager::setState(LedManager::State::ON);
  WifiManager::begin(config.ssid, config.password);
  lastWifiAttempt = millis();
  wifiAttemptStarted = true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  LedManager::begin();
  ButtonManager::begin();

  if (!settings.begin()) {
    LedManager::setState(LedManager::State::ON);
    return;
  }

  if (ButtonManager::bootPressedAtStartup()) {
    startSetupMode();
    return;
  }

  if (!settings.loadConfig(config) || config.ssid.isEmpty()) {
    startSetupMode();
    return;
  }

  startWorkingMode();
  Clock::begin();
  PowerMonitor::begin();
  History::begin();
  Telegram::begin(config.objectName);
}

void loop() {
  LedManager::update();
  Clock::update();

  if (mode == DeviceMode::SETUP) {
    setupPortal.update();
    if (setupPortal.configurationSaved()) {
      delay(500);
      ESP.restart();
    }
    return;
  }

  if (mode != DeviceMode::WORKING) return;

  PowerMonitor::update();
  History::update(PowerMonitor::hasMainsPower());

  PowerMonitor::Event powerEvent;
  if (PowerMonitor::takeEvent(powerEvent)) {
    Telegram::onPowerEvent(
      powerEvent.type == PowerMonitor::EventType::POWER_RESTORED,
      powerEvent.timestamp
    );
  }
  Telegram::update(config.objectName);

  if (ButtonManager::takeSetupRequest()) {
    startSetupMode();
    return;
  }

  WifiManager::update();

  if (WifiManager::isConnected()) {
    if (!webServer.isRunning()) webServer.begin();
    webServer.update(config.objectName);
    LedManager::setState(LedManager::State::OFF);
    wifiAttemptStarted = false;
    return;
  }

  LedManager::setState(LedManager::State::ON);
  if (millis() - lastWifiAttempt >= WIFI_RETRY_MS) {
    lastWifiAttempt = millis();
    WifiManager::reconnect(config.ssid, config.password);
    wifiAttemptStarted = true;
  }
}
