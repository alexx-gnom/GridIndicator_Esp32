#pragma once

#include <Arduino.h>

class WebServerManager {
public:
  bool begin();
  void update(const String& objectName);
  void stop();
  bool isRunning() const;

private:
  bool running = false;
};
