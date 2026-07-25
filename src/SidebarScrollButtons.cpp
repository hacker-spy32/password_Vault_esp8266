#include "SidebarScrollButtons.h"

SidebarScrollButtons::SidebarScrollButtons(TFT_eSPI* display)
    : tft(display),
      upPressed(false),
      downPressed(false) {
}

void SidebarScrollButtons::begin() {
    upPressed = false;
    downPressed = false;
}

void SidebarScrollButtons::draw(int currentScrollOffset, int maxScrollOffset) {
    if (!tft) return;
    
    bool canScrollUp = (currentScrollOffset > 0);
    bool canScrollDown = (currentScrollOffset < maxScrollOffset);
    
    // Draw outer sidebar panel background & border
    tft->fillRoundRect(SIDEBAR_X, SIDEBAR_Y, SIDEBAR_WIDTH, SIDEBAR_HEIGHT, 8, COLOR_SURFACE);
    tft->drawRoundRect(SIDEBAR_X, SIDEBAR_Y, SIDEBAR_WIDTH, SIDEBAR_HEIGHT, 8, COLOR_LINE);
    
    // Middle divider line
    tft->drawFastHLine(SIDEBAR_X + 4, SIDEBAR_Y + (SIDEBAR_HEIGHT / 2), SIDEBAR_WIDTH - 8, COLOR_LINE);
    
    // Draw Up & Down buttons
    drawUpButton(canScrollUp, upPressed);
    drawDownButton(canScrollDown, downPressed);
}

void SidebarScrollButtons::drawUpButton(bool enabled, bool pressed) {
    int x = SIDEBAR_X + 2;
    int y = UP_BTN_Y;
    int w = SIDEBAR_WIDTH - 4;
    int h = UP_BTN_HEIGHT;
    
    uint16_t bgColor = pressed ? COLOR_LINE : COLOR_SURFACE;
    uint16_t iconColor = !enabled ? COLOR_MUTED : (pressed ? COLOR_IVORY : COLOR_BRASS);
    
    tft->fillRoundRect(x, y, w, h, 6, bgColor);
    
    if (enabled && !pressed) {
        tft->drawRoundRect(x, y, w, h, 6, COLOR_LINE);
    } else if (pressed) {
        tft->drawRoundRect(x, y, w, h, 6, COLOR_BRASS);
    }
    
    // Center icon
    int centerX = x + w / 2;
    int centerY = y + h / 2;
    drawUpChevron(centerX, centerY, iconColor);
}

void SidebarScrollButtons::drawDownButton(bool enabled, bool pressed) {
    int x = SIDEBAR_X + 2;
    int y = DOWN_BTN_Y;
    int w = SIDEBAR_WIDTH - 4;
    int h = DOWN_BTN_HEIGHT;
    
    uint16_t bgColor = pressed ? COLOR_LINE : COLOR_SURFACE;
    uint16_t iconColor = !enabled ? COLOR_MUTED : (pressed ? COLOR_IVORY : COLOR_BRASS);
    
    tft->fillRoundRect(x, y, w, h, 6, bgColor);
    
    if (enabled && !pressed) {
        tft->drawRoundRect(x, y, w, h, 6, COLOR_LINE);
    } else if (pressed) {
        tft->drawRoundRect(x, y, w, h, 6, COLOR_BRASS);
    }
    
    // Center icon
    int centerX = x + w / 2;
    int centerY = y + h / 2;
    drawDownChevron(centerX, centerY, iconColor);
}

void SidebarScrollButtons::drawUpChevron(int centerX, int centerY, uint16_t color) {
    // Render aesthetic Up triangle / chevron
    tft->fillTriangle(
        centerX, centerY - 8,
        centerX - 8, centerY + 4,
        centerX + 8, centerY + 4,
        color
    );
}

void SidebarScrollButtons::drawDownChevron(int centerX, int centerY, uint16_t color) {
    // Render aesthetic Down triangle / chevron
    tft->fillTriangle(
        centerX, centerY + 8,
        centerX - 8, centerY - 4,
        centerX + 8, centerY - 4,
        color
    );
}

bool SidebarScrollButtons::isTouchOnSidebar(int16_t x, int16_t y) const {
    return (x >= SIDEBAR_X && x <= SIDEBAR_X + SIDEBAR_WIDTH &&
            y >= SIDEBAR_Y && y <= SIDEBAR_Y + SIDEBAR_HEIGHT);
}

bool SidebarScrollButtons::handleTouch(const TouchPoint& point, int& currentScrollOffset, int maxScrollOffset, int step) {
    if (!isTouchOnSidebar(point.x, point.y)) {
        return false;
    }
    
    int originalOffset = currentScrollOffset;
    
    // Check Up button tap
    if (point.y >= UP_BTN_Y && point.y <= UP_BTN_Y + UP_BTN_HEIGHT) {
        currentScrollOffset -= step;
        if (currentScrollOffset < 0) {
            currentScrollOffset = 0;
        }
        Serial.printf("[SIDEBAR] Scroll UP tapped: new offset=%d\n", currentScrollOffset);
    }
    // Check Down button tap
    else if (point.y >= DOWN_BTN_Y && point.y <= DOWN_BTN_Y + DOWN_BTN_HEIGHT) {
        currentScrollOffset += step;
        if (currentScrollOffset > maxScrollOffset) {
            currentScrollOffset = maxScrollOffset;
        }
        Serial.printf("[SIDEBAR] Scroll DOWN tapped: new offset=%d\n", currentScrollOffset);
    }
    
    return (currentScrollOffset != originalOffset);
}
