#include "PasswordDetailScreen.h"
#include <string.h>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_BRASS_DARK 0xE4C0   // Darker brass for gradients
#define COLOR_BRASS_TEXT 0x10A2   // Dark text on brass
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Delete/danger

PasswordDetailScreen::PasswordDetailScreen(TFT_eSPI* display, TouchManager* touchMgr, VaultStorage* vaultStorage)
    : tft(display),
      touch(touchMgr),
      vault(vaultStorage),
      needsExit(false),
      needsDelete(false),
      needsEdit(false),
      forceFullRedraw(true),
      screenIsDirty(true),
      currentPasswordId(0),
      passwordVisible(false),
      eyePressStartTime(0),
      eyeButtonPressed(false) {
    
    memset(&currentPassword, 0, sizeof(currentPassword));
}

void PasswordDetailScreen::begin() {
    Serial.println("[PASSWORD_DETAIL] Screen initialized");
    needsExit = false;
    needsDelete = false;
    needsEdit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    passwordVisible = false;
    eyeButtonPressed = false;
}

void PasswordDetailScreen::reset() {
    needsExit = false;
    needsDelete = false;
    needsEdit = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    passwordVisible = false;
    eyeButtonPressed = false;
}

void PasswordDetailScreen::setPasswordId(uint32_t id) {
    currentPasswordId = id;
    
    if (vault) {
        currentPassword = vault->getPassword(id);
        Serial.printf("[PASSWORD_DETAIL] Loaded password: %s (ID=%d)\n", 
                     currentPassword.title, currentPasswordId);
    }
    
    forceFullRedraw = true;
}

void PasswordDetailScreen::update() {
    // No continuous updates needed
}

