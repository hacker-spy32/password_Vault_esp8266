#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "Hardware.h"
#include "TouchManager.h"
#include "ScreenManager.h"
#include "PINManager.h"
#include "WebUISettingsManager.h"
#include "VaultStorage.h"
#include "portal_html.h"

// Global managers
TouchManager* touchManager = nullptr;
ScreenManager* screenManager = nullptr;
WebUISettingsManager webUISettings;
VaultStorage* vaultStorage = nullptr;

// Web server & Captive Portal DNS
ESP8266WebServer* webServer = nullptr;
DNSServer* dnsServer = nullptr;
bool webUIActive = false;

// ========== WEB SERVER ENDPOINT HANDLERS ==========

/**
 * GET /webui/settings
 * Returns current Web UI settings as JSON
 */
void handleGetWebUISettings() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
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
    String currentCode = webServer->arg("code");
    if (currentCode != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    String ssid = webServer->arg("ssid");
    String newCode = webServer->arg("pairCode");
    String password = webServer->arg("wifiPassword");
    String bgStr = webServer->arg("runInBackground");
    bool runInBg = (bgStr == "1" || bgStr == "true");
    
    if (webUISettings.setAll(ssid, newCode, password, runInBg)) {
        if (webUISettings.save()) {
            webServer->send(200, "text/plain", "OK");
            Serial.println("[WEBUI_API] Settings saved - Restart device to apply");
        } else {
            webServer->send(500, "text/plain", "Save failed");
        }
    } else {
        webServer->send(400, "text/plain", "Invalid parameters");
    }
}

/**
 * GET /list
 * Returns password list as JSON
 */
void handleList() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!vaultStorage) {
        webServer->send(500, "text/plain", "Vault not initialized");
        return;
    }
    
    // Get all passwords
    std::vector<PasswordEntry> entries = vaultStorage->getAllPasswords();
    
    // Build JSON array
    String json = "[";
    for (size_t i = 0; i < entries.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"id\":" + String(entries[i].id) + ",";
        json += "\"title\":\"" + String(entries[i].title) + "\",";
        json += "\"user\":\"" + String(entries[i].user) + "\",";
        json += "\"pass\":\"" + String(entries[i].pass) + "\",";
        json += "\"isOneTime\":" + String(entries[i].isOneTime ? "true" : "false") + ",";
        json += "\"url\":\"\"";  // URL field not in PasswordEntry struct
        json += "}";
    }
    json += "]";
    
    webServer->send(200, "application/json", json);
    Serial.printf("[WEBUI_API] GET /list - Returned %d entries\n", entries.size());
}

/**
 * POST /save
 * Save new password entry or bulk import
 */
void handleSave() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!vaultStorage) {
        webServer->send(500, "text/plain", "Vault not initialized");
        return;
    }
    
    // Check if this is a bulk import
    if (webServer->hasArg("bulk")) {
        String bulk = webServer->arg("bulk");
        Serial.println("[WEBUI_API] POST /save - Bulk import");
        
        // Parse CSV lines (title,user,pass[,url,isOneTime])
        int imported = 0;
        int lineStart = 0;
        while (lineStart < bulk.length()) {
            int lineEnd = bulk.indexOf('\n', lineStart);
            if (lineEnd < 0) lineEnd = bulk.length();
            
            String line = bulk.substring(lineStart, lineEnd);
            line.trim();
            
            if (line.length() > 0 && line.indexOf(',') > 0) {
                int comma1 = line.indexOf(',');
                int comma2 = line.indexOf(',', comma1 + 1);
                int comma3 = line.indexOf(',', comma2 + 1);
                int comma4 = line.indexOf(',', comma3 + 1);
                
                if (comma1 > 0 && comma2 > comma1) {
                    PasswordEntry entry;
                    memset(&entry, 0, sizeof(entry));
                    strncpy(entry.title, line.substring(0, comma1).c_str(), sizeof(entry.title) - 1);
                    strncpy(entry.user, line.substring(comma1 + 1, comma2).c_str(), sizeof(entry.user) - 1);
                    
                    String passStr = (comma3 > comma2) ? line.substring(comma2 + 1, comma3) : line.substring(comma2 + 1);
                    strncpy(entry.pass, passStr.c_str(), sizeof(entry.pass) - 1);
                    
                    entry.isOneTime = false;
                    if (comma4 > comma3) {
                        String oneTimeFlag = line.substring(comma4 + 1);
                        oneTimeFlag.trim();
                        entry.isOneTime = (oneTimeFlag == "1" || oneTimeFlag == "true");
                    }
                    
                    if (strlen(entry.user) > 0 && strlen(entry.pass) > 0) {
                        if (vaultStorage->addPassword(entry)) {
                            imported++;
                        }
                    }
                }
            }
            
            lineStart = lineEnd + 1;
        }
        
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] Imported %d entries\n", imported);
        return;
    }
    
    // Single entry save
    String title = webServer->arg("title");
    String user = webServer->arg("user");
    String pass = webServer->arg("pass");
    String oneTimeStr = webServer->arg("isOneTime");
    bool isOneTime = (oneTimeStr == "1" || oneTimeStr == "true");
    
    PasswordEntry entry;
    memset(&entry, 0, sizeof(entry));
    strncpy(entry.title, title.c_str(), sizeof(entry.title) - 1);
    strncpy(entry.user, user.c_str(), sizeof(entry.user) - 1);
    strncpy(entry.pass, pass.c_str(), sizeof(entry.pass) - 1);
    entry.isFavorite = false;
    entry.isOneTime = isOneTime;
    
    if (vaultStorage->addPassword(entry)) {
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] POST /save - Added: %s (OneTime=%d)\n", entry.title, entry.isOneTime);
    } else {
        webServer->send(500, "text/plain", "Save failed");
    }
}

