#include "WebUISettingsManager.h"
#include "Config.h"

const char* WebUISettingsManager::SETTINGS_FILE = "/webui_settings.dat";

WebUISettingsManager::WebUISettingsManager()
    : settingsLoaded(false) {
    setDefaults();
}

void WebUISettingsManager::begin() {
    DEBUG_LOG("WebUISettingsManager: Initializing...");
    
    // Ensure LittleFS is mounted
    if (!LittleFS.begin()) {
        DEBUG_LOG("WebUISettingsManager: LittleFS not mounted, attempting mount...");
        if (!LittleFS.begin()) {
            DEBUG_LOG("WebUISettingsManager: LittleFS mount failed, using defaults");
            setDefaults();
            return;
        }
    }
    
    if (!load()) {
        DEBUG_LOG("WebUISettingsManager: No saved settings, using defaults");
        setDefaults();
        // Save defaults for next time
        save();
    }
    
    DEBUG_LOG("WebUISettingsManager: Initialized");
    DEBUG_LOGF("  SSID: %s\n", settings.ssid);
    DEBUG_LOGF("  Pair Code: %s\n", settings.pairCode);
    DEBUG_LOGF("  Run in Background: %s\n", settings.runInBackground ? "YES" : "NO");
}

void WebUISettingsManager::setDefaults() {
    settings.magic = SETTINGS_MAGIC;
    settings.version = SETTINGS_VERSION;
    
    // Default WiFi credentials
    strncpy(settings.ssid, "SecureKey", sizeof(settings.ssid) - 1);
    settings.ssid[sizeof(settings.ssid) - 1] = '\0';
    
    strncpy(settings.pairCode, "123456", sizeof(settings.pairCode) - 1);
    settings.pairCode[sizeof(settings.pairCode) - 1] = '\0';
    
    strncpy(settings.wifiPassword, "securekey", sizeof(settings.wifiPassword) - 1);
    settings.wifiPassword[sizeof(settings.wifiPassword) - 1] = '\0';
    
    // Default: manual start only (NOT auto-start on boot)
    settings.runInBackground = false;
    
    // Clear reserved space
    memset(settings.reserved, 0, sizeof(settings.reserved));
    
    DEBUG_LOG("WebUISettingsManager: Defaults set");
}

bool WebUISettingsManager::save() {
    DEBUG_LOG("WebUISettingsManager: Saving settings to persistent storage");
    
    if (saveToFile()) {
        DEBUG_LOG("WebUISettingsManager: Settings saved successfully");
        return true;
    } else {
        DEBUG_LOG("WebUISettingsManager: Failed to save settings");
        return false;
    }
}

bool WebUISettingsManager::load() {
    DEBUG_LOG("WebUISettingsManager: Loading settings from persistent storage");
    
    if (loadFromFile()) {
        settingsLoaded = true;
        DEBUG_LOG("WebUISettingsManager: Settings loaded successfully");
        return true;
    } else {
        DEBUG_LOG("WebUISettingsManager: Failed to load settings");
        return false;
    }
}

bool WebUISettingsManager::saveToFile() {
    File file = LittleFS.open(SETTINGS_FILE, "w");
    if (!file) {
        Serial.println("[WEBUI_SETTINGS] Failed to open settings file for writing");
        return false;
    }
    
    size_t written = file.write((uint8_t*)&settings, sizeof(WebUISettings));
    file.close();
    
    if (written != sizeof(WebUISettings)) {
        Serial.println("[WEBUI_SETTINGS] Failed to write complete settings data");
        return false;
    }
    
    Serial.printf("[WEBUI_SETTINGS] Saved %d bytes to %s\n", written, SETTINGS_FILE);
    return true;
}

