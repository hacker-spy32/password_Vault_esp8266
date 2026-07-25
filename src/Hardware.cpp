#include "Hardware.h"
#include "SettingsManager.h"

// Initialize global hardware objects
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS);

void initHardware() {
    DEBUG_LOG("Initializing hardware with pin-safe sequence...");
    
    // Pin-safe initialization: Set CS pins to OUTPUT and HIGH BEFORE SPI.begin()
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);
    
    DEBUG_LOG("CS pins configured (HIGH)");
    
    // Initialize backlight control pin (optional - may not be wired)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);  // Turn backlight ON initially
    DEBUG_LOG("Backlight pin configured (HIGH)");
    
    // Now initialize SPI bus
    SPI.begin();
    DEBUG_LOG("SPI initialized");
    
    // Initialize TFT display with default landscape orientation
    // Orientation will be updated from settings after SettingsManager loads
    tft.init();
    tft.setRotation(1); // Default landscape orientation (will be updated)
    DEBUG_LOG("TFT initialized");
    
    // Touch controller will be initialized by TouchManager
    Serial.println("Hardware initialization complete");
}

void applyDisplayOrientation(SettingsManager* settings) {
    if (settings == nullptr) {
        DEBUG_LOG("Hardware: Cannot apply orientation - settings is null");
        return;
    }
    
    uint8_t orientation = settings->getOrientation();
    tft.setRotation(orientation);
    DEBUG_LOGF("Hardware: Display orientation set to %d\n", orientation);
}
