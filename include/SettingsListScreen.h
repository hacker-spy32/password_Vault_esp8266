#ifndef SETTINGSLISTSCREEN_H
#define SETTINGSLISTSCREEN_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "SettingsManager.h"
#include "SidebarScrollButtons.h"
#include "Config.h"

// Settings row items
enum SettingsRow {
    SETTINGS_ROW_NONE = 0,
    SETTINGS_ROW_AUTO_LOCK,
    SETTINGS_ROW_TWO_TAP_SLEEP,
    SETTINGS_ROW_CHANGE_PIN,  // NEW: Change PIN option
    SETTINGS_ROW_ADVANCED,
    SETTINGS_ROW_ABOUT
};

// Auto-lock timeout options (in seconds)
enum AutoLockTimeout {
    TIMEOUT_5S = 5,
    TIMEOUT_10S = 10,
    TIMEOUT_15S = 15,
    TIMEOUT_20S = 20,
    TIMEOUT_30S = 30,
    TIMEOUT_NEVER = 0
};

class SettingsListScreen {
public:
    SettingsListScreen(TFT_eSPI* display, TouchManager* touch, SettingsManager* settings);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Navigation
    bool needsToExit() const { return needsExit; }
    SettingsRow getSelectedAction() const { return selectedAction; }
    void clearAction() { selectedAction = SETTINGS_ROW_NONE; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    SettingsManager* settingsManager;
    SidebarScrollButtons* sidebarButtons;
    
    // State
    bool needsExit;
    SettingsRow selectedAction;
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Settings values
    uint8_t autoLockTimeout;  // In seconds (0 = never)
    bool twoTapSleepEnabled;
    
    // Scroll state (5 rows no longer fit in the 182px visible area,
    // so this screen now scrolls via the same sidebar rail as Passwords/Favorites)
    int scrollOffset;
    int maxScrollOffset;
    static const int ROW_COUNT = 5;
    
    // Layout constants
    static const int HEADER_HEIGHT = 56;
    static const int ROW_HEIGHT = 42;
    static const int ROW_SPACING = 8;
    static const int ROW_WIDTH = 256;    // Leaves room for the 40px sidebar zone (matches PasswordsScreen)
    static const int SIDE_MARGIN = 16;
    static const int ROWS_START_Y = 58;  // Start below header
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawSettingsRow(int y, const char* label, const char* value, const char* rightIcon, bool hasToggle, bool toggleState);
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    SettingsRow getRowAtPosition(int16_t x, int16_t y) const;
    bool isToggleSwitchPressed(int16_t x, int16_t y, SettingsRow row) const;
    bool isDropdownPressed(int16_t x, int16_t y, SettingsRow row) const;
    
    // Settings management
    void cycleAutoLockTimeout();
    void toggleTwoTapSleep();
    const char* getAutoLockTimeoutString() const;
    
    // Helper methods
    void drawToggleSwitch(int x, int y, bool enabled);
    void drawDropdownIndicator(int x, int y);
    void calculateMaxScroll();
};

#endif
