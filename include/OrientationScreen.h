#ifndef ORIENTATIONSCREEN_H
#define ORIENTATIONSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Forward declarations
class SettingsManager;

class OrientationScreen {
public:
    OrientationScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr);
    
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
    
    // Check if orientation was changed (to trigger display/touch update)
    bool orientationChanged() const { return orientationWasChanged; }
    
    // Reset exit flag and change flag
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    SettingsManager* settings;
    
    bool needsExit;  // Set to true when user exits screen
    bool orientationWasChanged;  // Set to true when orientation changes
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // UI constants
    static const int HEADER_HEIGHT = 40;
    static const int BUTTON_WIDTH = 280;
    static const int BUTTON_HEIGHT = 50;
    static const int BUTTON_SPACING = 20;
    
    // Button positions
    static const int LANDSCAPE_BTN_Y = 70;
    static const int LANDSCAPE_INV_BTN_Y = 140;
    static const int BACK_BTN_Y = 45;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Button hit detection
    bool isLandscapeButtonPressed(int16_t x, int16_t y);
    bool isLandscapeInvertedButtonPressed(int16_t x, int16_t y);
    bool isBackButtonPressed(int16_t x, int16_t y);
};

#endif // ORIENTATIONSCREEN_H
