#include "AboutScreen.h"

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata

AboutScreen::AboutScreen(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      needsExit(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void AboutScreen::begin() {
    Serial.println("[ABOUT] Screen initialized");
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AboutScreen::reset() {
    needsExit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AboutScreen::update() {
    // No continuous updates needed
}

void AboutScreen::draw() {
    if (forceFullRedraw) {
        tft->fillScreen(COLOR_INK);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

void AboutScreen::drawStaticUI() {
    // Status bar (24px)
    tft->fillRect(0, 0, SCREEN_WIDTH, 24, COLOR_INK);
    tft->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // Header (32px)
    tft->fillRect(0, 24, SCREEN_WIDTH, 32, COLOR_INK);
    
    // Back button
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 16, 32);
    
    // Title
    tft->setTextDatum(TC_DATUM);
    tft->drawString("About", SCREEN_WIDTH / 2, 32);
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    tft->setTextDatum(TL_DATUM);
    
    Serial.println("[ABOUT] Static UI drawn");
}

void AboutScreen::drawDynamicData() {
    // Clear content area
    tft->fillRect(0, HEADER_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_HEIGHT, COLOR_INK);
    
    int y = HEADER_HEIGHT + 30;
    
    // Project logo/name
    tft->setTextColor(COLOR_BRASS, COLOR_INK);
    tft->setTextSize(3);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("SecureKey", SCREEN_WIDTH / 2, y);
    y += 35;
    
    // Version
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(1);
    tft->drawString("Version 1.0.0", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT + 10;
    
    // Separator line
    tft->drawFastHLine(SIDE_MARGIN, y, SCREEN_WIDTH - (SIDE_MARGIN * 2), COLOR_LINE);
    y += 15;
    
    // Description
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(1);
    tft->setTextDatum(TC_DATUM);
    
    tft->drawString("A secure password manager", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT;
    tft->drawString("for embedded devices.", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT + 8;
    
    tft->drawString("Featuring offline-first", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT;
    tft->drawString("encrypted storage, intuitive", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT;
    tft->drawString("touch interface, and modern", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT;
    tft->drawString("design system aesthetics.", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT + 15;
    
    // Separator line
    tft->drawFastHLine(SIDE_MARGIN, y, SCREEN_WIDTH - (SIDE_MARGIN * 2), COLOR_LINE);
    y += 15;
    
    // Credits
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(1);
    tft->drawString("Created by", SCREEN_WIDTH / 2, y);
    y += LINE_HEIGHT + 5;
    
    tft->setTextColor(COLOR_BRASS, COLOR_INK);
    tft->setTextSize(2);
    tft->drawString("Master", SCREEN_WIDTH / 2, y);
    y += 25;
    
    // Copyright
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(1);
    tft->drawString("(c) 2024 All Rights Reserved", SCREEN_WIDTH / 2, y);
    
    tft->setTextDatum(TL_DATUM);
}

void AboutScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[ABOUT] Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[ABOUT] Back button pressed");
        needsExit = true;
        return;
    }
}

bool AboutScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}
