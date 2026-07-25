#include "SettingsScreen.h"
#include "SettingsManager.h"

// Design System Colors - Unified Dark Theme
#define COLOR_BACKGROUND 0x0841   // #0A0C10 - Pitch black background
#define COLOR_TEXT 0xFFDE         // #F4EFE4 - Clean white typography
#define COLOR_HEADER 0x18C3       // #141821 - Dark surface for header
#define COLOR_ITEM_BG 0x18C3      // #141821 - Card/container surfaces
#define COLOR_ITEM_BORDER 0x2945  // #262C3A - Slate/dark-grey outlines
#define COLOR_SELECTED 0xFD60     // #E8B564 - Brass accent for selection
#define COLOR_ACCENT 0xFD60       // #E8B564 - High-contrast accent
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Muted text for secondary info

SettingsScreen::SettingsScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr)
    : display(tft),
      touch(touchMgr),
      settings(settingsMgr),
      needsExit(false),
      forceFullRedraw(true),
      screenIsDirty(true),
      scrollY(0),
      dragging(false),
      lastTouchY(0) {
}

void SettingsScreen::begin() {
    DEBUG_LOG("SettingsScreen initialized");
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollY = 0;
    dragging = false;
}

void SettingsScreen::reset() {
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollY = 0;
    dragging = false;
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void SettingsScreen::update() {
    // Update scroll handling
    updateScroll();
}

// ============================================================================
// EXPERIMENTAL SCROLL FUNCTION
// ============================================================================

void SettingsScreen::updateScroll() {
    TouchState state = touch->getState();
    TouchPoint currentTouch = touch->getPoint();
    
    if (state == TOUCH_DOWN || state == TOUCH_MOVE) {
        if (currentTouch.valid) {
            if (!dragging) {
                // Start drag
                dragging = true;
                lastTouchY = currentTouch.y;
            } else {
                // Continue drag - calculate delta
                int deltaY = currentTouch.y - lastTouchY;
                scrollY += deltaY;
                
                // Apply scroll limits
                int maxScroll = 0;  // Top limit
                int minScroll = -(MENU_ITEM_HEIGHT * MENU_ITEM_COUNT - SCREEN_HEIGHT + 60);  // Bottom limit (60 for header)
                
                if (scrollY > maxScroll) scrollY = maxScroll;
                if (scrollY < minScroll) scrollY = minScroll;
                
                lastTouchY = currentTouch.y;
                screenIsDirty = true;  // Trigger redraw when scrolling
            }
        }
    } else {
        // Touch released (TOUCH_IDLE or TOUCH_UP)
        if (dragging) {
            dragging = false;
        }
    }
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void SettingsScreen::draw() {
    // THE STANDARD: Unified render pattern
    if (forceFullRedraw) {
        display->fillScreen(COLOR_BACKGROUND);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;  // Force initial data draw
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

// ============================================================================
// STATIC UI DRAWING - Headers (non-scrolling elements)
// ============================================================================

void SettingsScreen::drawStaticUI() {
    // Header bar (fixed, doesn't scroll)
    display->fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_HEADER);
    display->setTextColor(COLOR_TEXT, COLOR_HEADER);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Settings", SCREEN_WIDTH / 2, 10);
    
    // Back button indicator with accent color
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->setTextColor(COLOR_ACCENT, COLOR_BACKGROUND);
    display->drawString("< Back", 5, 45);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

// ============================================================================
// DYNAMIC DATA DRAWING - Scrollable menu items
// ============================================================================

void SettingsScreen::drawDynamicData() {
    // Clear the scrollable area (below header)
    display->fillRect(0, 60, SCREEN_WIDTH, SCREEN_HEIGHT - 60, COLOR_BACKGROUND);
    
    // Draw menu items with scrollY offset applied
    const char* menuItems[] = {
        "WiFi Settings",
        "Display Brightness",
        "Touch Calibration",
        "Password Manager",
        "Auto-Lock Timeout",
        "Sound Settings",
        "Backup & Restore",
        "Screen Orientation",    // NEW: Orientation option
        "Factory Reset",
        "About Device",
        "Exit Settings"
    };
    
    display->setTextSize(1);
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int itemY = 70 + (i * MENU_ITEM_HEIGHT) + scrollY;  // Apply scrollY offset
        
        // Only draw if visible on screen
        if (itemY + MENU_ITEM_HEIGHT > 60 && itemY < SCREEN_HEIGHT) {
            // Draw item background
            display->fillRect(10, itemY, SCREEN_WIDTH - 20, MENU_ITEM_HEIGHT - 5, COLOR_ITEM_BG);
            display->drawRect(10, itemY, SCREEN_WIDTH - 20, MENU_ITEM_HEIGHT - 5, COLOR_ITEM_BORDER);
            
            // Draw item text
            display->setTextColor(COLOR_TEXT, COLOR_ITEM_BG);
            display->setTextDatum(ML_DATUM);  // Middle-Left
            display->drawString(menuItems[i], 20, itemY + (MENU_ITEM_HEIGHT - 5) / 2);
            
            // Draw arrow indicator
            display->setTextDatum(MR_DATUM);  // Middle-Right
            display->drawString(">", SCREEN_WIDTH - 20, itemY + (MENU_ITEM_HEIGHT - 5) / 2);
        }
    }
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void SettingsScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("Settings: Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button (fixed position, no scroll offset)
    if (point.y >= 40 && point.y <= 60 && point.x <= 60) {
        DEBUG_LOG("Settings: Back button pressed");
        needsExit = true;
        return;
    }
    
    // Check menu items (accounting for scroll)
    int menuItem = getMenuItemAtPosition(point.x, point.y);
    if (menuItem >= 0) {
        DEBUG_LOGF("Settings: Menu item %d selected\n", menuItem);
        
        // Handle menu item selection
        switch (menuItem) {
            case 7:  // Screen Orientation
                DEBUG_LOG("Navigate to Orientation screen");
                // TODO: Navigate to orientation screen via ScreenManager
                break;
            case 8:  // Factory Reset
                DEBUG_LOG("Navigate to Factory Reset screen");
                // TODO: Navigate to factory reset screen
                break;
            case 10:  // Exit Settings
                needsExit = true;
                break;
            default:
                DEBUG_LOGF("Menu item %d not yet implemented\n", menuItem);
                break;
        }
        
        screenIsDirty = true;
    }
}

// ============================================================================
// MENU ITEM HIT DETECTION
// ============================================================================

int SettingsScreen::getMenuItemAtPosition(int16_t x, int16_t y) {
    // Only check if touch is in menu area (below back button)
    if (y < 60) return -1;
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int itemY = 70 + (i * MENU_ITEM_HEIGHT) + scrollY;  // Apply scrollY offset
        
        if (x >= 10 && x <= SCREEN_WIDTH - 10 &&
            y >= itemY && y <= itemY + MENU_ITEM_HEIGHT - 5) {
            return i;
        }
    }
    
    return -1;  // No item hit
}
