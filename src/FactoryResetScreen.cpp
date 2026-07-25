#include "FactoryResetScreen.h"
#include "SettingsManager.h"

// Design System Colors - Unified Dark Theme
#define COLOR_BACKGROUND 0x0841   // #0A0C10 - Pitch black background
#define COLOR_TEXT 0xFFDE         // #F4EFE4 - Clean white typography
#define COLOR_ERROR 0xF800        // Red for errors
#define COLOR_WARNING 0xFC00      // Orange/yellow for warnings
#define COLOR_BUTTON_CANCEL 0x18C3   // #141821 - Dark surface
#define COLOR_BUTTON_CANCEL_BORDER 0x2945  // #262C3A - Slate outline
#define COLOR_BUTTON_CONFIRM 0x7800   // Dark red for danger action
#define COLOR_BUTTON_CONFIRM_BORDER 0xF800  // Bright red border
#define COLOR_ACCENT 0xFD60       // #E8B564 - Brass accent

FactoryResetScreen::FactoryResetScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr)
    : display(tft),
      touch(touchMgr),
      settings(settingsMgr),
      needsExit(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void FactoryResetScreen::begin() {
    DEBUG_LOG("FactoryResetScreen initialized");
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void FactoryResetScreen::reset() {
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void FactoryResetScreen::update() {
    // No time-based updates needed for this screen
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void FactoryResetScreen::draw() {
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
// STATIC UI DRAWING - Headers, buttons, labels
// ============================================================================

void FactoryResetScreen::drawStaticUI() {
    // Primary Heading
    display->setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);  // Top-Center
    display->drawString("Factory Reset", SCREEN_WIDTH / 2, 30);
    
    // Warning Icon (Triangle with !)
    display->drawTriangle(SCREEN_WIDTH / 2, 65, 
                         SCREEN_WIDTH / 2 - 25, 105,
                         SCREEN_WIDTH / 2 + 25, 105, COLOR_WARNING);
    display->setTextSize(3);
    display->drawString("!", SCREEN_WIDTH / 2, 75);
    
    // Description
    display->setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    display->setTextSize(1);
    display->drawString("This will reset", SCREEN_WIDTH / 2, 120);
    display->drawString("all settings/passwords", SCREEN_WIDTH / 2, 135);
    
    // Additional warning
    display->setTextColor(COLOR_WARNING, COLOR_BACKGROUND);
    display->drawString("This cannot be undone!", SCREEN_WIDTH / 2, 160);
    
    // Cancel Button (Bottom Left)
    display->fillRect(CANCEL_BTN_X, CANCEL_BTN_Y, CANCEL_BTN_W, CANCEL_BTN_H, COLOR_BUTTON_CANCEL);
    display->drawRect(CANCEL_BTN_X, CANCEL_BTN_Y, CANCEL_BTN_W, CANCEL_BTN_H, COLOR_BUTTON_CANCEL_BORDER);
    display->setTextColor(TFT_WHITE, COLOR_BUTTON_CANCEL);
    display->setTextSize(1);
    display->drawString("Cancel", CANCEL_BTN_X + CANCEL_BTN_W / 2, CANCEL_BTN_Y + CANCEL_BTN_H / 2);
    
    // Factory Reset Button (Bottom Right) - RED
    display->fillRect(RESET_BTN_X, RESET_BTN_Y, RESET_BTN_W, RESET_BTN_H, TFT_MAROON);
    display->drawRect(RESET_BTN_X, RESET_BTN_Y, RESET_BTN_W, RESET_BTN_H, COLOR_ERROR);
    display->setTextColor(TFT_WHITE, TFT_MAROON);
    display->drawString("Factory", RESET_BTN_X + RESET_BTN_W / 2, RESET_BTN_Y + RESET_BTN_H / 2 - 5);
    display->drawString("Reset", RESET_BTN_X + RESET_BTN_W / 2, RESET_BTN_Y + RESET_BTN_H / 2 + 5);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

void FactoryResetScreen::drawDynamicData() {
    // No dynamic content for this screen
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void FactoryResetScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("FactoryReset: Touch at (%d,%d)\n", point.x, point.y);
    
    if (isCancelButtonPressed(point.x, point.y)) {
        DEBUG_LOG("FactoryReset: Cancel pressed");
        needsExit = true;  // Return to settings without action
    }
    else if (isResetButtonPressed(point.x, point.y)) {
        DEBUG_LOG("FactoryReset: Factory Reset pressed");
        performFactoryReset();
        needsExit = true;  // Return to settings after action
    }
}

// ============================================================================
// BUTTON HIT DETECTION
// ============================================================================

bool FactoryResetScreen::isCancelButtonPressed(int16_t x, int16_t y) {
    return (x >= CANCEL_BTN_X && x <= (CANCEL_BTN_X + CANCEL_BTN_W) &&
            y >= CANCEL_BTN_Y && y <= (CANCEL_BTN_Y + CANCEL_BTN_H));
}

bool FactoryResetScreen::isResetButtonPressed(int16_t x, int16_t y) {
    return (x >= RESET_BTN_X && x <= (RESET_BTN_X + RESET_BTN_W) &&
            y >= RESET_BTN_Y && y <= (RESET_BTN_Y + RESET_BTN_H));
}

// ============================================================================
// ACTION HANDLER
// ============================================================================

void FactoryResetScreen::performFactoryReset() {
    DEBUG_LOG("FactoryReset: Performing factory reset...");
    
    // Reset ALL settings and data to factory defaults
    settings->factoryReset();
    
    // Save to persistent storage
    settings->save();
    
    DEBUG_LOG("FactoryReset: Factory reset complete");
}
