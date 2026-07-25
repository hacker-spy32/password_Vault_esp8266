/**
 * SecureKey - Web UI Integration Example
 * 
 * This file demonstrates complete integration of the Web UI Settings feature
 * into your SecureKey firmware. Copy relevant sections to your main.ino or main.cpp.
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// SecureKey managers
#include "WebUISettingsManager.h"
#include "PINManager.h"
#include "VaultStorage.h"
#include "SettingsManager.h"
#include "portal_html.h"

// Hardware
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "ScreenManager.h"

// ========== GLOBAL INSTANCES ==========

// Web UI components
WebUISettingsManager webUISettings;
ESP8266WebServer* webServer = nullptr;
bool webUIActive = false;

// Existing managers (your existing code)
PINManager pinManager;
VaultStorage vaultStorage;
SettingsManager settingsManager;
TFT_eSPI tft;
TouchManager touchManager(&tft);
ScreenManager screenManager(&tft, &touchManager, &settingsManager, &vaultStorage);

// ========== WEB SERVER ENDPOINT HANDLERS ==========

// ===== EXISTING ENDPOINTS (PRESERVE ALL - YOUR CODE) =====

void handleList() {
    // Your existing /list implementation
    // Verify code, return password list as JSON
}

void handleSave() {
    // Your existing /save implementation
    // Save new password entry
}

void handleEdit() {
    // Your existing /edit implementation
    // Update existing password entry
}

void handleDelete() {
    // Your existing /delete implementation
    // Delete password entry
}

void handleExport() {
    // Your existing /export implementation
    // Export passwords as CSV
}

// ===== NEW WEB UI SETTINGS ENDPOINTS =====

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
 */
