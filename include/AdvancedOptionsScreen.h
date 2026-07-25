#ifndef ADVANCEDOPTIONSSCREEN_H
#define ADVANCEDOPTIONSSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Forward declarations
class SettingsManager;

// Navigation actions for menu items
enum AdvancedOptionAction {
    ACTION_NONE = -1,
    ACTION_TOUCH_TEST = 0,
    ACTION_CALIBRATION = 1,
    ACTION_ORIENTATION = 2,
    ACTION_RESET_CALIBRATION = 3,
    ACTION_FACTORY_RESET = 4,
    ACTION_BACK = 99
};

class AdvancedOptionsScreen {
public:
    AdvancedOptionsScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr);
    
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
    
    // Get the selected action (if any)
    AdvancedOptionAction getSelectedAction() const { return selectedAction; }
    
    // Clear selected action after it's been handled
    void clearAction() { selectedAction = ACTION_NONE; }
    
    // Reset exit flag and state
    void reset();
    
    // Scroll control (called by external FAB handler)
    void scrollDown();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    SettingsManager* settings;
    
    bool needsExit;  // Set to true when user exits screen
    AdvancedOptionAction selectedAction;  // Set when user selects an option
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Scroll offset (discrete step scrolling)
    int scrollOffset;  // Current scroll position in items (0, 1, 2...)
    
    // Menu items
    static const int MENU_ITEM_HEIGHT = 50;
    static const int MENU_ITEM_COUNT = 5;  // 5 advanced options
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Scroll handling (discrete step)
    int getMaxScrollOffset() const;
    
    // Menu item hit detection
    int getMenuItemAtPosition(int16_t x, int16_t y);
    
    // Menu item labels
    const char* getMenuItemLabel(int index);
};

#endif // ADVANCEDOPTIONSSCREEN_H
