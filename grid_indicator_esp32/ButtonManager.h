#pragma once
#include <Arduino.h>

namespace ButtonManager {
// Call once during startup, before checking bootPressedAtStartup().
void begin();
// True when BOOT is held LOW at startup.
bool bootPressedAtStartup();
// Returns true once per press after the button has been held for 5 seconds.
bool takeSetupRequest();
}
