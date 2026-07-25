#include "AdvancedOptionsScreen.h"
#include "SettingsManager.h"
#include "FloatingScrollButton.h"

// Design System Colors - Unified Dark Theme
#define COLOR_BACKGROUND 0x0841   // #0A0C10 - Pitch black background
#define COLOR_TEXT 0xFFDE         // #F4EFE4 - Clean white typography
#define COLOR_HEADER 0x18C3       // #141821 - Dark surface for header
#define COLOR_ITEM_BG 0x18C3      // #141821 - Card/container surfaces
#define COLOR_ITEM_BORDER 0x2945  // #262C3A - Slate/dark-grey outlines
#define COLOR_ITEM_DIAGNOSTIC 0x2945   // Diagnostic items (same as border)
#define COLOR_ITEM_CONFIG 0x18C3       // Config items (same as surface)
#define COLOR_ITEM_DANGER 0x7800       // Danger items (dark red #780000)
#define COLOR_ACCENT 0xFD60       // #E8B564 - High-contrast accent
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Muted secondary text

AdvancedOptionsScreen::AdvancedOptionsScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr)
    : display(tft),
      touch(touchMgr),
      settings(settingsMgr),
      needsExit(false),
      selectedAction(ACTION_NONE),
      forceFullRedraw(true),
      screenIsDirty(true),
      scrollOffset(0) {
}

void AdvancedOptionsScreen::begin() {
    DEBUG_LOG("AdvancedOptionsScreen initialized");
    needsExit = false;
    selectedAction = ACTION_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
}

