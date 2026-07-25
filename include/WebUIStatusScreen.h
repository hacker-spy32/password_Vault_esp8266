#ifndef WEBUISTATUSSCREEN_H
#define WEBUISTATUSSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP8266WiFi.h>
#include "TouchManager.h"
#include "WebUISettingsManager.h"

/**
 * WebUIStatusScreen - Displays Web UI connection information
 * Shows SSID, Password, Pair Code, and IP address when Web UI is active
 */
class WebUIStatusScreen {
public:
    WebUIStatusScreen(TFT_eSPI* display, TouchManager* touchMgr, WebUISettingsManager* webUISettings, bool* webUIActive);
    
    // Lifecycle
    void begin();
    void reset();
    
    // Update and render
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // State
    bool needsExit() const { return exitRequested; }
    bool needsRedraw() const { return forceFullRedraw || screenIsDirty; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    WebUISettingsManager* webSettings;
    bool* isWebUIActive;
    
    // Screen state
    bool forceFullRedraw;
    bool screenIsDirty;
    bool exitRequested;
    
    // Layout constants
    static const int HEADER_HEIGHT = 56;
    static const int SIDE_MARGIN = 20;
    static const int TOGGLE_Y = 145;
    static const int TOGGLE_HEIGHT = 32;
    static const int BUTTON_Y = 190;  // Fixed button position for both states
    static const int BUTTON_HEIGHT = 32;
    static const int BUTTON_WIDTH = 130;
    
    // Drawing
    void drawStaticUI();
    void drawDynamicData();
    void drawToggleSwitch(int x, int y, bool enabled);
    
    // Touch handling
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    bool isStartStopButtonPressed(int16_t x, int16_t y) const;
    bool isToggleTouch(int16_t x, int16_t y) const;
};

#endif // WEBUISTATUSSCREEN_H