void handleSaveWebUISettings() {
    // Verify pair code authorization (use CURRENT code, not the new one)
    String currentCode = webServer->arg("code");
    if (currentCode != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        Serial.println("[WEBUI_API] POST /webui/settings - Forbidden (wrong code)");
        return;
    }
    
    // Extract parameters
    String ssid = webServer->arg("ssid");
    String newCode = webServer->arg("pairCode");
    String password = webServer->arg("wifiPassword");
    String bgStr = webServer->arg("runInBackground");
    bool runInBg = (bgStr == "1" || bgStr == "true");
    
    Serial.println("[WEBUI_API] POST /webui/settings - Received:");
    Serial.printf("  SSID: %s\n", ssid.c_str());
    Serial.printf("  Code: %s\n", newCode.c_str());
    Serial.printf("  Pass length: %d\n", password.length());
    Serial.printf("  Run in BG: %s\n", runInBg ? "YES" : "NO");
    
    // Validate and save
    if (webUISettings.setAll(ssid, newCode, password, runInBg)) {
        if (webUISettings.save()) {
            webServer->send(200, "text/plain", "OK");
            Serial.println("[WEBUI_API] Settings saved successfully");
            Serial.println("[WEBUI_API] Restart device to apply WiFi changes");
        } else {
            webServer->send(500, "text/plain", "Save failed");
            Serial.println("[WEBUI_API] Failed to save to storage");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid parameters");
        Serial.println("[WEBUI_API] Validation failed");
    }
}

// ========== WEB UI START/STOP FUNCTIONS ==========

/**
 * Start the Web UI (WiFi hotspot + web server)
 * Uses settings from WebUISettingsManager
 */
void startWebUI() {
    if (webUIActive) {
        Serial.println("[WEBUI] Already running");
        return;
    }
    
    Serial.println("[WEBUI] ========================================");
    Serial.println("[WEBUI] Starting Web UI...");
    
    // Get settings
    String ssid = webUISettings.getSSID();
    String password = webUISettings.getWiFiPassword();
    String pairCode = webUISettings.getPairCode();
    
    Serial.printf("[WEBUI] SSID: %s\n", ssid.c_str());
    Serial.printf("[WEBUI] Password: %s\n", password.c_str());
    Serial.printf("[WEBUI] Pair Code: %s\n", pairCode.c_str());
    
    // Start SoftAP
    WiFi.mode(WIFI_AP);
    bool apStarted = WiFi.softAP(ssid.c_str(), password.c_str());
    
    if (!apStarted) {
        Serial.println("[WEBUI] ERROR: Failed to start Access Point");
        return;
    }
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("[WEBUI] Access Point started\n");
    Serial.printf("[WEBUI] IP Address: %s\n", IP.toString().c_str());
    
    // Create and configure web server
    webServer = new ESP8266WebServer(80);
    
    // Serve main portal HTML
    webServer->on("/", HTTP_GET, []() {
        webServer->send_P(200, "text/html", PORTAL_HTML);
        Serial.println("[WEBUI] Served portal HTML");
    });
    
    // ===== EXISTING ENDPOINTS (YOUR IMPLEMENTATIONS) =====
    webServer->on("/list", HTTP_GET, handleList);
    webServer->on("/save", HTTP_POST, handleSave);
    webServer->on("/edit", HTTP_POST, handleEdit);
    webServer->on("/delete", HTTP_POST, handleDelete);
    webServer->on("/export", HTTP_GET, handleExport);
    
    // ===== NEW WEB UI SETTINGS ENDPOINTS =====
    webServer->on("/webui/settings", HTTP_GET, handleGetWebUISettings);
    webServer->on("/webui/settings", HTTP_POST, handleSaveWebUISettings);
    
    // 404 handler
    webServer->onNotFound([]() {
        webServer->send(404, "text/plain", "Not found");
        Serial.printf("[WEBUI] 404: %s\n", webServer->uri().c_str());
    });
    
    // Start server
    webServer->begin();
    webUIActive = true;
    
    Serial.println("[WEBUI] Web server started on port 80");
    Serial.println("[WEBUI] ========================================");
    Serial.println("[WEBUI] STATUS: ACTIVE");
    Serial.println("[WEBUI] Connect to WiFi and navigate to:");
    Serial.printf("[WEBUI]   http://%s\n", IP.toString().c_str());
    Serial.println("[WEBUI] ========================================");
}

/**
 * Stop the Web UI (turn off WiFi hotspot and web server)
 */
void stopWebUI() {
    if (!webUIActive) {
        Serial.println("[WEBUI] Already stopped");
        return;
    }
    
    Serial.println("[WEBUI] ========================================");
    Serial.println("[WEBUI] Stopping Web UI...");
    
    // Stop web server
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
        Serial.println("[WEBUI] Web server stopped");
    }
    
    // Disconnect clients and stop WiFi
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WEBUI] WiFi Access Point stopped");
    
    webUIActive = false;
    Serial.println("[WEBUI] STATUS: INACTIVE");
    Serial.println("[WEBUI] ========================================");
}

/**
 * Check if Web UI is currently running
 */
bool isWebUIActive() {
    return webUIActive;
}

/**
 * Get Web UI status for display on TFT
 */
String getWebUIStatus() {
    if (!webUIActive) {
        return "Web UI: Inactive";
    }
    
    String status = "Web UI: Active\n";
    status += "SSID: " + webUISettings.getSSID() + "\n";
    status += "Code: " + webUISettings.getPairCode() + "\n";
    status += "IP: " + WiFi.softAPIP().toString();
    return status;
}

// ========== ARDUINO SETUP ==========

