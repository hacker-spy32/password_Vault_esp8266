# Web UI Settings Integration Guide

## Overview
This guide shows how to integrate the new Web UI Settings feature into your SecureKey firmware.

## Files Created/Modified

### New Files
1. **include/WebUISettingsManager.h** - Settings manager header
2. **src/WebUISettingsManager.cpp** - Settings manager implementation
3. **portal_html.h** - Updated with Settings page (preserves all existing functionality)

### Files to Modify
1. **main.ino** (or main.cpp) - Add boot logic and web server endpoints
2. **platformio.ini** - Ensure ESP8266WiFi and ESP8266WebServer libraries are included

## Step 1: Include Headers

Add to your main.ino or main.cpp:

```cpp
#include "WebUISettingsManager.h"
#include "portal_html.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
```

## Step 2: Create Global Instances

```cpp
// Web UI components
WebUISettingsManager webUISettings;
ESP8266WebServer* webServer = nullptr;  // Pointer so we can create/destroy as needed
bool webUIActive = false;
```

## Step 3: Add REST Endpoint Handlers

```cpp
// ===== WEB UI SETTINGS ENDPOINTS (NEW) =====

/**
 * GET /webui/settings
 * Returns current Web UI settings as JSON
 */
void handleGetWebUISettings() {
    // Verify pair code authorization
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    // Build JSON response
    String json = "{";
    json += "\"ssid\":\"" + webUISettings.getSSID() + "\",";
    json += "\"pairCode\":\"" + webUISettings.getPairCode() + "\",";
    json += "\"wifiPassword\":\"" + webUISettings.getWiFiPassword() + "\",";
    json += "\"runInBackground\":" + String(webUISettings.getRunInBackground() ? "true" : "false");
    json += "}";
    
    webServer->send(200, "application/json", json);
    Serial.println("[WEBUI_API] GET /webui/settings - OK");
}

/**
 * POST /webui/settings
 * Saves new Web UI settings
 * 
 * Expected POST parameters:
 * - code: Current pair code (for authorization)
 * - ssid: New WiFi SSID (1-32 chars)
 * - pairCode: New pair code (6 digits)
 * - wifiPassword: New WiFi password (8-63 chars)
 * - runInBackground: "1" or "0"
 */
void handleSaveWebUISettings() {
    // Verify pair code authorization (use CURRENT code, not the new one)
    String currentCode = webServer->arg("code");
    if (currentCode != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    // Extract parameters
    String ssid = webServer->arg("ssid");
    String newCode = webServer->arg("pairCode");
    String password = webServer->arg("wifiPassword");
    String bgStr = webServer->arg("runInBackground");
    bool runInBg = (bgStr == "1" || bgStr == "true");
    
    // Validate and save
    if (webUISettings.setAll(ssid, newCode, password, runInBg)) {
        if (webUISettings.save()) {
            webServer->send(200, "text/plain", "OK");
            Serial.println("[WEBUI_API] POST /webui/settings - Settings saved");
            
            // Note: WiFi changes will take effect after restart
            Serial.println("[WEBUI_API] Restart device to apply WiFi changes");
        } else {
            webServer->send(500, "text/plain", "Save failed");
            Serial.println("[WEBUI_API] POST /webui/settings - Save failed");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid parameters");
        Serial.println("[WEBUI_API] POST /webui/settings - Validation failed");
    }
}
```

## Step 4: Web UI Start/Stop Functions

```cpp
/**
 * Start the Web UI (WiFi hotspot + web server)
 * Uses settings from WebUISettingsManager
 */
void startWebUI() {
    if (webUIActive) {
        Serial.println("[WEBUI] Already running");
        return;
    }
    
    Serial.println("[WEBUI] Starting Web UI...");
    
    // Get settings
    String ssid = webUISettings.getSSID();
    String password = webUISettings.getWiFiPassword();
    String pairCode = webUISettings.getPairCode();
    
    Serial.printf("[WEBUI] SSID: %s\n", ssid.c_str());
    Serial.printf("[WEBUI] Pair Code: %s\n", pairCode.c_str());
    
    // Start SoftAP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), password.c_str());
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("[WEBUI] Access Point started\n");
    Serial.printf("[WEBUI] IP: %s\n", IP.toString().c_str());
    
    // Create and configure web server
    webServer = new ESP8266WebServer(80);
    
    // Serve main portal HTML
    webServer->on("/", HTTP_GET, []() {
        webServer->send_P(200, "text/html", PORTAL_HTML);
    });
    
    // ===== EXISTING ENDPOINTS (PRESERVE ALL) =====
    // Add your existing /list, /save, /edit, /delete, /export endpoints here
    // Example:
    // webServer->on("/list", HTTP_GET, handleList);
    // webServer->on("/save", HTTP_POST, handleSave);
    // webServer->on("/edit", HTTP_POST, handleEdit);
    // webServer->on("/delete", HTTP_POST, handleDelete);
    // webServer->on("/export", HTTP_GET, handleExport);
    
    // ===== NEW WEB UI SETTINGS ENDPOINTS =====
    webServer->on("/webui/settings", HTTP_GET, handleGetWebUISettings);
    webServer->on("/webui/settings", HTTP_POST, handleSaveWebUISettings);
    
    // Start server
    webServer->begin();
    webUIActive = true;
    
    Serial.println("[WEBUI] Web UI started successfully");
    Serial.println("[WEBUI] Connect to WiFi and navigate to http://192.168.4.1");
}

/**
 * Stop the Web UI (turn off WiFi hotspot and web server)
 */
void stopWebUI() {
    if (!webUIActive) {
        Serial.println("[WEBUI] Already stopped");
        return;
    }
    
    Serial.println("[WEBUI] Stopping Web UI...");
    
    // Stop web server
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
    }
    
    // Stop WiFi
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    
    webUIActive = false;
    Serial.println("[WEBUI] Web UI stopped");
}
```

