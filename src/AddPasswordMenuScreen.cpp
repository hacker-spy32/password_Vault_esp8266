#include "AddPasswordMenuScreen.h"

// ============================================================================
// DESIGN SYSTEM - EXACT COLOR TOKENS (RGB565 format for TFT)
// ============================================================================
#define COLOR_INK 0x0841          // #0A0C10 - Base background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Danger/cancel actions

// ============================================================================
// LAYOUT CONSTANTS - EXACT PIXEL MEASUREMENTS
// ============================================================================
#define STATUS_BAR_HEIGHT 24      // Top status bar (brand + system icons)
#define HEADER_HEIGHT 32          // Navigation header
#define TOTAL_TOP_HEIGHT 56       // Status bar + Header (24 + 32)

#define CARD_BORDER_RADIUS 10     // Rounded corners for cards (9-11px range)
#define ROW_HEIGHT 72             // Height of each menu row
#define ROW_SPACING 12            // Gap between rows
#define SIDE_MARGIN 16            // Left/right screen margins
#define CONTENT_START_Y 64        // Where content begins (after header + margin)

AddPasswordMenuScreen::AddPasswordMenuScreen(TFT_eSPI* tft, TouchManager* touchMgr)
    : display(tft),
      touch(touchMgr),
      needsExit(false),
      selectedMethod(METHOD_NONE),
      forceFullRedraw(true),
      screenIsDirty(true) {
}

void AddPasswordMenuScreen::begin() {
    DEBUG_LOG("AddPasswordMenuScreen initialized");
    needsExit = false;
    selectedMethod = METHOD_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AddPasswordMenuScreen::reset() {
    needsExit = false;
    selectedMethod = METHOD_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void AddPasswordMenuScreen::update() {
    // No continuous updates needed
}

void AddPasswordMenuScreen::draw() {
    // THE STANDARD: Unified render pattern
    if (forceFullRedraw) {
        display->fillScreen(COLOR_INK);  // Deep black background
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

void AddPasswordMenuScreen::drawStaticUI() {
    // ========================================================================
    // STATUS BAR - 24px height at top
    // ========================================================================
    display->fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_INK);
    
    // PassGuard brand dot (left side) - small brass circle
    display->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // System status icons (right side) - placeholder WiFi indicator
    display->setTextColor(COLOR_MUTED, COLOR_INK);
    display->setTextSize(1);
    display->setTextDatum(TR_DATUM);
    display->drawString("WiFi", SCREEN_WIDTH - 12, 8);
    
    // ========================================================================
    // HEADER - 32px height with border-bottom separator
    // ========================================================================
    display->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_INK);
    
    // Left-pointing back arrow (crisp < symbol)
    display->setTextColor(COLOR_IVORY, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    display->drawString("<", 16, STATUS_BAR_HEIGHT + 8);
    
    // Title 'Add Password' - Space Grotesk style (centered)
    display->setTextColor(COLOR_IVORY, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Add Password", SCREEN_WIDTH / 2, STATUS_BAR_HEIGHT + 8);
    
    // Border-bottom separator (1px line at bottom of header)
    display->drawFastHLine(0, TOTAL_TOP_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    // Reset text datum
    display->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPasswordMenu - Static UI drawn (Pixel-Perfect)");
}

void AddPasswordMenuScreen::drawDynamicData() {
    // Clear content area (below header)
    display->fillRect(0, TOTAL_TOP_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - TOTAL_TOP_HEIGHT, COLOR_INK);
    
    DEBUG_LOG("AddPasswordMenu - Drawing list rows with exact spacing...");
    
    int cardWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);  // Full width minus margins
    
    // ========================================================================
    // ROW 1: "Add Manually"
    // ========================================================================
    int row1Y = CONTENT_START_Y;
    
    // Draw card container with Surface color and Line border
    display->fillRoundRect(SIDE_MARGIN, row1Y, cardWidth, ROW_HEIGHT, CARD_BORDER_RADIUS, COLOR_SURFACE);
    display->drawRoundRect(SIDE_MARGIN, row1Y, cardWidth, ROW_HEIGHT, CARD_BORDER_RADIUS, COLOR_LINE);
    
    // Title text - primary, larger
    display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    display->drawString("Add Manually", SIDE_MARGIN + 16, row1Y + 14);
    
    // Subtitle text - JetBrains Mono style (smaller, muted)
    display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->drawString("Enter credentials directly", SIDE_MARGIN + 16, row1Y + 38);
    
    // Right chevron icon (trailing edge)
    display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    display->setTextSize(2);
    display->setTextDatum(TR_DATUM);
    display->drawString(">", SCREEN_WIDTH - SIDE_MARGIN - 16, row1Y + 26);
    
    DEBUG_LOGF("Row 1 rendered at Y=%d\n", row1Y);
    
    // ========================================================================
    // ROW 2: "Web UI" (Server Control)
    // ========================================================================
    int row2Y = row1Y + ROW_HEIGHT + ROW_SPACING;
    
    // Draw card container
    display->fillRoundRect(SIDE_MARGIN, row2Y, cardWidth, ROW_HEIGHT, CARD_BORDER_RADIUS, COLOR_SURFACE);
    display->drawRoundRect(SIDE_MARGIN, row2Y, cardWidth, ROW_HEIGHT, CARD_BORDER_RADIUS, COLOR_LINE);
    
    // Title text
    display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    display->drawString("Web UI", SIDE_MARGIN + 16, row2Y + 14);
    
    // Subtitle text
    display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->drawString("Control WiFi hotspot", SIDE_MARGIN + 16, row2Y + 38);
    
    // Right chevron icon
    display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    display->setTextSize(2);
    display->setTextDatum(TR_DATUM);
    display->drawString(">", SCREEN_WIDTH - SIDE_MARGIN - 16, row2Y + 26);
    
    DEBUG_LOGF("Row 2 rendered at Y=%d\n", row2Y);
    
    // Reset text datum
    display->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPasswordMenu - Pixel-perfect rendering complete");
}

void AddPasswordMenuScreen::onTouchEvent(const TouchPoint& point) {
    DEBUG_LOGF("AddPasswordMenu: Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        DEBUG_LOG("AddPasswordMenu: Back button pressed");
        needsExit = true;
        return;
    }
    
    // Check menu items
    int item = getMenuItemAtPosition(point.x, point.y);
    if (item >= 0) {
        selectedMethod = (AddPasswordMethod)item;
        DEBUG_LOGF("AddPasswordMenu: Selected method %d\n", selectedMethod);
    }
}

bool AddPasswordMenuScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    // Back arrow touch zone (left portion of header)
    return (y >= STATUS_BAR_HEIGHT && y <= TOTAL_TOP_HEIGHT && x <= 60);
}

int AddPasswordMenuScreen::getMenuItemAtPosition(int16_t x, int16_t y) const {
    int cardWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    // Row 1: Add Manually
    int row1Y = CONTENT_START_Y;
    if (x >= SIDE_MARGIN && x <= SIDE_MARGIN + cardWidth &&
        y >= row1Y && y <= row1Y + ROW_HEIGHT) {
        return METHOD_MANUAL;
    }
    
    // Row 2: Web UI Control
    int row2Y = row1Y + ROW_HEIGHT + ROW_SPACING;
    if (x >= SIDE_MARGIN && x <= SIDE_MARGIN + cardWidth &&
        y >= row2Y && y <= row2Y + ROW_HEIGHT) {
        return METHOD_WEBUI_CONTROL;
    }
    
    return METHOD_NONE;
}
