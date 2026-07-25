#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>

// Screen orientation modes
enum ScreenOrientation {
    ORIENTATION_LANDSCAPE = 1,          // Standard landscape (90°)
    ORIENTATION_LANDSCAPE_INVERTED = 3  // Inverted landscape (270°)
};

// Settings data structure for persistence
struct SettingsData {
    uint32_t magic;              // Magic number for validation (0x53455454 = "SETT")
    uint8_t version;             // Settings format version
    
    // Auto-Lock / Screen Timeout
    uint8_t autoLockTimeout;     // In seconds (0 = Never, 5, 10, 15, 20, 30)
    
    // Two Tap Sleep
    bool twoTapSleepEnabled;     // Enable two-tap to sleep gesture
    
    // Screen Orientation
    uint8_t screenOrientation;   // 1 = landscape, 3 = inverted
    
    // Future expansion
    uint8_t reserved[32];        // Reserved for future settings
};

// SettingsManager - Handles persistent storage of settings and calibration data
class SettingsManager {
public:
    SettingsManager();
    
    // Initialize settings from storage
    void begin();
    
    // Reset touch calibration to factory defaults
    void resetCalibration();
    
    // Reset ALL settings and data to factory defaults
    void factoryReset();
    
    // Save current settings to persistent storage
    void save();
    
    // Load settings from persistent storage
    void load();
    
    // ===== Auto-Lock / Screen Timeout =====
    uint8_t getAutoLockTimeout() const { return settings.autoLockTimeout; }
    void setAutoLockTimeout(uint8_t seconds);
    bool isAutoLockEnabled() const { return settings.autoLockTimeout > 0; }
    
    // ===== Two Tap Sleep =====
    bool getTwoTapSleepEnabled() const { return settings.twoTapSleepEnabled; }
    void setTwoTapSleepEnabled(bool enabled);
    
    // ===== Screen Orientation =====
    uint8_t getOrientation() const { return settings.screenOrientation; }
    void setOrientation(uint8_t mode);
    
private:
    static const char* SETTINGS_FILE;
    static const uint32_t SETTINGS_MAGIC = 0x53455454; // "SETT"
    static const uint8_t SETTINGS_VERSION = 1;
    
    SettingsData settings;
    bool settingsLoaded;
    
    // Internal helpers
    void setDefaults();
    bool saveToFile();
    bool loadFromFile();
};

#endif // SETTINGSMANAGER_H
