
#pragma once

#include <Arduino.h>

class SetupPortalManager {
public:
  bool begin();
  void update();
  void stop();

  bool configurationSaved() const;

private:
  bool running = false;
  bool configSaved = false;
};