/**
 * POST /edit
 * Edit existing password entry
 */
void handleEdit() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!vaultStorage) {
        webServer->send(500, "text/plain", "Vault not initialized");
        return;
    }
    
    uint32_t id = webServer->arg("id").toInt();
    String title = webServer->arg("title");
    String user = webServer->arg("user");
    String pass = webServer->arg("pass");
    // url parameter ignored
    
    PasswordEntry entry;
    entry.id = id;
    strncpy(entry.title, title.c_str(), sizeof(entry.title) - 1);
    entry.title[sizeof(entry.title) - 1] = '\0';
    strncpy(entry.user, user.c_str(), sizeof(entry.user) - 1);
    entry.user[sizeof(entry.user) - 1] = '\0';
    strncpy(entry.pass, pass.c_str(), sizeof(entry.pass) - 1);
    entry.pass[sizeof(entry.pass) - 1] = '\0';
    
    if (vaultStorage->updatePassword(id, entry)) {
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] POST /edit - Updated ID=%d\n", id);
    } else {
        webServer->send(500, "text/plain", "Update failed");
    }
}

/**
 * POST /delete
 * Delete password entry
 */
void handleDelete() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!vaultStorage) {
        webServer->send(500, "text/plain", "Vault not initialized");
        return;
    }
    
    uint32_t id = webServer->arg("id").toInt();
    
    if (vaultStorage->deletePassword(id)) {
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] POST /delete - Deleted ID=%d\n", id);
    } else {
        webServer->send(500, "text/plain", "Delete failed");
    }
}

/**
 * GET /export
 * Export passwords as CSV
 */
void handleExport() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!vaultStorage) {
        webServer->send(500, "text/plain", "Vault not initialized");
        return;
    }
    
    std::vector<PasswordEntry> entries = vaultStorage->getAllPasswords();
    
    String csv = "title,username,password,url\n";
    for (const auto& entry : entries) {
        csv += String(entry.title) + ",";
        csv += String(entry.user) + ",";
        csv += String(entry.pass) + ",";
        csv += "\n";  // No URL field in PasswordEntry
    }
    
    webServer->sendHeader("Content-Disposition", "attachment; filename=securekey_export.csv");
    webServer->send(200, "text/csv", csv);
    Serial.printf("[WEBUI_API] GET /export - Exported %d entries\n", entries.size());
}

/**
 * GET /temp_pins/list
 * Returns list of temporary keypad PINs as JSON
 */
void handleListTempPINs() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!screenManager || !screenManager->getPINManager()) {
        webServer->send(500, "text/plain", "PINManager not initialized");
        return;
    }
    
    std::vector<TempPIN> pins = screenManager->getPINManager()->getTempPINs();
    String json = "[";
    for (size_t i = 0; i < pins.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"id\":" + String(pins[i].id) + ",";
        json += "\"pin\":\"" + String(pins[i].pin) + "\",";
        json += "\"note\":\"" + String(pins[i].note) + "\",";
        json += "\"isUsed\":" + String(pins[i].isUsed ? "true" : "false");
        json += "}";
    }
    json += "]";
    
    webServer->send(200, "application/json", json);
    Serial.printf("[WEBUI_API] GET /temp_pins/list - Returned %d entries\n", pins.size());
}

/**
 * POST /temp_pins/add
 * Add a new 4-digit temporary keypad PIN
 */
