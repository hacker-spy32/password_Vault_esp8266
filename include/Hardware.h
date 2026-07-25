#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "Config.h"

// Forward declaration
class SettingsManager;

// Global hardware objects
extern TFT_eSPI tft;
extern XPT2046_Touchscreen ts;

// Hardware initialization function with pin-safe sequence
void initHardware();

// Apply display orientation from settings
void applyDisplayOrientation(SettingsManager* settings);

#endif // HARDWARE_H
