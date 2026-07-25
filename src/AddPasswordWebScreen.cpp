#include "AddPasswordWebScreen.h"

// Design System Colors - Unified Dark Theme
#define COLOR_BACKGROUND 0x0841   // #0A0C10 - Pitch black background
#define COLOR_TEXT 0xFFDE         // #F4EFE4 - Clean white typography
#define COLOR_HEADER 0x18C3       // #141821 - Dark surface for header
#define COLOR_INFO 0xFD60         // #E8B564 - Brass accent for info
#define COLOR_ACCENT 0xFD60       // #E8B564 - High-contrast accent
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Muted secondary text

AddPasswordWebScreen::AddPasswordWebScreen(TFT_eSPI* tft, TouchManager* touchMgr)
    : display(tft),
      touch(touchMgr),
      needsExit(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void AddPasswordWebScreen::begin() {
    DEBUG_LOG("AddPasswordWebScreen initialized");
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AddPasswordWebScreen::reset() {
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AddPasswordWebScreen::update() {
    // No continuous updates needed
}

void AddPasswordWebScreen::draw() {
    // THE STANDARD: Unified render pattern
    if (forceFullRedraw) {
        display->fillScreen(COLOR_BACKGROUND);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

void AddPasswordWebScreen::drawStaticUI() {
    // Header bar
    display->fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER);
    display->setTextColor(TFT_BLACK, COLOR_HEADER);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Web UI Import", SCREEN_WIDTH / 2, 10);
    
    // Back button
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->drawString("< Back", 5, 45);
    
    display->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPasswordWeb - Static UI drawn");
}

void AddPasswordWebScreen::drawDynamicData() {
    // Clear content area
    display->fillRect(0, 50, SCREEN_WIDTH, SCREEN_HEIGHT - 50, COLOR_BACKGROUND);
    
    // Draw placeholder/empty state message
    display->setTextColor(COLOR_INFO, COLOR_BACKGROUND);
    display->setTextSize(1);
    display->setTextDatum(TC_DATUM);
    
    int centerY = SCREEN_HEIGHT / 2 - 20;
    
    display->drawString("Web UI Import", SCREEN_WIDTH / 2, centerY);
    display->drawString("Coming Soon", SCREEN_WIDTH / 2, centerY + 15);
    
    display->setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    display->drawString("This feature will allow you", SCREEN_WIDTH / 2, centerY + 40);
    display->drawString("to import passwords from", SCREEN_WIDTH / 2, centerY + 55);
    display->drawString("a web-based interface.", SCREEN_WIDTH / 2, centerY + 70);
    
    display->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPasswordWeb - Dynamic data drawn");
}

void AddPasswordWebScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("AddPasswordWeb: Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        DEBUG_LOG("AddPasswordWeb: Back button pressed");
        needsExit = true;
        return;
    }
}

bool AddPasswordWebScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= HEADER_HEIGHT && y <= HEADER_HEIGHT + 20 && x <= 60);
}