void setup() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("       SecureKey Password Manager       ");
    Serial.println("========================================");
    Serial.println();
    
    // Initialize LittleFS
    Serial.println("[BOOT] Initializing LittleFS...");
    if (!LittleFS.begin()) {
        Serial.println("[FATAL] LittleFS mount failed!");
        Serial.println("[FATAL] Device cannot function without storage");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("[BOOT] LittleFS mounted successfully");
    
    // Initialize Web UI Settings Manager (NEW)
    Serial.println("[BOOT] Initializing Web UI Settings...");
    webUISettings.begin();
    Serial.println("[BOOT] Web UI Settings initialized");
    
    // Initialize PIN Manager
    Serial.println("[BOOT] Initializing PIN Manager...");
    pinManager.begin();
    Serial.println("[BOOT] PIN Manager initialized");
    
    // Initialize Vault Storage
    Serial.println("[BOOT] Initializing Vault Storage...");
    vaultStorage.begin();
    Serial.println("[BOOT] Vault Storage initialized");
    
    // Initialize Settings Manager
    Serial.println("[BOOT] Initializing Settings Manager...");
    settingsManager.begin();
    Serial.println("[BOOT] Settings Manager initialized");
    
    // Initialize hardware (TFT, touch, etc.)
    Serial.println("[BOOT] Initializing hardware...");
    tft.init();
    tft.setRotation(settingsManager.getOrientation());
    touchManager.begin();
    screenManager.begin();
    Serial.println("[BOOT] Hardware initialized");
    
    // ===== WEB UI AUTO-START LOGIC (NEW) =====
    Serial.println();
    Serial.println("========================================");
    Serial.println("       Web UI Initialization            ");
    Serial.println("========================================");
    
    if (webUISettings.getRunInBackground()) {
        Serial.println("[BOOT] Run in Background: ON");
        Serial.println("[BOOT] Starting Web UI automatically...");
        startWebUI();
    } else {
        Serial.println("[BOOT] Run in Background: OFF");
        Serial.println("[BOOT] Web UI not started");
        Serial.println("[BOOT] Use 'Start Web UI' button on device to enable");
    }
    
    Serial.println("========================================");
    Serial.println("[BOOT] Setup complete - entering main loop");
    Serial.println("========================================");
    Serial.println();
}

// ========== ARDUINO MAIN LOOP ==========

void loop() {
    // Handle web server requests if active
    if (webUIActive && webServer) {
        webServer->handleClient();
    }
    
    // Handle touch input
    touchManager.update();
    
    // Update screen manager
    screenManager.update();
    
    // Render screen
    screenManager.render();
    
    // Small delay to prevent watchdog timeout
    yield();
}

// ========== TFT INTERFACE INTEGRATION EXAMPLES ==========

/**
 * Example: User presses "Start Web UI" button on TFT interface
 * Call this from your TFT button handler
 */
void onStartWebUIButtonPressed() {
    if (!webUIActive) {
        Serial.println("[TFT] User requested Web UI start");
        startWebUI();
        
        // Show confirmation on TFT
        if (webUIActive) {
            // Display success message with connection details
            String message = "Web UI Started\n\n";
            message += "SSID: " + webUISettings.getSSID() + "\n";
            message += "Password: " + webUISettings.getWiFiPassword() + "\n";
            message += "Code: " + webUISettings.getPairCode() + "\n";
            message += "IP: " + WiFi.softAPIP().toString();
            
            // TODO: Show this message on your TFT screen
            // Example: screenManager.showToast(message);
            Serial.println("[TFT] Displaying connection info to user");
        } else {
            // Failed to start
            Serial.println("[TFT] Failed to start Web UI");
            // TODO: Show error on TFT
        }
    } else {
        Serial.println("[TFT] Web UI already running");
        // TODO: Show "Already running" message on TFT
    }
}

/**
 * Example: User presses "Stop Web UI" button on TFT interface
 * Call this from your TFT button handler
 */
void onStopWebUIButtonPressed() {
    if (webUIActive) {
        Serial.println("[TFT] User requested Web UI stop");
        stopWebUI();
        
        // Show confirmation on TFT
        // TODO: Show "Web UI stopped" message
        Serial.println("[TFT] Web UI stopped by user");
    } else {
        Serial.println("[TFT] Web UI not running");
        // TODO: Show "Not running" message on TFT
    }
}

/**
 * Example: Display Web UI status on TFT settings screen
 * Call this when rendering your settings screen
 */
void displayWebUIStatusOnTFT() {
    if (webUIActive) {
        // Show active status with connection details
        tft.setTextColor(TFT_GREEN);
        tft.println("Web UI: ACTIVE");
        tft.setTextColor(TFT_WHITE);
        tft.printf("SSID: %s\n", webUISettings.getSSID().c_str());
        tft.printf("Code: %s\n", webUISettings.getPairCode().c_str());
        tft.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        // Show inactive status
        tft.setTextColor(TFT_RED);
        tft.println("Web UI: INACTIVE");
        tft.setTextColor(TFT_WHITE);
        
        if (webUISettings.getRunInBackground()) {
            tft.println("(Auto-start enabled)");
        } else {
            tft.println("(Manual start only)");
        }
    }
}
