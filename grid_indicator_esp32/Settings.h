
#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct DeviceConfig {
  String ssid;
  String password;
  String objectName;
};

class SettingsManager {
public:
  bool begin();
  bool hasWifiConfig();
  bool loadConfig(DeviceConfig& config);
  bool saveConfig(const DeviceConfig& config);
  bool clearWifiConfig();

private:
  Preferences preferences;
};
