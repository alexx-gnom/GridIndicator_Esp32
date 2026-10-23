
#include "Settings.h"

namespace {
  constexpr char NVS_NAMESPACE[] = "grid_settings";
  constexpr char KEY_WIFI_SSID[] = "wifi_ssid";
  constexpr char KEY_WIFI_PASSWORD[] = "wifi_pass";
  constexpr char KEY_OBJECT_NAME[] = "object_name";
}

bool SettingsManager::begin() {
  return preferences.begin(NVS_NAMESPACE, false);
}

bool SettingsManager::hasWifiConfig() {
  return preferences.isKey(KEY_WIFI_SSID) &&
         preferences.isKey(KEY_WIFI_PASSWORD) &&
         preferences.getString(KEY_WIFI_SSID, "").length() > 0;
}

bool SettingsManager::loadConfig(DeviceConfig& config) {
  if (!hasWifiConfig()) {
    config = {};
    return false;
  }

  config.ssid = preferences.getString(KEY_WIFI_SSID, "");
  config.password = preferences.getString(KEY_WIFI_PASSWORD, "");
  config.objectName = preferences.getString(KEY_OBJECT_NAME, "");

  return config.ssid.length() > 0;
}

bool SettingsManager::saveConfig(const DeviceConfig& config) {
  if (config.ssid.length() == 0 ||
      config.objectName.length() == 0) {
    return false;
  }

  bool ok = true;

  ok &= preferences.putString(KEY_WIFI_SSID, config.ssid)
        == config.ssid.length();

  ok &= preferences.putString(KEY_WIFI_PASSWORD, config.password)
        == config.password.length();

  ok &= preferences.putString(KEY_OBJECT_NAME, config.objectName)
        == config.objectName.length();

  return ok;
}

bool SettingsManager::clearWifiConfig() {
  preferences.remove(KEY_WIFI_SSID);
  preferences.remove(KEY_WIFI_PASSWORD);
  preferences.remove(KEY_OBJECT_NAME);

  return !preferences.isKey(KEY_WIFI_SSID) &&
         !preferences.isKey(KEY_WIFI_PASSWORD) &&
         !preferences.isKey(KEY_OBJECT_NAME);
}
