#include "WebUIStatusScreen.h"

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_SUCCESS 0x07E0      // Green for active status
#define COLOR_ERROR 0xF800        // Red for inactive status

WebUIStatusScreen::WebUIStatusScreen(TFT_eSPI* display, TouchManager* touchMgr, WebUISettingsManager* webUISettings, bool* webUIActive)
    : tft(display),
      touch(touchMgr),
      webSettings(webUISettings),
      isWebUIActive(webUIActive),
      forceFullRedraw(true),
      screenIsDirty(true),
      exitRequested(false) {
}

void WebUIStatusScreen::begin() {
    Serial.println("[WEBUI_SCREEN] Screen initialized");
    exitRequested = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void WebUIStatusScreen::reset() {
    exitRequested = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void WebUIStatusScreen::update() {
    // Screen updates driven by touch events
}

void WebUIStatusScreen::draw() {
    if (forceFullRedraw) {
        tft->fillScreen(COLOR_INK);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

void WebUIStatusScreen::drawStaticUI() {
    // Status bar (24px)
    tft->fillRect(0, 0, 320, 24, COLOR_INK);
    tft->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // Header (32px)
    tft->fillRect(0, 24, 320, 32, COLOR_INK);
    
    // Back button
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 16, 32);
    
    // Title
    tft->setTextDatum(TC_DATUM);
    tft->drawString("Web UI", 160, 32);
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, 320, COLOR_LINE);
    
    Serial.println("[WEBUI_SCREEN] Static UI drawn");
}

void WebUIStatusScreen::drawDynamicData() {
    // Clear content area
    tft->fillRect(0, HEADER_HEIGHT, 320, 240 - HEADER_HEIGHT, COLOR_INK);
    
    int y = HEADER_HEIGHT + 6;
    
    // Status indicator
    if (*isWebUIActive) {
        // ACTIVE - Show green status (compact)
        tft->setTextColor(COLOR_SUCCESS, COLOR_INK);
        tft->setTextSize(2);
        tft->setTextDatum(TC_DATUM);
        tft->drawString("ACTIVE", 160, y);
        y += 20;
        
        tft->setTextSize(1);
        tft->setTextDatum(TL_DATUM);
        
        // WiFi Name
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("SSID:");
        tft->setTextColor(COLOR_IVORY, COLOR_INK);
        tft->setCursor(SIDE_MARGIN + 42, y);
        tft->print(webSettings->getSSID());
        y += 14;
        
        // WiFi Password
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("Pass:");
        tft->setTextColor(COLOR_IVORY, COLOR_INK);
        tft->setCursor(SIDE_MARGIN + 42, y);
        tft->print(webSettings->getWiFiPassword());
        y += 14;
        
        // Pair Code (prominent)
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("Code:");
        tft->setTextColor(COLOR_BRASS, COLOR_INK);
        tft->setTextSize(2);
        tft->setCursor(SIDE_MARGIN + 42, y - 2);
        tft->print(webSettings->getPairCode());
        y += 20;
        
        // IP Address
        tft->setTextSize(1);
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("URL:");
        tft->setTextColor(COLOR_IVORY, COLOR_INK);
        IPAddress ip = WiFi.softAPIP();
        tft->setCursor(SIDE_MARGIN + 42, y);
        tft->print("http://" + ip.toString());
        
    } else {
        // INACTIVE - Show red status (compact)
        tft->setTextColor(COLOR_ERROR, COLOR_INK);
        tft->setTextSize(2);
        tft->setTextDatum(TC_DATUM);
        tft->drawString("INACTIVE", 160, y);
        y += 20;
        
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setTextSize(1);
        tft->setTextDatum(TL_DATUM);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("Web UI is currently off.");
        y += 14;
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("Tap Start button below to launch WiFi AP");
        y += 14;
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("and access the password web portal.");
        y += 18;
        
        tft->setTextColor(COLOR_MUTED, COLOR_INK);
        tft->setCursor(SIDE_MARGIN, y);
        tft->print("SSID: ");
        tft->setTextColor(COLOR_IVORY, COLOR_INK);
        tft->print(webSettings->getSSID());
    }
    
    // Draw "Run in Background" Toggle Row (Y=145)
    tft->drawFastHLine(SIDE_MARGIN, TOGGLE_Y - 4, 320 - (2 * SIDE_MARGIN), COLOR_LINE);
    
    tft->setTextSize(1);
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setCursor(SIDE_MARGIN, TOGGLE_Y + 2);
    tft->print("Run in Background");
    
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setCursor(SIDE_MARGIN, TOGGLE_Y + 16);
    tft->print("Auto-start WiFi AP on boot");
    
    bool bgEnabled = webSettings->getRunInBackground();
    drawToggleSwitch(250, TOGGLE_Y + 4, bgEnabled);
    
    // Draw Start/Stop Action Button (Y=190)
    int startBtnX = (320 - BUTTON_WIDTH) / 2;
    if (*isWebUIActive) {
        tft->fillRoundRect(startBtnX, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6, 0x7800);  // Dark red
        tft->drawRoundRect(startBtnX, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6, COLOR_ERROR);
        tft->setTextColor(COLOR_ERROR, 0x7800);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Stop Web UI", 160, BUTTON_Y + BUTTON_HEIGHT / 2);
    } else {
        tft->fillRoundRect(startBtnX, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6, COLOR_SURFACE);
        tft->drawRoundRect(startBtnX, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 6, COLOR_BRASS);
        tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Start Web UI", 160, BUTTON_Y + BUTTON_HEIGHT / 2);
    }
    
    tft->setTextDatum(TL_DATUM);  // Reset
    Serial.println("[WEBUI_SCREEN] Dynamic data drawn");
}

void WebUIStatusScreen::drawToggleSwitch(int x, int y, bool enabled) {
    int width = 46;
    int height = 22;
    uint16_t bgColor = enabled ? COLOR_BRASS : COLOR_SURFACE;
    uint16_t borderColor = enabled ? COLOR_BRASS : COLOR_LINE;
    
    // Outer rounded pill capsule
    tft->fillRoundRect(x, y, width, height, height / 2, bgColor);
    tft->drawRoundRect(x, y, width, height, height / 2, borderColor);
    
    // Inner circular knob
    int knobX = enabled ? (x + width - height / 2) : (x + height / 2);
    int knobY = y + height / 2;
    uint16_t knobColor = enabled ? COLOR_INK : COLOR_MUTED;
    
    tft->fillCircle(knobX, knobY, (height / 2) - 3, knobColor);
}

void WebUIStatusScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[WEBUI_SCREEN] Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[WEBUI_SCREEN] Back button pressed");
        exitRequested = true;
        return;
    }
    
    // Check Run in Background toggle touch
    if (isToggleTouch(point.x, point.y)) {
        bool current = webSettings->getRunInBackground();
        webSettings->setRunInBackground(!current);
        webSettings->save();
        Serial.printf("[WEBUI_SCREEN] Toggled Run in Background to: %s\n", !current ? "ON" : "OFF");
        
        forceFullRedraw = true;
        screenIsDirty = true;
        return;
    }
    
    // Check Start/Stop button
    if (isStartStopButtonPressed(point.x, point.y)) {
        Serial.println("[WEBUI_SCREEN] Start/Stop button pressed");
        
        // Call external functions to start/stop Web UI
        extern void startWebUI();
        extern void stopWebUI();
        
        if (*isWebUIActive) {
            Serial.println("[WEBUI_SCREEN] Stopping Web UI...");
            stopWebUI();
        } else {
            Serial.println("[WEBUI_SCREEN] Starting Web UI...");
            startWebUI();
        }
        
        // Redraw to show new status
        forceFullRedraw = true;
        screenIsDirty = true;
        return;
    }
}

bool WebUIStatusScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}

bool WebUIStatusScreen::isStartStopButtonPressed(int16_t x, int16_t y) const {
    int startBtnX = (320 - BUTTON_WIDTH) / 2;
    return (x >= startBtnX && x <= startBtnX + BUTTON_WIDTH &&
            y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT);
}

bool WebUIStatusScreen::isToggleTouch(int16_t x, int16_t y) const {
    return (y >= TOGGLE_Y - 4 && y <= TOGGLE_Y + TOGGLE_HEIGHT &&
            x >= SIDE_MARGIN && x <= 320 - SIDE_MARGIN);
}