void handleAddTempPIN() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!screenManager || !screenManager->getPINManager()) {
        webServer->send(500, "text/plain", "PINManager not initialized");
        return;
    }
    
    String pin = webServer->arg("pin");
    String note = webServer->arg("note");
    
    if (screenManager->getPINManager()->addTempPIN(pin.c_str(), note.c_str())) {
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] POST /temp_pins/add - Added PIN: %s (%s)\n", pin.c_str(), note.c_str());
    } else {
        webServer->send(400, "text/plain", "Invalid PIN (Must be 4 digits)");
    }
}

/**
 * POST /temp_pins/delete
 * Delete a temporary keypad PIN by ID
 */
void handleDeleteTempPIN() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!screenManager || !screenManager->getPINManager()) {
        webServer->send(500, "text/plain", "PINManager not initialized");
        return;
    }
    
    uint32_t id = webServer->arg("id").toInt();
    if (screenManager->getPINManager()->deleteTempPIN(id)) {
        webServer->send(200, "text/plain", "OK");
        Serial.printf("[WEBUI_API] POST /temp_pins/delete - Deleted ID=%u\n", id);
    } else {
        webServer->send(404, "text/plain", "ID not found");
    }
}

/**
 * POST /temp_pins/clear_expired
 * Clear all used/expired temporary PINs
 */
void handleClearExpiredTempPINs() {
    String code = webServer->arg("code");
    if (code != webUISettings.getPairCode()) {
        webServer->send(403, "text/plain", "Forbidden");
        return;
    }
    
    if (!screenManager || !screenManager->getPINManager()) {
        webServer->send(500, "text/plain", "PINManager not initialized");
        return;
    }
    
    screenManager->getPINManager()->clearExpiredTempPINs();
    webServer->send(200, "text/plain", "OK");
    Serial.println("[WEBUI_API] POST /temp_pins/clear_expired - Cleared expired PINs");
}

// ========== WEB UI START/STOP FUNCTIONS ==========

/**
 * Start the Web UI (WiFi hotspot + web server + Captive Portal DNS)
 */
void startWebUI() {
    if (webUIActive) {
        Serial.println("[WEBUI] Already running");
        return;
    }
    
    Serial.println("[WEBUI] ========================================");
    Serial.println("[WEBUI] Starting Web UI & Captive Portal...");
    
    String ssid = webUISettings.getSSID();
    String password = webUISettings.getWiFiPassword();
    String pairCode = webUISettings.getPairCode();
    
    Serial.printf("[WEBUI] SSID: %s\n", ssid.c_str());
    Serial.printf("[WEBUI] Pair Code: %s\n", pairCode.c_str());
    
    // Safely deselect TFT SPI before powering on WiFi radio
    digitalWrite(TFT_CS, HIGH);
    
    // Start SoftAP
    WiFi.mode(WIFI_AP);
    bool apStarted = WiFi.softAP(ssid.c_str(), password.c_str());
    
    if (!apStarted) {
        Serial.println("[WEBUI] ERROR: Failed to start Access Point");
        return;
    }
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("[WEBUI] Access Point started - IP: %s\n", IP.toString().c_str());
    
    // Start Captive Portal DNS Server (redirect all DNS queries to ESP8266 IP)
    dnsServer = new DNSServer();
    dnsServer->start(53, "*", IP);
    Serial.println("[WEBUI] Captive Portal DNS Server active on port 53");
    
    // Create web server
    webServer = new ESP8266WebServer(80);
    
    // Serve portal HTML
    webServer->on("/", HTTP_GET, []() {
        webServer->send_P(200, "text/html", PORTAL_HTML);
    });
    
    // Password manager endpoints
    webServer->on("/list", HTTP_GET, handleList);
    webServer->on("/save", HTTP_POST, handleSave);
    webServer->on("/edit", HTTP_POST, handleEdit);
    webServer->on("/delete", HTTP_POST, handleDelete);
    webServer->on("/export", HTTP_GET, handleExport);
    
    // Temporary Keypad PIN endpoints
    webServer->on("/temp_pins/list", HTTP_GET, handleListTempPINs);
    webServer->on("/temp_pins/add", HTTP_POST, handleAddTempPIN);
    webServer->on("/temp_pins/delete", HTTP_POST, handleDeleteTempPIN);
    webServer->on("/temp_pins/clear_expired", HTTP_POST, handleClearExpiredTempPINs);
    
    // Web UI Settings endpoints
    webServer->on("/webui/settings", HTTP_GET, handleGetWebUISettings);
    webServer->on("/webui/settings", HTTP_POST, handleSaveWebUISettings);
    
    // Captive portal detection probe endpoints (Android, iOS, Windows, macOS, Firefox)
    auto handleCaptiveRedirect = []() {
        IPAddress apIP = WiFi.softAPIP();
        String target = "http://" + apIP.toString() + "/";
        webServer->sendHeader("Location", target, true);
        webServer->send(302, "text/plain", "");
    };

    webServer->on("/generate_204", handleCaptiveRedirect);
    webServer->on("/gen_204", handleCaptiveRedirect);
    webServer->on("/hotspot-detect.html", handleCaptiveRedirect);
    webServer->on("/canonical.html", handleCaptiveRedirect);
    webServer->on("/connecttest.txt", handleCaptiveRedirect);
    webServer->on("/redirect", handleCaptiveRedirect);
    webServer->on("/success.txt", handleCaptiveRedirect);
    webServer->on("/nintendowifi.jsp", handleCaptiveRedirect);
    
    // Captive portal catch-all handler for unknown domain requests
    webServer->onNotFound([]() {
        IPAddress apIP = WiFi.softAPIP();
        String host = webServer->hostHeader();
        
        // If request is specifically for root host IP or local endpoint, serve 404
        if (host == apIP.toString() || host.indexOf("192.168.4.1") >= 0) {
            webServer->send(404, "text/plain", "Not found");
        } else {
            // Redirect external domain probe requests to local portal IP
            String target = "http://" + apIP.toString() + "/";
            webServer->sendHeader("Location", target, true);
            webServer->send(302, "text/plain", "");
        }
    });
    
    webServer->begin();
    webUIActive = true;
    
    // Re-initialize ILI9341 display registers to recover from any WiFi RF power glitches
    tft.init();
    applyDisplayOrientation(screenManager ? screenManager->getSettingsManager() : nullptr);
    if (screenManager) {
        screenManager->requestFullRedraw();
        screenManager->render();
    }
    
    Serial.println("[WEBUI] Web server started");
    Serial.printf("[WEBUI] Connect to WiFi and open: http://%s\n", IP.toString().c_str());
    Serial.println("[WEBUI] Captive Portal active - Auto popup enabled");
    Serial.println("[WEBUI] ========================================");
}

