#include "CalibrationResetScreen.h"
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

CalibrationResetScreen::CalibrationResetScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr)
    : display(tft),
      touch(touchMgr),
      settings(settingsMgr),
      needsExit(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void CalibrationResetScreen::begin() {
    DEBUG_LOG("CalibrationResetScreen initialized");
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void CalibrationResetScreen::reset() {
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void CalibrationResetScreen::update() {
    // No time-based updates needed for this screen
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void CalibrationResetScreen::draw() {
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

void CalibrationResetScreen::drawStaticUI() {
    // Primary Heading
    display->setTextColor(COLOR_ERROR, COLOR_BACKGROUND);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);  // Top-Center
    display->drawString("Reset Calibration", SCREEN_WIDTH / 2, 20);
    display->drawString("to Factory", SCREEN_WIDTH / 2, 40);
    
    // Warning Icon (Triangle with !)
    display->drawTriangle(SCREEN_WIDTH / 2, 75, 
                         SCREEN_WIDTH / 2 - 25, 115,
                         SCREEN_WIDTH / 2 + 25, 115, COLOR_WARNING);
    display->setTextSize(3);
    display->drawString("!", SCREEN_WIDTH / 2, 85);
    
    // Description
    display->setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    display->setTextSize(1);
    display->drawString("This will reset touch", SCREEN_WIDTH / 2, 130);
    display->drawString("calibration to defaults", SCREEN_WIDTH / 2, 145);
    
    // Cancel Button (Bottom Left)
    display->fillRect(CANCEL_BTN_X, CANCEL_BTN_Y, CANCEL_BTN_W, CANCEL_BTN_H, COLOR_BUTTON_CANCEL);
    display->drawRect(CANCEL_BTN_X, CANCEL_BTN_Y, CANCEL_BTN_W, CANCEL_BTN_H, COLOR_BUTTON_CANCEL_BORDER);
    display->setTextColor(TFT_WHITE, COLOR_BUTTON_CANCEL);
    display->setTextSize(1);
    display->drawString("Cancel", CANCEL_BTN_X + CANCEL_BTN_W / 2, CANCEL_BTN_Y + CANCEL_BTN_H / 2);
    
    // Reset Calibration Button (Bottom Right) - RED
    display->fillRect(RESET_BTN_X, RESET_BTN_Y, RESET_BTN_W, RESET_BTN_H, TFT_MAROON);
    display->drawRect(RESET_BTN_X, RESET_BTN_Y, RESET_BTN_W, RESET_BTN_H, COLOR_ERROR);
    display->setTextColor(TFT_WHITE, TFT_MAROON);
    display->drawString("Reset", RESET_BTN_X + RESET_BTN_W / 2, RESET_BTN_Y + RESET_BTN_H / 2 - 5);
    display->drawString("Calibration", RESET_BTN_X + RESET_BTN_W / 2, RESET_BTN_Y + RESET_BTN_H / 2 + 5);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

void CalibrationResetScreen::drawDynamicData() {
    // No dynamic content for this screen
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void CalibrationResetScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("CalibrationReset: Touch at (%d,%d)\n", point.x, point.y);
    
    if (isCancelButtonPressed(point.x, point.y)) {
        DEBUG_LOG("CalibrationReset: Cancel pressed");
        needsExit = true;  // Return to settings without action
    }
    else if (isResetButtonPressed(point.x, point.y)) {
        DEBUG_LOG("CalibrationReset: Reset Calibration pressed");
        performReset();
        needsExit = true;  // Return to settings after action
    }
}

// ============================================================================
// BUTTON HIT DETECTION
// ============================================================================

bool CalibrationResetScreen::isCancelButtonPressed(int16_t x, int16_t y) {
    return (x >= CANCEL_BTN_X && x <= (CANCEL_BTN_X + CANCEL_BTN_W) &&
            y >= CANCEL_BTN_Y && y <= (CANCEL_BTN_Y + CANCEL_BTN_H));
}

bool CalibrationResetScreen::isResetButtonPressed(int16_t x, int16_t y) {
    return (x >= RESET_BTN_X && x <= (RESET_BTN_X + RESET_BTN_W) &&
            y >= RESET_BTN_Y && y <= (RESET_BTN_Y + RESET_BTN_H));
}

// ============================================================================
// ACTION HANDLER
// ============================================================================

void CalibrationResetScreen::performReset() {
    DEBUG_LOG("CalibrationReset: Performing calibration reset...");
    
    // Reset calibration to factory defaults
    settings->resetCalibration();
    
    // Save to persistent storage
    settings->save();
    
    DEBUG_LOG("CalibrationReset: Reset complete");
}
