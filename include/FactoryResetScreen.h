#ifndef FACTORYRESETSCREEN_H
#define FACTORYRESETSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Forward declarations
class SettingsManager;

class FactoryResetScreen {
public:
    FactoryResetScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr);
    
    // Initialize screen
    void begin();
    
    // Update logic (NO DRAWING)
    void update();
    
    // Draw function (ONLY DRAWING) - THE STANDARD
    void draw();
    
    // Handle touch events (NO DRAWING)
    void onTouchEvent(const TouchPoint& point);
    
    // Check if user wants to exit
    bool needsToExit() const { return needsExit; }
    
    // Reset exit flag
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    SettingsManager* settings;
    
    bool needsExit;  // Set to true when user cancels or completes action
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Button zones
    static const int16_t CANCEL_BTN_X = 20;
    static const int16_t CANCEL_BTN_Y = 190;
    static const int16_t CANCEL_BTN_W = 130;
    static const int16_t CANCEL_BTN_H = 40;
    
    static const int16_t RESET_BTN_X = 170;
    static const int16_t RESET_BTN_Y = 190;
    static const int16_t RESET_BTN_W = 130;
    static const int16_t RESET_BTN_H = 40;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Button hit detection
    bool isCancelButtonPressed(int16_t x, int16_t y);
    bool isResetButtonPressed(int16_t x, int16_t y);
    
    // Action handler
    void performFactoryReset();
};

#endif // FACTORYRESETSCREEN_H
