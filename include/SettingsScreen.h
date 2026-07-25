#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Forward declarations
class SettingsManager;

class SettingsScreen {
public:
    SettingsScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr);
    
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
    
    bool needsExit;  // Set to true when user exits settings
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Experimental scroll members
    int scrollY;
    bool dragging;
    int lastTouchY;
    
    // Menu items
    static const int MENU_ITEM_HEIGHT = 50;
    static const int MENU_ITEM_COUNT = 11;  // Updated: added Orientation option
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Scroll handling
    void updateScroll();
    
    // Menu item hit detection
    int getMenuItemAtPosition(int16_t x, int16_t y);
};

#endif // SETTINGSSCREEN_H
