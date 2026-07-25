#include "LockScreen.h"

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata

LockScreen::LockScreen(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      forceFullRedraw(true),
      screenIsDirty(true),
      unlockRequested(false) {
}

void LockScreen::begin() {
    Serial.println("[LOCK_SCREEN] Screen initialized (Static Mode)");
    unlockRequested = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void LockScreen::reset() {
    Serial.println("[LOCK_SCREEN] Reset called (Static Mode)");
    unlockRequested = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void LockScreen::update() {
    // Static lock screen - no dynamic animation updates needed
}

void LockScreen::draw() {
    if (forceFullRedraw) {
        tft->fillScreen(COLOR_INK);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = false;
    }
}

void LockScreen::drawStaticUI() {
    // Draw status bar dot
    tft->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // Draw lock icon (simple padlock outline)
    int lockX = SCREEN_WIDTH / 2;
    int lockY = 65;
    
    // Lock body (rectangle)
    tft->fillRoundRect(lockX - 20, lockY, 40, 50, 6, COLOR_SURFACE);
    tft->drawRoundRect(lockX - 20, lockY, 40, 50, 6, COLOR_LINE);
    
    // Lock shackle (arc)
    tft->drawCircle(lockX, lockY - 10, 15, COLOR_LINE);
    tft->drawCircle(lockX, lockY - 10, 16, COLOR_LINE);
    tft->fillRect(lockX - 17, lockY - 10, 34, 12, COLOR_INK);  // Bottom half cover
    
    // Keyhole
    tft->fillCircle(lockX, lockY + 20, 5, COLOR_LINE);
    tft->fillRect(lockX - 2, lockY + 20, 4, 15, COLOR_LINE);
    
    // Draw "Swipe to Unlock" text
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("Swipe to Unlock", SCREEN_WIDTH / 2, 140);
    
    // Draw static arrow indicators (> > > >)
    int baseX = 95;
    int spacing = 35;
    for (int i = 0; i < 4; i++) {
        uint16_t color = (i < 2) ? COLOR_MUTED : COLOR_BRASS;
        tft->setTextColor(color, COLOR_INK);
        tft->setTextSize(2);
        tft->setTextDatum(TL_DATUM);
        tft->drawString(">", baseX + (i * spacing), 175);
    }
    
    // Instruction text
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(1);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("Tap anywhere to continue", SCREEN_WIDTH / 2, 210);
    
    tft->setTextDatum(TL_DATUM);  // Reset
    
    Serial.println("[LOCK_SCREEN] Static UI drawn (Clean static image)");
}

void LockScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[LOCK_SCREEN] Touch detected at (%d,%d)\n", point.x, point.y);
    Serial.println("[LOCK_SCREEN] Unlock requested");
    unlockRequested = true;
}
