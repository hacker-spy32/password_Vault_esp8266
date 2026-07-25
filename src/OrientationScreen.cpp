#include "OrientationScreen.h"
#include "SettingsManager.h"
#include "Hardware.h"

// Design System Colors - Unified Dark Theme
#define COLOR_BACKGROUND 0x0841   // #0A0C10 - Pitch black background
#define COLOR_TEXT 0xFFDE         // #F4EFE4 - Clean white typography
#define COLOR_HEADER 0x18C3       // #141821 - Dark surface for header
#define COLOR_BUTTON_BG 0x18C3    // #141821 - Button background
#define COLOR_BUTTON_BORDER 0x2945  // #262C3A - Slate/dark-grey outlines
#define COLOR_BUTTON_SELECTED 0xFD60  // #E8B564 - Brass accent for selection
#define COLOR_BUTTON_TEXT 0xFFDE  // #F4EFE4 - Clean white text
#define COLOR_ACCENT 0xFD60       // #E8B564 - High-contrast accent
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Muted secondary text

OrientationScreen::OrientationScreen(TFT_eSPI* tft, TouchManager* touchMgr, SettingsManager* settingsMgr)
    : display(tft),
      touch(touchMgr),
      settings(settingsMgr),
      needsExit(false),
      orientationWasChanged(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void OrientationScreen::begin() {
    DEBUG_LOG("OrientationScreen initialized");
    needsExit = false;
    orientationWasChanged = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void OrientationScreen::reset() {
    needsExit = false;
    orientationWasChanged = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void OrientationScreen::update() {
    // No continuous updates needed for this screen
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void OrientationScreen::draw() {
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
// STATIC UI DRAWING - Headers and buttons
// ============================================================================

void OrientationScreen::drawStaticUI() {
    // Header bar
    display->fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER);
    display->setTextColor(COLOR_TEXT, COLOR_HEADER);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Screen Orientation", SCREEN_WIDTH / 2, 10);
    
    // Back button indicator with accent color
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->setTextColor(COLOR_ACCENT, COLOR_BACKGROUND);
    display->drawString("< Back", 5, BACK_BTN_Y);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

// ============================================================================
// DYNAMIC DATA DRAWING - Option buttons
// ============================================================================

void OrientationScreen::drawDynamicData() {
    uint8_t currentOrientation = settings->getOrientation();
    
    // Calculate centered X position for buttons
    int buttonX = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
    
    // Draw Landscape button
    bool isLandscapeSelected = (currentOrientation == ORIENTATION_LANDSCAPE);
    uint16_t landscapeColor = isLandscapeSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_BG;
    
    display->fillRect(buttonX, LANDSCAPE_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, landscapeColor);
    display->drawRect(buttonX, LANDSCAPE_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_BUTTON_BORDER);
    
    display->setTextColor(COLOR_BUTTON_TEXT, landscapeColor);
    display->setTextSize(2);
    display->setTextDatum(MC_DATUM);  // Middle-Center
    display->drawString("Landscape", SCREEN_WIDTH / 2, LANDSCAPE_BTN_Y + BUTTON_HEIGHT / 2);
    
    // Draw Landscape Inverted button
    bool isLandscapeInvSelected = (currentOrientation == ORIENTATION_LANDSCAPE_INVERTED);
    uint16_t landscapeInvColor = isLandscapeInvSelected ? COLOR_BUTTON_SELECTED : COLOR_BUTTON_BG;
    
    display->fillRect(buttonX, LANDSCAPE_INV_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, landscapeInvColor);
    display->drawRect(buttonX, LANDSCAPE_INV_BTN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_BUTTON_BORDER);
    
    display->setTextColor(COLOR_BUTTON_TEXT, landscapeInvColor);
    display->setTextSize(2);
    display->setTextDatum(MC_DATUM);
    display->drawString("Landscape Inverted", SCREEN_WIDTH / 2, LANDSCAPE_INV_BTN_Y + BUTTON_HEIGHT / 2);
    
    // Draw indicator text
    display->setTextSize(1);
    display->setTextColor(COLOR_MUTED, COLOR_BACKGROUND);
    display->setTextDatum(TC_DATUM);
    display->drawString("Current selection highlighted", SCREEN_WIDTH / 2, 200);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void OrientationScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("Orientation: Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        DEBUG_LOG("Orientation: Back button pressed");
        needsExit = true;
        return;
    }
    
    // Check Landscape button
    if (isLandscapeButtonPressed(point.x, point.y)) {
        DEBUG_LOG("Orientation: Landscape selected");
        uint8_t currentOrientation = settings->getOrientation();
        
        if (currentOrientation != ORIENTATION_LANDSCAPE) {
            settings->setOrientation(ORIENTATION_LANDSCAPE);
            settings->save();
            orientationWasChanged = true;
            screenIsDirty = true;
            
            // Apply orientation change immediately
            tft.setRotation(ORIENTATION_LANDSCAPE);
            DEBUG_LOG("Display rotation set to Landscape (1)");
        }
        return;
    }
    
    // Check Landscape Inverted button
    if (isLandscapeInvertedButtonPressed(point.x, point.y)) {
        DEBUG_LOG("Orientation: Landscape Inverted selected");
        uint8_t currentOrientation = settings->getOrientation();
        
        if (currentOrientation != ORIENTATION_LANDSCAPE_INVERTED) {
            settings->setOrientation(ORIENTATION_LANDSCAPE_INVERTED);
            settings->save();
            orientationWasChanged = true;
            screenIsDirty = true;
            
            // Apply orientation change immediately
            tft.setRotation(ORIENTATION_LANDSCAPE_INVERTED);
            DEBUG_LOG("Display rotation set to Landscape Inverted (3)");
        }
        return;
    }
}

// ============================================================================
// BUTTON HIT DETECTION
// ============================================================================

bool OrientationScreen::isLandscapeButtonPressed(int16_t x, int16_t y) {
    int buttonX = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= LANDSCAPE_BTN_Y && y <= LANDSCAPE_BTN_Y + BUTTON_HEIGHT);
}

bool OrientationScreen::isLandscapeInvertedButtonPressed(int16_t x, int16_t y) {
    int buttonX = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
    return (x >= buttonX && x <= buttonX + BUTTON_WIDTH &&
            y >= LANDSCAPE_INV_BTN_Y && y <= LANDSCAPE_INV_BTN_Y + BUTTON_HEIGHT);
}

bool OrientationScreen::isBackButtonPressed(int16_t x, int16_t y) {
    return (y >= HEADER_HEIGHT && y <= HEADER_HEIGHT + 20 && x <= 60);
}
