#include "DeleteConfirmDialog.h"
#include <string.h>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Delete/danger

DeleteConfirmDialog::DeleteConfirmDialog(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      needsExit(false),
      confirmed(false),
      cancelled(false),
      forceFullRedraw(true),
      screenIsDirty(true) {
    
    memset(itemName, 0, sizeof(itemName));
}

void DeleteConfirmDialog::begin() {
    Serial.println("[DELETE_DIALOG] Dialog initialized");
    needsExit = false;
    confirmed = false;
    cancelled = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void DeleteConfirmDialog::reset() {
    needsExit = false;
    confirmed = false;
    cancelled = false;
    forceFullRedraw = true;
    screenIsDirty = true;
}

void DeleteConfirmDialog::setItemName(const char* name) {
    if (name) {
        strncpy(itemName, name, sizeof(itemName) - 1);
        itemName[sizeof(itemName) - 1] = '\0';
    } else {
        strcpy(itemName, "this item");
    }
    screenIsDirty = true;
}

void DeleteConfirmDialog::update() {
    // No continuous updates needed
}

void DeleteConfirmDialog::draw() {
    if (forceFullRedraw) {
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        drawDynamicData();
        screenIsDirty = false;
    }
}

void DeleteConfirmDialog::drawStaticUI() {
    // Calculate dialog position (centered)
    int dialogX = (SCREEN_WIDTH - DIALOG_WIDTH) / 2;
    int dialogY = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2;
    
    // Semi-transparent overlay (darken background)
    // Draw multiple rectangles to simulate transparency
    for (int i = 0; i < SCREEN_HEIGHT; i += 4) {
        tft->drawFastHLine(0, i, SCREEN_WIDTH, 0x2104);  // Dark gray
        tft->drawFastHLine(0, i + 1, SCREEN_WIDTH, 0x2104);
    }
    
    // Dialog container (raised card)
    tft->fillRoundRect(dialogX, dialogY, DIALOG_WIDTH, DIALOG_HEIGHT, 12, COLOR_SURFACE);
    tft->drawRoundRect(dialogX, dialogY, DIALOG_WIDTH, DIALOG_HEIGHT, 12, COLOR_LINE);
    tft->drawRoundRect(dialogX + 1, dialogY + 1, DIALOG_WIDTH - 2, DIALOG_HEIGHT - 2, 12, COLOR_LINE);
    
    // Warning icon (exclamation mark in coral circle)
    int iconCenterX = dialogX + DIALOG_WIDTH / 2;
    int iconCenterY = dialogY + 40;
    tft->fillCircle(iconCenterX, iconCenterY, 18, COLOR_CORAL);
    tft->setTextColor(COLOR_IVORY, COLOR_CORAL);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("!", iconCenterX, iconCenterY);
    
    Serial.println("[DELETE_DIALOG] Static UI drawn");
}

void DeleteConfirmDialog::drawDynamicData() {
    int dialogX = (SCREEN_WIDTH - DIALOG_WIDTH) / 2;
    int dialogY = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2;
    
    // Clear text area
    tft->fillRect(dialogX + 10, dialogY + 70, DIALOG_WIDTH - 20, 50, COLOR_SURFACE);
    
    // Title
    tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("Delete Password?", dialogX + DIALOG_WIDTH / 2, dialogY + 75);
    
    // Message
    tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    tft->setTextSize(1);
    tft->drawString("This action cannot be undone.", dialogX + DIALOG_WIDTH / 2, dialogY + 98);
    
    // Item name (truncated if too long)
    tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
    String truncated = String(itemName);
    if (truncated.length() > 24) {
        truncated = truncated.substring(0, 21) + "...";
    }
    tft->drawString(truncated, dialogX + DIALOG_WIDTH / 2, dialogY + 114);
    
    // Buttons at bottom
    int buttonY = dialogY + DIALOG_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (DIALOG_WIDTH - 40 - BUTTON_SPACING) / 2;
    int cancelX = dialogX + 20;
    int deleteX = cancelX + buttonWidth + BUTTON_SPACING;
    
    // Cancel button (brass outline)
    tft->fillRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_SURFACE);
    tft->drawRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_BRASS);
    tft->drawRoundRect(cancelX + 1, buttonY + 1, buttonWidth - 2, BUTTON_HEIGHT - 2, 6, COLOR_BRASS);
    tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Cancel", cancelX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    // Delete button (coral filled)
    tft->fillRoundRect(deleteX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_CORAL);
    tft->setTextColor(COLOR_IVORY, COLOR_CORAL);
    tft->setTextSize(2);
    tft->drawString("Delete", deleteX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    tft->setTextDatum(TL_DATUM);
}

void DeleteConfirmDialog::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[DELETE_DIALOG] Touch at (%d,%d)\n", point.x, point.y);
    
    // Check Delete button
    if (isDeleteButtonPressed(point.x, point.y)) {
        Serial.println("[DELETE_DIALOG] Delete confirmed");
        confirmed = true;
        needsExit = true;
        return;
    }
    
    // Check Cancel button
    if (isCancelButtonPressed(point.x, point.y)) {
        Serial.println("[DELETE_DIALOG] Delete cancelled");
        cancelled = true;
        needsExit = true;
        return;
    }
}

bool DeleteConfirmDialog::isDeleteButtonPressed(int16_t x, int16_t y) const {
    int dialogX = (SCREEN_WIDTH - DIALOG_WIDTH) / 2;
    int dialogY = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2;
    int buttonY = dialogY + DIALOG_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (DIALOG_WIDTH - 40 - BUTTON_SPACING) / 2;
    int deleteX = dialogX + 20 + buttonWidth + BUTTON_SPACING;
    
    return (x >= deleteX && x <= deleteX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

bool DeleteConfirmDialog::isCancelButtonPressed(int16_t x, int16_t y) const {
    int dialogX = (SCREEN_WIDTH - DIALOG_WIDTH) / 2;
    int dialogY = (SCREEN_HEIGHT - DIALOG_HEIGHT) / 2;
    int buttonY = dialogY + DIALOG_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (DIALOG_WIDTH - 40 - BUTTON_SPACING) / 2;
    int cancelX = dialogX + 20;
    
    return (x >= cancelX && x <= cancelX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}
