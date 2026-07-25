#include "SettingsManager.h"
#include "Config.h"

const char* SettingsManager::SETTINGS_FILE = "/settings.dat";

SettingsManager::SettingsManager()
    : settingsLoaded(false) {
    setDefaults();
}

void SettingsManager::begin() {
    DEBUG_LOG("SettingsManager: Initializing...");
    
    // Ensure LittleFS is mounted (may already be mounted by VaultStorage)
    if (!LittleFS.begin()) {
        DEBUG_LOG("SettingsManager: LittleFS not mounted, attempting mount...");
        if (!LittleFS.begin()) {
            DEBUG_LOG("SettingsManager: LittleFS mount failed, using defaults");
            setDefaults();
            return;
        }
    }
    
    load();
    DEBUG_LOG("SettingsManager: Initialized");
}

void SettingsManager::setDefaults() {
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    settings.autoLockTimeout = 15;  // Default 15 seconds
    settings.twoTapSleepEnabled = false;  // Default disabled
    settings.screenOrientation = ORIENTATION_LANDSCAPE;
    memset(settings.reserved, 0, sizeof(settings.reserved));
    
    DEBUG_LOG("SettingsManager: Defaults set");
}

void SettingsManager::resetCalibration() {
    DEBUG_LOG("SettingsManager: Resetting calibration to factory defaults");
    
    // TODO: Reset touch calibration values to defaults
    // This will be implemented when touch calibration data is added to settings
    
    DEBUG_LOG("SettingsManager: Calibration reset complete");
}

void SettingsManager::factoryReset() {
    DEBUG_LOG("SettingsManager: Performing FACTORY RESET");
    
    // Reset to defaults
    setDefaults();
    
    // Save defaults
    save();
    
    // Reset calibration
    resetCalibration();
    
    DEBUG_LOG("SettingsManager: Factory reset complete");
}

void SettingsManager::save() {
    DEBUG_LOG("SettingsManager: Saving settings to persistent storage");
    
    if (saveToFile()) {
        DEBUG_LOG("SettingsManager: Settings saved successfully");
    } else {
        DEBUG_LOG("SettingsManager: Failed to save settings");
    }
}

void SettingsManager::load() {
    DEBUG_LOG("SettingsManager: Loading settings from persistent storage");
    
    if (loadFromFile()) {
        settingsLoaded = true;
        DEBUG_LOG("SettingsManager: Settings loaded successfully");
        DEBUG_LOGF("  Auto-Lock: %d seconds\n", settings.autoLockTimeout);
        DEBUG_LOGF("  Two Tap Sleep: %s\n", settings.twoTapSleepEnabled ? "ON" : "OFF");
        DEBUG_LOGF("  Orientation: %d\n", settings.screenOrientation);
    } else {
        DEBUG_LOG("SettingsManager: No settings file found, using defaults");
        setDefaults();
        // Save defaults for next time
        save();
    }
}

bool SettingsManager::saveToFile() {
    File file = LittleFS.open(SETTINGS_FILE, "w");
    if (!file) {
        Serial.println("[SETTINGS] Failed to open settings file for writing");
        return false;
    }
    
    size_t written = file.write((uint8_t*)&settings, sizeof(SettingsData));
    file.close();
    
    if (written != sizeof(SettingsData)) {
        Serial.println("[SETTINGS] Failed to write complete settings data");
        return false;
    }
    
    Serial.printf("[SETTINGS] Saved %d bytes to %s\n", written, SETTINGS_FILE);
    return true;
}

bool SettingsManager::loadFromFile() {
    if (!LittleFS.exists(SETTINGS_FILE)) {
        Serial.println("[SETTINGS] Settings file does not exist");
        return false;
    }
    
    File file = LittleFS.open(SETTINGS_FILE, "r");
    if (!file) {
        Serial.println("[SETTINGS] Failed to open settings file for reading");
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize != sizeof(SettingsData)) {
        Serial.printf("[SETTINGS] Settings file size mismatch: %d != %d\n", fileSize, sizeof(SettingsData));
        file.close();
        return false;
    }
    
    size_t read = file.read((uint8_t*)&settings, sizeof(SettingsData));
    file.close();
    
    if (read != sizeof(SettingsData)) {
        Serial.println("[SETTINGS] Failed to read complete settings data");
        return false;
    }
    
    // Validate magic number
    if (settings.magic != SETTINGS_MAGIC) {
        Serial.printf("[SETTINGS] Invalid magic number: 0x%08X\n", settings.magic);
        return false;
    }
    
    // Validate version
    if (settings.version != SETTINGS_VERSION) {
        Serial.printf("[SETTINGS] Unsupported settings version: %d\n", settings.version);
        return false;
    }
    
    Serial.printf("[SETTINGS] Loaded %d bytes from %s\n", read, SETTINGS_FILE);
    return true;
}

// ===== Auto-Lock / Screen Timeout =====

void SettingsManager::setAutoLockTimeout(uint8_t seconds) {
    // Validate timeout value (0, 5, 10, 15, 20, 30)
    if (seconds == 0 || seconds == 5 || seconds == 10 || 
        seconds == 15 || seconds == 20 || seconds == 30) {
        settings.autoLockTimeout = seconds;
        DEBUG_LOGF("SettingsManager: Auto-Lock timeout set to %d seconds\n", seconds);
        save();  // Auto-save
    } else {
        DEBUG_LOGF("SettingsManager: Invalid auto-lock timeout: %d\n", seconds);
    }
}

// ===== Two Tap Sleep =====

void SettingsManager::setTwoTapSleepEnabled(bool enabled) {
    settings.twoTapSleepEnabled = enabled;
    DEBUG_LOGF("SettingsManager: Two Tap Sleep %s\n", enabled ? "ENABLED" : "DISABLED");
    save();  // Auto-save
}

// ===== Screen Orientation =====

void SettingsManager::setOrientation(uint8_t mode) {
    // Validate orientation mode (only landscape modes supported)
    if (mode == ORIENTATION_LANDSCAPE || mode == ORIENTATION_LANDSCAPE_INVERTED) {
        settings.screenOrientation = mode;
        DEBUG_LOGF("SettingsManager: Orientation set to %d\n", mode);
        save();  // Auto-save
    } else {
        DEBUG_LOGF("SettingsManager: Invalid orientation %d, keeping current\n", mode);
    }
}
