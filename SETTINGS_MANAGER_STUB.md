# SettingsManager - Stub Implementation

## Current Status

A **stub/placeholder** implementation of SettingsManager has been created to allow the reset screens to compile. This is a minimal interface that needs to be expanded with actual functionality.

## Files Created

```
include/SettingsManager.h    - Interface definition
src/SettingsManager.cpp      - Stub implementation
```

## Current Interface

```cpp
class SettingsManager {
public:
    void begin();              // Initialize from storage
    void resetCalibration();   // Reset touch cal to defaults
    void factoryReset();       // Reset everything
    void save();               // Save to EEPROM/Flash
    void load();               // Load from EEPROM/Flash
};
```

## TODO: Implement Full Functionality

### 1. Add Settings Data Structure

```cpp
struct SettingsData {
    // Touch calibration
    int16_t touchCalX_min;
    int16_t touchCalX_max;
    int16_t touchCalY_min;
    int16_t touchCalY_max;
    
    // User preferences
    uint8_t screenBrightness;
    bool soundEnabled;
    uint16_t lockTimeout;
    
    // Version and validation
    uint32_t magicNumber;  // For detecting valid data
    uint16_t version;
    
    // Add more settings as needed
};
```

### 2. Implement EEPROM Storage

```cpp
#include <EEPROM.h>

#define SETTINGS_MAGIC 0xCAFEBABE
#define SETTINGS_VERSION 1
#define EEPROM_SIZE 512

void SettingsManager::save() {
    EEPROM.begin(EEPROM_SIZE);
    
    settingsData.magicNumber = SETTINGS_MAGIC;
    settingsData.version = SETTINGS_VERSION;
    
    EEPROM.put(0, settingsData);
    EEPROM.commit();
    EEPROM.end();
}

void SettingsManager::load() {
    EEPROM.begin(EEPROM_SIZE);
    
    SettingsData temp;
    EEPROM.get(0, temp);
    
    if (temp.magicNumber == SETTINGS_MAGIC && 
        temp.version == SETTINGS_VERSION) {
        settingsData = temp;
    } else {
        // No valid data, use defaults
        factoryReset();
    }
    
    EEPROM.end();
}
```

### 3. Implement resetCalibration()

```cpp
void SettingsManager::resetCalibration() {
    // Reset to factory defaults from Config.h
    settingsData.touchCalX_min = TOUCH_CAL_X_MIN;
    settingsData.touchCalX_max = TOUCH_CAL_X_MAX;
    settingsData.touchCalY_min = TOUCH_CAL_Y_MIN;
    settingsData.touchCalY_max = TOUCH_CAL_Y_MAX;
    
    DEBUG_LOG("Calibration reset to factory defaults");
}
```

### 4. Implement factoryReset()

```cpp
void SettingsManager::factoryReset() {
    // Reset ALL settings
    resetCalibration();
    
    // Reset preferences
    settingsData.screenBrightness = 255;
    settingsData.soundEnabled = true;
    settingsData.lockTimeout = 30000;
    
    // Clear passwords (TODO: implement password storage)
    // clearAllPasswords();
    
    DEBUG_LOG("Factory reset complete");
}
```

### 5. Add Getters/Setters

```cpp
// Touch calibration
int16_t getTouchCalXMin() const { return settingsData.touchCalX_min; }
int16_t getTouchCalXMax() const { return settingsData.touchCalX_max; }
int16_t getTouchCalYMin() const { return settingsData.touchCalY_min; }
int16_t getTouchCalYMax() const { return settingsData.touchCalY_max; }

void setTouchCalibration(int16_t xMin, int16_t xMax, int16_t yMin, int16_t yMax) {
    settingsData.touchCalX_min = xMin;
    settingsData.touchCalX_max = xMax;
    settingsData.touchCalY_min = yMin;
    settingsData.touchCalY_max = yMax;
}

// Preferences
uint8_t getBrightness() const { return settingsData.screenBrightness; }
void setBrightness(uint8_t value) { settingsData.screenBrightness = value; }

bool isSoundEnabled() const { return settingsData.soundEnabled; }
void setSoundEnabled(bool enabled) { settingsData.soundEnabled = enabled; }
```

