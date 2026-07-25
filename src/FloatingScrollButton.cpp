#include "FloatingScrollButton.h"

FloatingScrollButton::FloatingScrollButton(TFT_eSPI* display)
    : tft(display),
      buttonPressed(false),
      buttonWasTapped(false),
      needsRedraw(true) {
    
    // Calculate button position (bottom-right corner, inset)
    buttonCenterX = SCREEN_WIDTH - BUTTON_INSET_X - BUTTON_RADIUS;
    buttonCenterY = SCREEN_HEIGHT - BUTTON_INSET_Y - BUTTON_RADIUS;
}

void FloatingScrollButton::begin() {
    buttonPressed = false;
    buttonWasTapped = false;
    needsRedraw = true;
    
    DEBUG_LOG("FloatingScrollButton initialized");
}

void FloatingScrollButton::update(const TouchPoint& point, TouchState state) {
    bool wasPressedBefore = buttonPressed;
    
    // Check if touch is on button
    bool touchOnButton = isTouchOnButton(point.x, point.y);
    
    // Update button state based on touch state
    if (state == TOUCH_DOWN || state == TOUCH_MOVE) {
        if (point.valid && touchOnButton) {
            buttonPressed = true;
        } else {
            buttonPressed = false;
        }
    } else if (state == TOUCH_UP) {
        // Tap detected if button was pressed and now released
        if (buttonPressed && touchOnButton) {
            buttonWasTapped = true;
            DEBUG_LOG("FAB: Tap detected");
        }
        buttonPressed = false;
    } else {
        // TOUCH_IDLE
        buttonPressed = false;
    }
    
    // Set redraw flag if state changed
    if (buttonPressed != wasPressedBefore) {
        needsRedraw = true;
    }
}

void FloatingScrollButton::draw() {
    if (!needsRedraw) {
        return;  // Skip redraw if state hasn't changed
    }
    
    // Choose color based on state
    uint16_t fillColor = buttonPressed ? COLOR_PRESSED : COLOR_IDLE;
    
    // Draw the button
    drawButton(fillColor);
    
    // Draw the chevron
    uint16_t chevronColor = buttonPressed ? TFT_WHITE : TFT_BLACK;
    drawChevron(chevronColor);
    
    needsRedraw = false;
    
    DEBUG_LOGF("FAB: Drawn (pressed=%d)\n", buttonPressed);
}

bool FloatingScrollButton::wasTapped() {
    return buttonWasTapped;
}

void FloatingScrollButton::clearTap() {
    buttonWasTapped = false;
}

bool FloatingScrollButton::isTouchOnButton(int16_t x, int16_t y) const {
    // Calculate distance from touch point to button center
    int dx = x - buttonCenterX;
    int dy = y - buttonCenterY;
    int distanceSquared = (dx * dx) + (dy * dy);
    int radiusSquared = BUTTON_RADIUS * BUTTON_RADIUS;
    
    return distanceSquared <= radiusSquared;
}

void FloatingScrollButton::drawButton(uint16_t fillColor) {
    // Draw filled circle
    tft->fillCircle(buttonCenterX, buttonCenterY, BUTTON_RADIUS, fillColor);
    
    // Draw border
    tft->drawCircle(buttonCenterX, buttonCenterY, BUTTON_RADIUS, COLOR_BORDER);
    tft->drawCircle(buttonCenterX, buttonCenterY, BUTTON_RADIUS - 1, COLOR_BORDER);
}

void FloatingScrollButton::drawChevron(uint16_t color) {
    // Draw downward-pointing V chevron
    // Left stroke: from top-left to bottom-center
    tft->drawLine(buttonCenterX - CHEVRON_SIZE, buttonCenterY - CHEVRON_SIZE / 2,
                  buttonCenterX, buttonCenterY + CHEVRON_SIZE / 2, color);
    
    // Right stroke: from top-right to bottom-center
    tft->drawLine(buttonCenterX, buttonCenterY + CHEVRON_SIZE / 2,
                  buttonCenterX + CHEVRON_SIZE, buttonCenterY - CHEVRON_SIZE / 2, color);
    
    // Draw again with slight offset for thickness
    tft->drawLine(buttonCenterX - CHEVRON_SIZE, buttonCenterY - CHEVRON_SIZE / 2 + 1,
                  buttonCenterX, buttonCenterY + CHEVRON_SIZE / 2 + 1, color);
    
    tft->drawLine(buttonCenterX, buttonCenterY + CHEVRON_SIZE / 2 + 1,
                  buttonCenterX + CHEVRON_SIZE, buttonCenterY - CHEVRON_SIZE / 2 + 1, color);
}
