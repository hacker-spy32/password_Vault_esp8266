#ifndef WEBUISETTINGSMANAGER_H
#define WEBUISETTINGSMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>

// Web UI Settings data structure for persistence
struct WebUISettings {
    uint32_t magic;              // Magic number for validation (0x57554953 = "WUIS")
    uint8_t version;             // Settings format version
    
    // WiFi Hotspot Configuration
    char ssid[33];               // WiFi SSID (max 32 chars + null terminator)
    char pairCode[7];            // 6-digit pairing code + null terminator
    char wifiPassword[64];       // WiFi password (8-63 chars + null terminator)
    
    // Auto-Start Configuration
    bool runInBackground;        // true = auto-start on boot, false = manual start only
    
    // Future expansion
    uint8_t reserved[64];        // Reserved for future settings (e.g., port, timeout, etc.)
};

/**
 * WebUISettingsManager - Manages Web UI configuration settings
 * 
 * Handles persistent storage of WiFi hotspot credentials, pairing code,
 * and auto-start behavior for the Web UI interface.
 */
class WebUISettingsManager {
public:
    WebUISettingsManager();
    
    /**
     * Initialize settings manager and load from storage
     */
    void begin();
    
    /**
     * Save current settings to persistent storage
     * @return true if save successful, false otherwise
     */
    bool save();
    
    /**
     * Load settings from persistent storage
     * @return true if load successful, false otherwise (defaults will be used)
     */
    bool load();
    
    /**
     * Reset settings to factory defaults
     */
    void setDefaults();
    
    // ===== Getters =====
    
    /**
     * Get the WiFi SSID (hotspot name)
     * @return SSID string
     */
    String getSSID() const { return String(settings.ssid); }
    
    /**
     * Get the 6-digit pairing code
     * @return Pair code string
     */
    String getPairCode() const { return String(settings.pairCode); }
    
    /**
     * Get the WiFi password
     * @return WiFi password string
     */
    String getWiFiPassword() const { return String(settings.wifiPassword); }
    
    /**
     * Check if Web UI should auto-start on boot
     * @return true if run in background enabled, false if manual start only
     */
    bool getRunInBackground() const { return settings.runInBackground; }
    
    // ===== Setters =====
    
    /**
     * Set the WiFi SSID (hotspot name)
     * @param ssid WiFi SSID (1-32 characters)
     * @return true if valid and set, false otherwise
     */
    bool setSSID(const String& ssid);
    
    /**
     * Set the 6-digit pairing code
     * @param code Pairing code (exactly 6 digits)
     * @return true if valid and set, false otherwise
     */
    bool setPairCode(const String& code);
    
    /**
     * Set the WiFi password
     * @param password WiFi password (8-63 characters)
     * @return true if valid and set, false otherwise
     */
    bool setWiFiPassword(const String& password);
    
    /**
     * Set auto-start behavior
     * @param enabled true = auto-start on boot, false = manual start only
     */
    void setRunInBackground(bool enabled);
    
    /**
     * Batch update all settings at once
     * @param ssid WiFi SSID
     * @param code Pairing code
     * @param password WiFi password
     * @param runInBg Run in background flag
     * @return true if all values valid and set, false otherwise
     */
    bool setAll(const String& ssid, const String& code, const String& password, bool runInBg);
    
private:
    static const char* SETTINGS_FILE;
    static const uint32_t SETTINGS_MAGIC = 0x57554953; // "WUIS"
    static const uint8_t SETTINGS_VERSION = 1;
    
    WebUISettings settings;
    bool settingsLoaded;
    
    // Internal helpers
    bool saveToFile();
    bool loadFromFile();
    bool validateSSID(const String& ssid) const;
    bool validatePairCode(const String& code) const;
    bool validatePassword(const String& password) const;
};

#endif // WEBUISETTINGSMANAGER_H