void PasswordDetailScreen::draw() {
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

void PasswordDetailScreen::drawStaticUI() {
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
    
    // Title (service name)
    tft->setTextDatum(TC_DATUM);
    tft->drawString(currentPassword.title, SCREEN_WIDTH / 2, 32);
    
    // One-Time badge (top right)
    if (currentPassword.isOneTime) {
        tft->fillRoundRect(SCREEN_WIDTH - 76, 30, 60, 18, 4, COLOR_CORAL);
        tft->setTextColor(COLOR_INK, COLOR_CORAL);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("1-TIME", SCREEN_WIDTH - 46, 39);
    }
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    tft->setTextDatum(TL_DATUM);
    
    Serial.println("[PASSWORD_DETAIL] Static UI drawn");
}

void PasswordDetailScreen::drawDynamicData() {
    // Clear content area
    tft->fillRect(0, HEADER_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HEADER_HEIGHT, COLOR_INK);
    
    // Calculate field positions
    int y = HEADER_HEIGHT + 16;
    
    // Title field
    drawField(y, "TITLE", currentPassword.title, false, false);
    y += FIELD_HEIGHT + FIELD_SPACING;
    
    // User field
    drawField(y, "USER", currentPassword.user, false, false);
    y += FIELD_HEIGHT + FIELD_SPACING;
    
    // Password field (with eye icon)
    drawField(y, "PASSWORD", currentPassword.pass, !passwordVisible, true);
    
    // Draw action buttons at bottom
    drawActionButtons();
}

void PasswordDetailScreen::drawField(int y, const char* label, const char* value, bool isMasked, bool hasEyeIcon) {
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    // Field container
    tft->fillRoundRect(SIDE_MARGIN, y, fieldWidth, FIELD_HEIGHT, 8, COLOR_SURFACE);
    tft->drawRoundRect(SIDE_MARGIN, y, fieldWidth, FIELD_HEIGHT, 8, COLOR_LINE);
    
    // Label
    tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
    tft->setTextSize(1);
    tft->setTextDatum(TL_DATUM);
    tft->drawString(label, SIDE_MARGIN + 12, y + 10);
    
    // Value
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    
    if (value[0] == '\0') {
        tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
        tft->drawString("...", SIDE_MARGIN + 12, y + 32);
    } else if (isMasked) {
        // Masked password
        int len = strlen(value);
        String masked = "";
        for (int i = 0; i < len && i < 16; i++) {
            masked += "*";
        }
        tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        tft->drawString(masked, SIDE_MARGIN + 12, y + 32);
    } else {
        // Normal text
        tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        tft->drawString(value, SIDE_MARGIN + 12, y + 32);
    }
    
    // Eye icon for password field
    if (hasEyeIcon) {
        int eyeX = SIDE_MARGIN + fieldWidth - 40;
        int eyeY = y + (FIELD_HEIGHT - 24) / 2;
        drawEyeIcon(eyeX, eyeY, eyeButtonPressed);
    }
    
    tft->setTextDatum(TL_DATUM);
}

void PasswordDetailScreen::drawEyeIcon(int x, int y, bool isPressed) {
    // Simple eye icon representation
    uint16_t bgColor = isPressed ? COLOR_BRASS : COLOR_SURFACE;
    uint16_t iconColor = isPressed ? COLOR_INK : COLOR_MUTED;
    
    // Background circle
    tft->fillCircle(x + 12, y + 12, 14, bgColor);
    tft->drawCircle(x + 12, y + 12, 14, iconColor);
    
    // Eye symbol (simplified)
    if (passwordVisible) {
        // Open eye - just draw circle
        tft->fillCircle(x + 12, y + 12, 6, iconColor);
    } else {
        // Closed eye - draw line
        tft->drawLine(x + 6, y + 12, x + 18, y + 12, iconColor);
    }
}

void PasswordDetailScreen::drawActionButtons() {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 16) / 3; // 3 buttons with 8px gaps
    
    int editX = SIDE_MARGIN;
    int favX = editX + buttonWidth + 8;
    int deleteX = favX + buttonWidth + 8;
    
    // Edit button (brass outline)
    tft->fillRoundRect(editX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_INK);
    tft->drawRoundRect(editX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_BRASS);
    tft->drawRoundRect(editX + 1, buttonY + 1, buttonWidth - 2, BUTTON_HEIGHT - 2, 6, COLOR_BRASS);
    tft->setTextColor(COLOR_BRASS, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Edit", editX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    // Favorite button (heart icon) - draw actual heart shape
    tft->fillRoundRect(favX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_INK);
    
    if (currentPassword.isFavorite) {
        // Draw button border in coral
        tft->drawRoundRect(favX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_CORAL);
        tft->drawRoundRect(favX + 1, buttonY + 1, buttonWidth - 2, BUTTON_HEIGHT - 2, 6, COLOR_CORAL);
        
        // Draw filled heart in coral
        drawHeart(favX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2, 12, COLOR_CORAL, true);
    } else {
        // Draw button border in muted
        tft->drawRoundRect(favX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_MUTED);
        
        // Draw outline heart in muted
        drawHeart(favX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2, 12, COLOR_MUTED, false);
    }
    
    // Delete button (coral filled)
    tft->fillRoundRect(deleteX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_CORAL);
    tft->setTextColor(COLOR_IVORY, COLOR_CORAL);
    tft->setTextSize(2);
    tft->drawString("Del", deleteX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    tft->setTextDatum(TL_DATUM);
}

void PasswordDetailScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[PASSWORD_DETAIL] Touch at (%d,%d)\n", point.x, point.y);
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_DETAIL] Back button pressed");
        if (currentPassword.isOneTime && vault && currentPasswordId != 0) {
            vault->deletePassword(currentPasswordId);
            Serial.printf("[PASSWORD_DETAIL] One-time guest password expired and auto-deleted: ID=%d\n", currentPasswordId);
        }
        needsExit = true;
        return;
    }
    
    // Check eye icon (toggle password visibility)
    if (isEyeIconPressed(point.x, point.y)) {
        passwordVisible = !passwordVisible;
        eyeButtonPressed = !eyeButtonPressed;
        forceFullRedraw = true;  // Force full redraw to update eye icon and password field
        screenIsDirty = true;
        Serial.printf("[PASSWORD_DETAIL] Password visibility toggled: %d\n", passwordVisible);
        return;
    }
    
    // Check Edit button
    if (isEditButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_DETAIL] Edit button pressed - NOT YET IMPLEMENTED");
        // TODO: Open edit screen
        // For now, just show in serial that it was pressed
        needsEdit = true;
        return;
    }
    
    // Check Favorite button
    if (isFavoriteButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_DETAIL] Favorite button pressed");
        toggleFavorite();
        forceFullRedraw = true;  // Force full redraw to update favorite button
        screenIsDirty = true;
        return;
    }
    
    // Check Delete button
    if (isDeleteButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_DETAIL] Delete button pressed");
        needsDelete = true;
        // Note: Screen will exit immediately, so no need to redraw
        return;
    }
}