void AdvancedOptionsScreen::reset() {
    needsExit = false;
    selectedAction = ACTION_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void AdvancedOptionsScreen::update() {
    // No continuous updates needed - FAB handles scroll in ScreenManager
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void AdvancedOptionsScreen::draw() {
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

void AdvancedOptionsScreen::drawStaticUI() {
    // Header bar (fixed, doesn't scroll)
    display->fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_HEADER);
    display->setTextColor(COLOR_TEXT, COLOR_HEADER);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Advanced Options", SCREEN_WIDTH / 2, 10);
    
    // Back button indicator with accent color
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->setTextColor(COLOR_ACCENT, COLOR_BACKGROUND);
    display->drawString("< Back", 5, 45);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
    
    DEBUG_LOG("AdvancedOptions - Static UI drawn");
}

// ============================================================================
// DYNAMIC DATA DRAWING - Scrollable menu items
// ============================================================================

void AdvancedOptionsScreen::drawDynamicData() {
    // Clear the scrollable area (below header)
    display->fillRect(0, 60, SCREEN_WIDTH, SCREEN_HEIGHT - 60, COLOR_BACKGROUND);
    
    display->setTextSize(1);
    
    // Calculate Y offset based on scroll offset (discrete steps)
    int scrollY = -(scrollOffset * MENU_ITEM_HEIGHT);
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int itemY = 70 + (i * MENU_ITEM_HEIGHT) + scrollY;  // Apply scroll offset
        
        // Only draw if visible on screen
        if (itemY + MENU_ITEM_HEIGHT > 60 && itemY < SCREEN_HEIGHT) {
            // Choose color based on item type
            uint16_t itemColor;
            if (i == 0) {
                itemColor = COLOR_ITEM_DIAGNOSTIC;  // Touch Test (green)
            } else if (i >= 3) {
                itemColor = COLOR_ITEM_DANGER;      // Reset/Factory (red)
            } else {
                itemColor = COLOR_ITEM_CONFIG;      // Calibration/Orientation (blue)
            }
            
            // Draw item background
            display->fillRect(10, itemY, SCREEN_WIDTH - 20, MENU_ITEM_HEIGHT - 5, itemColor);
            display->drawRect(10, itemY, SCREEN_WIDTH - 20, MENU_ITEM_HEIGHT - 5, COLOR_ITEM_BORDER);
            
            // Draw item text
            display->setTextColor(COLOR_TEXT, itemColor);
            display->setTextDatum(ML_DATUM);  // Middle-Left
            display->drawString(getMenuItemLabel(i), 20, itemY + (MENU_ITEM_HEIGHT - 5) / 2);
            
            // Draw arrow indicator
            display->setTextDatum(MR_DATUM);  // Middle-Right
            display->drawString(">", SCREEN_WIDTH - 20, itemY + (MENU_ITEM_HEIGHT - 5) / 2);
        }
    }
    
    display->setTextDatum(TL_DATUM);  // Reset to default
    
    DEBUG_LOG("AdvancedOptions - Dynamic data drawn");
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void AdvancedOptionsScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("AdvancedOptions: Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button (fixed position, no scroll offset)
    if (point.y >= 40 && point.y <= 60 && point.x <= 60) {
        DEBUG_LOG("AdvancedOptions: Back button pressed");
        needsExit = true;
        return;
    }
    
    // Check menu items (accounting for scroll)
    int menuItem = getMenuItemAtPosition(point.x, point.y);
    if (menuItem >= 0) {
        DEBUG_LOGF("AdvancedOptions: Menu item %d selected\n", menuItem);
        
        // Map menu item to action
        switch (menuItem) {
            case 0:
                selectedAction = ACTION_TOUCH_TEST;
                DEBUG_LOG("Action: Touch Test");
                break;
            case 1:
                selectedAction = ACTION_CALIBRATION;
                DEBUG_LOG("Action: Touch Calibration");
                break;
            case 2:
                selectedAction = ACTION_ORIENTATION;
                DEBUG_LOG("Action: Orientation");
                break;
            case 3:
                selectedAction = ACTION_RESET_CALIBRATION;
                DEBUG_LOG("Action: Reset Calibration");
                break;
            case 4:
                selectedAction = ACTION_FACTORY_RESET;
                DEBUG_LOG("Action: Factory Reset");
                break;
        }
        
        screenIsDirty = true;
    }
}

// ============================================================================
// MENU ITEM HIT DETECTION
// ============================================================================

int AdvancedOptionsScreen::getMenuItemAtPosition(int16_t x, int16_t y) {
    // Only check if touch is in menu area (below back button)
    if (y < 60) return -1;
    
    // Calculate Y offset based on scroll offset
    int scrollY = -(scrollOffset * MENU_ITEM_HEIGHT);
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int itemY = 70 + (i * MENU_ITEM_HEIGHT) + scrollY;  // Apply scroll offset
        
        if (x >= 10 && x <= SCREEN_WIDTH - 10 &&
            y >= itemY && y <= itemY + MENU_ITEM_HEIGHT - 5) {
            return i;
        }
    }
    
    return -1;  // No item hit
}

// ============================================================================
// MENU ITEM LABELS
// ============================================================================

const char* AdvancedOptionsScreen::getMenuItemLabel(int index) {
    switch (index) {
        case 0: return "Touch Test";
        case 1: return "Touch Calibration";
        case 2: return "Screen Orientation";
        case 3: return "Reset Calibration";
        case 4: return "Factory Reset";
        default: return "Unknown";
    }
}


// ============================================================================
// SCROLL HELPERS (Discrete Step Scrolling)
// ============================================================================

void AdvancedOptionsScreen::scrollDown() {
    int maxScroll = getMaxScrollOffset();
    
    if (scrollOffset < maxScroll) {
        scrollOffset++;
        screenIsDirty = true;
        DEBUG_LOGF("AdvancedOptions: Scrolled to offset %d\n", scrollOffset);
    } else {
        DEBUG_LOG("AdvancedOptions: Already at bottom");
    }
}

int AdvancedOptionsScreen::getMaxScrollOffset() const {
    // Calculate how many items can be scrolled
    // If all items fit on screen, max offset is 0
    int visibleHeight = SCREEN_HEIGHT - 70;  // Screen height minus header
    int totalHeight = MENU_ITEM_COUNT * MENU_ITEM_HEIGHT;
    
    if (totalHeight <= visibleHeight) {
        return 0;  // All items visible, no scroll needed
    }
    
    // Calculate how many full items need to scroll to show all content
    int maxOffset = (totalHeight - visibleHeight) / MENU_ITEM_HEIGHT;
    if ((totalHeight - visibleHeight) % MENU_ITEM_HEIGHT > 0) {
        maxOffset++;  // Round up if partial item
    }
    
    return maxOffset;
}