## Step 5: Boot Logic in setup()

```cpp
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("SecureKey Starting...");
    Serial.println("========================================");
    
    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("[FATAL] LittleFS mount failed");
        return;
    }
    
    // Initialize Web UI Settings Manager
    webUISettings.begin();
    
    // Initialize other managers (PIN, Vault, Settings, etc.)
    // ...
    
    // ===== WEB UI AUTO-START LOGIC =====
    if (webUISettings.getRunInBackground()) {
        Serial.println("[BOOT] Run in Background is ON - starting Web UI automatically");
        startWebUI();
    } else {
        Serial.println("[BOOT] Run in Background is OFF - Web UI not started");
        Serial.println("[BOOT] Start Web UI manually from device TFT interface");
    }
    
    Serial.println("[BOOT] Setup complete");
}
```

## Step 6: Main Loop

```cpp
void loop() {
    // Handle web server requests if active
    if (webUIActive && webServer) {
        webServer->handleClient();
    }
    
    // Your existing main loop code
    // ...
}
```

## Step 7: TFT Interface Integration

Add a "Start Web UI" button to your TFT interface. When pressed:

```cpp
void onStartWebUIButtonPressed() {
    if (!webUIActive) {
        Serial.println("[TFT] User requested Web UI start");
        startWebUI();
        
        // Show confirmation on TFT
        // Display WiFi SSID, password, and IP address
        String message = "Web UI Started\n";
        message += "SSID: " + webUISettings.getSSID() + "\n";
        message += "Pass: " + webUISettings.getWiFiPassword() + "\n";
        message += "IP: " + WiFi.softAPIP().toString();
        
        // Show message on your TFT screen
        // ...
    } else {
        Serial.println("[TFT] Web UI already running");
    }
}

void onStopWebUIButtonPressed() {
    if (webUIActive) {
        Serial.println("[TFT] User requested Web UI stop");
        stopWebUI();
        
        // Show confirmation on TFT
        // ...
    }
}
```

## Step 8: Update platformio.ini

Ensure you have the required libraries:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps = 
    bodmer/TFT_eSPI@^2.5.43
    PaulStoffregen/XPT2046_Touchscreen
    ESP8266WiFi
    ESP8266WebServer
```

## Usage Flow

### Scenario 1: Manual Start (Default)
1. Device boots with `runInBackground = false`
2. User navigates to Settings on TFT
3. User presses "Start Web UI" button
4. WiFi hotspot starts using saved credentials
5. User connects phone/laptop to WiFi
6. User opens browser to 192.168.4.1
7. User enters pair code shown on device
8. User manages passwords via web interface
9. When done, user presses "Stop Web UI" on TFT

### Scenario 2: Auto-Start Background Mode
1. User goes to Web UI Settings page (via web interface)
2. User enables "Run in Background" toggle
3. User saves settings
4. User restarts device
5. WiFi hotspot starts automatically on boot
6. Web UI is always available (until manually stopped)

### Scenario 3: Changing Credentials
1. User connects to Web UI
2. User opens Settings page
3. User changes SSID, pair code, and/or password
4. User saves
5. User restarts device
6. New credentials take effect

## Security Notes

1. **Pair Code Authorization**: All Web UI API endpoints require the pair code
2. **WiFi Password**: Minimum 8 characters enforced
3. **No Plaintext Storage**: Settings are stored in binary format on LittleFS
4. **Manual Stop**: Even in background mode, user can stop Web UI from TFT
5. **Validation**: All user inputs are validated before saving

## Future Enhancements (Preparation)

The code is designed to easily add these features later:

### Random Generation Buttons

```cpp
// Generate random 6-digit code
String generateRandomCode() {
    String code = "";
    for (int i = 0; i < 6; i++) {
        code += String(random(0, 10));
    }
    return code;
}

// Generate random secure password
String generateRandomPassword(int length = 16) {
    const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz23456789!@#$%&*?";
    String password = "";
    for (int i = 0; i < length; i++) {
        password += chars[random(0, strlen(chars))];
    }
    return password;
}

// Add endpoints for these:
// GET /webui/generate/code
// GET /webui/generate/password
```

### SSID Suffix Randomization

```cpp
// Generate SSID with random suffix
String generateSSIDWithSuffix(const String& base) {
    return base + "-" + String(random(1000, 9999));
}
```

Frontend can easily add "Generate" buttons next to each field that call these endpoints.

## Debugging

Enable debug output:

```cpp
#define DEBUG 1

#if DEBUG
  #define DEBUG_LOG(x) Serial.println(x)
  #define DEBUG_LOGF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_LOG(x)
  #define DEBUG_LOGF(x, ...)
#endif
```

Monitor serial output at 115200 baud to see Web UI lifecycle events.

## Testing Checklist

- [ ] Settings load on boot with defaults
- [ ] Settings save successfully to LittleFS
- [ ] WiFi hotspot starts with correct SSID/password
- [ ] Web interface loads at 192.168.4.1
- [ ] Settings page opens and loads current values
- [ ] SSID validation (1-32 chars)
- [ ] Pair code validation (6 digits)
- [ ] Password validation (8+ chars)
- [ ] Settings save from web interface
- [ ] Run in Background toggle works
- [ ] Auto-start on boot works when enabled
- [ ] Manual start from TFT works
- [ ] Manual stop from TFT works
- [ ] All existing password manager features still work
- [ ] Import/Export still works
- [ ] Search still works
- [ ] Add/Edit/Delete still works

## Complete!

The Web UI Settings feature is now fully integrated while preserving all existing functionality.