bool PasswordDetailScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}

bool PasswordDetailScreen::isEyeIconPressed(int16_t x, int16_t y) const {
    // Password field is third field
    int fieldY = HEADER_HEIGHT + 16 + (FIELD_HEIGHT + FIELD_SPACING) * 2;
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    int eyeX = SIDE_MARGIN + fieldWidth - 40;
    int eyeY = fieldY + (FIELD_HEIGHT - 24) / 2;
    
    // Check if touch is within eye icon circle
    int dx = x - (eyeX + 12);
    int dy = y - (eyeY + 12);
    return (dx * dx + dy * dy) <= (14 * 14); // radius 14
}

bool PasswordDetailScreen::isEditButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 16) / 3;
    int editX = SIDE_MARGIN;
    
    return (x >= editX && x <= editX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

bool PasswordDetailScreen::isFavoriteButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 16) / 3;
    int favX = SIDE_MARGIN + buttonWidth + 8;
    
    return (x >= favX && x <= favX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

bool PasswordDetailScreen::isDeleteButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 16) / 3;
    int deleteX = SIDE_MARGIN + (buttonWidth + 8) * 2;
    
    return (x >= deleteX && x <= deleteX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

void PasswordDetailScreen::toggleFavorite() {
    if (vault && currentPasswordId != 0) {
        vault->toggleFavorite(currentPasswordId);
        
        // Reload the password to get updated favorite status
        currentPassword = vault->getPassword(currentPasswordId);
        
        screenIsDirty = true;
        Serial.printf("[PASSWORD_DETAIL] Favorite toggled: %d\n", currentPassword.isFavorite);
    }
}

void PasswordDetailScreen::drawHeart(int centerX, int centerY, int size, uint16_t color, bool filled) {
    // Heart is composed of two circles at the top and a triangle at the bottom
    // Size parameter controls the overall scale
    
    int radius = size / 3;  // Radius for the two top circles
    int leftCircleX = centerX - radius;
    int rightCircleX = centerX + radius;
    int circleY = centerY - radius / 2;
    
    // Bottom point of the heart
    int bottomX = centerX;
    int bottomY = centerY + size;
    
    if (filled) {
        // Fill the two circles at the top
        tft->fillCircle(leftCircleX, circleY, radius, color);
        tft->fillCircle(rightCircleX, circleY, radius, color);
        
        // Fill the triangular bottom part
        tft->fillTriangle(
            leftCircleX - radius, circleY,
            rightCircleX + radius, circleY,
            bottomX, bottomY,
            color
        );
        
    } else {
        // Draw outline by drawing filled heart, then smaller filled heart in background color
        
        // Draw outer filled heart in the outline color
        tft->fillCircle(leftCircleX, circleY, radius, color);
        tft->fillCircle(rightCircleX, circleY, radius, color);
        tft->fillTriangle(
            leftCircleX - radius, circleY,
            rightCircleX + radius, circleY,
            bottomX, bottomY,
            color
        );
        
        // Draw inner filled heart in background color (2 pixels smaller)
        int innerRadius = radius - 2;
        if (innerRadius > 0) {
            int innerLeftCircleX = centerX - innerRadius;
            int innerRightCircleX = centerX + innerRadius;
            int innerCircleY = centerY - innerRadius / 2;
            int innerBottomY = centerY + size - 3;
            
            tft->fillCircle(innerLeftCircleX, innerCircleY, innerRadius, COLOR_INK);
            tft->fillCircle(innerRightCircleX, innerCircleY, innerRadius, COLOR_INK);
            tft->fillTriangle(
                innerLeftCircleX - innerRadius, innerCircleY,
                innerRightCircleX + innerRadius, innerCircleY,
                bottomX, innerBottomY,
                COLOR_INK
            );
        }
    }
}