## Integration with CalibrationWizardScreen

After calibration is complete, save the results:

```cpp
// In CalibrationWizardScreen::calculateCalibration()
void CalibrationWizardScreen::calculateCalibration() {
    DEBUG_LOG("Calculating calibration parameters...");
    
    // Calculate calibration from collected points
    int16_t xMin = calData.rawX[POINT_TOP_LEFT];
    int16_t xMax = calData.rawX[POINT_BOTTOM_RIGHT];
    int16_t yMin = calData.rawY[POINT_TOP_LEFT];
    int16_t yMax = calData.rawY[POINT_BOTTOM_RIGHT];
    
    // Apply to settings (needs SettingsManager reference)
    // settings->setTouchCalibration(xMin, xMax, yMin, yMax);
    // settings->save();
    
    DEBUG_LOG("Calibration calculation complete");
}
```

## Integration with TouchManager

Apply calibration values from settings:

```cpp
// In TouchManager constructor or begin()
void TouchManager::begin() {
    // Get calibration from settings
    int16_t xMin = settings->getTouchCalXMin();
    int16_t xMax = settings->getTouchCalXMax();
    int16_t yMin = settings->getTouchCalYMin();
    int16_t yMax = settings->getTouchCalYMax();
    
    // Apply to touch controller
    ts.setCalibration(xMin, xMax, yMin, yMax);
    
    DEBUG_LOG("Touch calibration applied");
}
```

## Usage Example

```cpp
// In main.cpp or ScreenManager
SettingsManager* settings = nullptr;

void setup() {
    // Create settings manager
    settings = new SettingsManager();
    settings->begin();  // Loads from EEPROM
    
    // Pass to screens that need it
    calibrationResetScreen = new CalibrationResetScreen(&tft, touchMgr, settings);
    factoryResetScreen = new FactoryResetScreen(&tft, touchMgr, settings);
}
```

## Storage Layout (Example)

```
EEPROM Address Map:
─────────────────────────────────
0-3:   Magic Number (0xCAFEBABE)
4-5:   Version (uint16_t)
6-7:   touchCalX_min
8-9:   touchCalX_max
10-11: touchCalY_min
12-13: touchCalY_max
14:    screenBrightness
15:    soundEnabled
16-17: lockTimeout
18-?:  Reserved for future use
─────────────────────────────────
Total: ~512 bytes allocated
```

## Testing Checklist

Once fully implemented:

- [ ] Settings save to EEPROM
- [ ] Settings load from EEPROM on startup
- [ ] Invalid/missing data triggers factory reset
- [ ] resetCalibration() restores touch defaults
- [ ] factoryReset() clears everything
- [ ] Calibration values applied to TouchManager
- [ ] Settings persist across reboots
- [ ] EEPROM wear leveling considered (if needed)

## Security Considerations

If storing passwords:

```cpp
// DO NOT store passwords in plain text
// Use encryption or hashing:
#include <Crypto.h>

void SettingsManager::storePassword(const char* password) {
    // Hash the password before storage
    uint8_t hash[32];
    SHA256.hash((uint8_t*)password, strlen(password), hash);
    
    // Store hash, not password
    memcpy(settingsData.passwordHash, hash, 32);
    save();
}

bool SettingsManager::verifyPassword(const char* password) {
    uint8_t hash[32];
    SHA256.hash((uint8_t*)password, strlen(password), hash);
    
    return memcmp(settingsData.passwordHash, hash, 32) == 0;
}
```

## Current Status Summary

✅ Header file created
✅ Stub implementation created
✅ Reset screens can compile
⏳ Actual storage not implemented
⏳ Data structure not defined
⏳ EEPROM operations not implemented
⏳ Integration with other screens needed

The stub allows the project to compile while you implement the actual functionality incrementally.