/**
 * Stop the Web UI
 */
void stopWebUI() {
    if (!webUIActive) {
        Serial.println("[WEBUI] Already stopped");
        return;
    }
    
    Serial.println("[WEBUI] Stopping Web UI...");
    
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }
    
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
    }
    
    digitalWrite(TFT_CS, HIGH);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    
    webUIActive = false;
    
    // Re-initialize ILI9341 display registers
    tft.init();
    applyDisplayOrientation(screenManager ? screenManager->getSettingsManager() : nullptr);
    if (screenManager) {
        screenManager->requestFullRedraw();
        screenManager->render();
    }
    
    Serial.println("[WEBUI] Web UI stopped");
}

/**
 * Check if Web UI is running
 */
bool isWebUIActive() {
    return webUIActive;
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("\nStarting NodeMCU ESP8266 Event-Driven Touch System...");
    
    // CPU frequency is set to 160MHz in platformio.ini
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    
    // Initialize hardware with pin-safe sequence
    initHardware();
    
    // Initialize Web UI Settings Manager
    Serial.println("[BOOT] Initializing Web UI Settings...");
    webUISettings.begin();
    
    // Initialize Vault Storage
    Serial.println("[BOOT] Initializing Vault Storage...");
    vaultStorage = new VaultStorage();
    vaultStorage->begin();
    
    // Create touch manager
    touchManager = new TouchManager(&ts);
    touchManager->begin();
    
    // Create screen manager
    screenManager = new ScreenManager(&tft, touchManager);
    screenManager->begin();
    
    // Set Web UI references for Web UI Status screen
    screenManager->setWebUIReferences(&webUISettings, &webUIActive);
    
    // Draw initial screen
    screenManager->render();
    
    Serial.println("System ready - Event-driven loop active");
    
    // ===== WEB UI AUTO-START LOGIC =====
    if (webUISettings.getRunInBackground()) {
        Serial.println("[BOOT] Run in Background: ON - Starting Web UI automatically");
        startWebUI();
    } else {
        Serial.println("[BOOT] Run in Background: OFF - Web UI not started");
        Serial.println("[BOOT] Use TFT interface to start Web UI manually");
    }
}

void loop() {
    // Handle web server & captive portal DNS if active
    if (webUIActive) {
        if (dnsServer) {
            dnsServer->processNextRequest();
        }
        if (webServer) {
            webServer->handleClient();
        }
    }
    
    // Event-driven architecture:
    // 1. Update touch state machine
    touchManager->update();
    
    // 2. Update screen logic based on touch events
    screenManager->update();
    
    // 3. Check auto-lock timer
    screenManager->checkAutoLock();
    
    // 4. Render screen if needed (Touch Test screen updates continuously)
    screenManager->render();
    
    // Small delay for stability (50ms = 20 updates per second)
    delay(50);
}