bool WebUISettingsManager::loadFromFile() {
    if (!LittleFS.exists(SETTINGS_FILE)) {
        Serial.println("[WEBUI_SETTINGS] Settings file does not exist");
        return false;
    }
    
    File file = LittleFS.open(SETTINGS_FILE, "r");
    if (!file) {
        Serial.println("[WEBUI_SETTINGS] Failed to open settings file for reading");
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize != sizeof(WebUISettings)) {
        Serial.printf("[WEBUI_SETTINGS] Settings file size mismatch: %d != %d\n", fileSize, sizeof(WebUISettings));
        file.close();
        return false;
    }
    
    size_t read = file.read((uint8_t*)&settings, sizeof(WebUISettings));
    file.close();
    
    if (read != sizeof(WebUISettings)) {
        Serial.println("[WEBUI_SETTINGS] Failed to read complete settings data");
        return false;
    }
    
    // Validate magic number
    if (settings.magic != SETTINGS_MAGIC) {
        Serial.printf("[WEBUI_SETTINGS] Invalid magic number: 0x%08X\n", settings.magic);
        return false;
    }
    
    // Validate version
    if (settings.version != SETTINGS_VERSION) {
        Serial.printf("[WEBUI_SETTINGS] Unsupported settings version: %d\n", settings.version);
        return false;
    }
    
    Serial.printf("[WEBUI_SETTINGS] Loaded %d bytes from %s\n", read, SETTINGS_FILE);
    return true;
}

// ===== Validation Helpers =====

bool WebUISettingsManager::validateSSID(const String& ssid) const {
    // SSID must be 1-32 characters
    return ssid.length() >= 1 && ssid.length() <= 32;
}

bool WebUISettingsManager::validatePairCode(const String& code) const {
    // Code must be exactly 6 digits
    if (code.length() != 6) return false;
    
    for (char c : code) {
        if (!isdigit(c)) return false;
    }
    
    return true;
}

bool WebUISettingsManager::validatePassword(const String& password) const {
    // Password must be 8-63 characters
    return password.length() >= 8 && password.length() <= 63;
}

// ===== Setters with Validation =====

bool WebUISettingsManager::setSSID(const String& ssid) {
    if (!validateSSID(ssid)) {
        DEBUG_LOGF("WebUISettingsManager: Invalid SSID: '%s'\n", ssid.c_str());
        return false;
    }
    
    strncpy(settings.ssid, ssid.c_str(), sizeof(settings.ssid) - 1);
    settings.ssid[sizeof(settings.ssid) - 1] = '\0';
    
    DEBUG_LOGF("WebUISettingsManager: SSID set to '%s'\n", settings.ssid);
    return true;
}

bool WebUISettingsManager::setPairCode(const String& code) {
    if (!validatePairCode(code)) {
        DEBUG_LOGF("WebUISettingsManager: Invalid pair code: '%s'\n", code.c_str());
        return false;
    }
    
    strncpy(settings.pairCode, code.c_str(), sizeof(settings.pairCode) - 1);
    settings.pairCode[sizeof(settings.pairCode) - 1] = '\0';
    
    DEBUG_LOGF("WebUISettingsManager: Pair code set to '%s'\n", settings.pairCode);
    return true;
}

bool WebUISettingsManager::setWiFiPassword(const String& password) {
    if (!validatePassword(password)) {
        DEBUG_LOGF("WebUISettingsManager: Invalid WiFi password (length: %d)\n", password.length());
        return false;
    }
    
    strncpy(settings.wifiPassword, password.c_str(), sizeof(settings.wifiPassword) - 1);
    settings.wifiPassword[sizeof(settings.wifiPassword) - 1] = '\0';
    
    DEBUG_LOG("WebUISettingsManager: WiFi password updated");
    return true;
}

void WebUISettingsManager::setRunInBackground(bool enabled) {
    settings.runInBackground = enabled;
    DEBUG_LOGF("WebUISettingsManager: Run in background set to %s\n", enabled ? "YES" : "NO");
}

bool WebUISettingsManager::setAll(const String& ssid, const String& code, const String& password, bool runInBg) {
    // Validate all inputs first
    if (!validateSSID(ssid)) {
        DEBUG_LOG("WebUISettingsManager: Batch update failed - invalid SSID");
        return false;
    }
    
    if (!validatePairCode(code)) {
        DEBUG_LOG("WebUISettingsManager: Batch update failed - invalid pair code");
        return false;
    }
    
    if (!validatePassword(password)) {
        DEBUG_LOG("WebUISettingsManager: Batch update failed - invalid password");
        return false;
    }
    
    // All valid - update all at once
    setSSID(ssid);
    setPairCode(code);
    setWiFiPassword(password);
    setRunInBackground(runInBg);
    
    DEBUG_LOG("WebUISettingsManager: Batch update successful");
    return true;
}
