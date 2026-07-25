#include "PasswordsScreen.h"
#include <algorithm>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Warning / One-time badge

PasswordsScreen::PasswordsScreen(TFT_eSPI* display, TouchManager* touchMgr, VaultStorage* vaultStorage)
    : tft(display),
      touch(touchMgr),
      vault(vaultStorage),
      sidebarButtons(nullptr),
      needsExit(false),
      selectedPasswordId(0),
      forceFullRedraw(true),
      screenIsDirty(true),
      fabVisible(false),
      scrollOffset(0),
      maxScrollOffset(0),
      totalCount(0) {
    
    sidebarButtons = new SidebarScrollButtons(display);
}

void PasswordsScreen::begin() {
    Serial.println("[PASSWORDS] Screen initialized");
    needsExit = false;
    selectedPasswordId = 0;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
    
    refreshPasswordList();
    
    if (sidebarButtons) {
        sidebarButtons->begin();
    }
}

void PasswordsScreen::reset() {
    needsExit = false;
    selectedPasswordId = 0;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
    
    refreshPasswordList();
}

void PasswordsScreen::update() {
    fabVisible = (maxScrollOffset > 0);
}

void PasswordsScreen::draw() {
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
    
    // Draw right sidebar scroll buttons
    if (sidebarButtons) {
        sidebarButtons->draw(scrollOffset, maxScrollOffset);
    }
}

void PasswordsScreen::drawStaticUI() {
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
    tft->drawString("Passwords", SCREEN_WIDTH / 2, 32);
    
    // Count badge (top-right)
    char countStr[16];
    snprintf(countStr, sizeof(countStr), "%d TOTAL", totalCount);
    tft->setTextSize(1);
    tft->setTextColor(COLOR_BRASS, COLOR_INK);
    tft->setTextDatum(TR_DATUM);
    tft->drawString(countStr, SCREEN_WIDTH - 48, 34);
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    tft->setTextDatum(TL_DATUM);
    
    Serial.println("[PASSWORDS] Static UI drawn");
}

void PasswordsScreen::drawDynamicData() {
    // Clear content area (excluding sidebar at x=280)
    tft->fillRect(0, HEADER_HEIGHT, 278, SCREEN_HEIGHT - HEADER_HEIGHT, COLOR_INK);
    
    if (passwords.empty()) {
        drawEmptyState();
        return;
    }
    
    // Draw password rows
    int y = HEADER_HEIGHT - scrollOffset;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (y + ROW_HEIGHT > HEADER_HEIGHT && y < SCREEN_HEIGHT) {
            drawPasswordRow(y, passwords[i], i);
        }
        
        y += ROW_HEIGHT + ROW_SPACING;
    }
}

void PasswordsScreen::drawPasswordRow(int y, const PasswordEntry& entry, int index) {
    // Row width accounts for right sidebar (width = 256px)
    int rowWidth = 256;
    
    // Row container background
    tft->fillRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 10, COLOR_SURFACE);
    tft->drawRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 10, COLOR_LINE);
    
    // Avatar circle with initial
    int avatarX = SIDE_MARGIN + 8;
    int avatarY = y + (ROW_HEIGHT - AVATAR_SIZE) / 2;
    
    uint16_t avatarColor = getAvatarColor(entry.title);
    tft->fillCircle(avatarX + AVATAR_SIZE / 2, avatarY + AVATAR_SIZE / 2, AVATAR_SIZE / 2, avatarColor);
    
    // Initial letter
    char initial[2] = {getInitialLetter(entry.title), '\0'};
    tft->setTextColor(COLOR_INK, avatarColor);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(initial, avatarX + AVATAR_SIZE / 2, avatarY + AVATAR_SIZE / 2);
    
    // Service name (title)
    int textX = avatarX + AVATAR_SIZE + 10;
    tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    
    // Truncate long titles to fit row cleanly
    char shortTitle[12];
    strncpy(shortTitle, entry.title, sizeof(shortTitle) - 1);
    shortTitle[sizeof(shortTitle) - 1] = '\0';
    tft->drawString(shortTitle, textX, y + 12);
    
    // Username/email (subtext)
    tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    tft->setTextSize(1);
    char shortUser[16];
    strncpy(shortUser, entry.user, sizeof(shortUser) - 1);
    shortUser[sizeof(shortUser) - 1] = '\0';
    tft->drawString(shortUser, textX, y + 36);
    
    // If one-time password, draw 1-TIME badge
    if (entry.isOneTime) {
        int badgeX = SIDE_MARGIN + rowWidth - 58;
        int badgeY = y + 12;
        tft->fillRoundRect(badgeX, badgeY, 40, 16, 4, COLOR_CORAL);
        tft->setTextColor(COLOR_INK, COLOR_CORAL);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("1-TIME", badgeX + 20, badgeY + 8);
    }
    
    // Chevron icon (right side)
    int chevronX = SIDE_MARGIN + rowWidth - 12;
    tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(">", chevronX, y + ROW_HEIGHT / 2);
    
    tft->setTextDatum(TL_DATUM);
}

void PasswordsScreen::drawEmptyState() {
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    
    int centerY = HEADER_HEIGHT + (SCREEN_HEIGHT - HEADER_HEIGHT) / 2;
    tft->drawString("No passwords yet", 130, centerY - 20);
    
    tft->setTextSize(1);
    tft->drawString("Add your first password", 130, centerY + 10);
    tft->drawString("using Web UI or + button", 130, centerY + 26);
    
    tft->setTextDatum(TL_DATUM);
}

void PasswordsScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[PASSWORDS] Touch at (%d,%d)\n", point.x, point.y);
    
    // Check right sidebar scroll buttons first
    if (sidebarButtons && sidebarButtons->isTouchOnSidebar(point.x, point.y)) {
        if (sidebarButtons->handleTouch(point, scrollOffset, maxScrollOffset)) {
            screenIsDirty = true;
        }
        return;
    }
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORDS] Back button pressed");
        needsExit = true;
        return;
    }
    
    // Check password row tap
    int index = getPasswordIndexAtPosition(point.x, point.y);
    if (index >= 0 && index < (int)passwords.size()) {
        selectedPasswordId = passwords[index].id;
        Serial.printf("[PASSWORDS] Selected password: %s (ID=%d)\n", 
                     passwords[index].title, selectedPasswordId);
        return;
    }
}

bool PasswordsScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}

int PasswordsScreen::getPasswordIndexAtPosition(int16_t x, int16_t y) const {
    if (y < HEADER_HEIGHT || y >= SCREEN_HEIGHT) {
        return -1;
    }
    
    // Bounds for row container (width = 256)
    if (x < SIDE_MARGIN || x > SIDE_MARGIN + 256) {
        return -1;
    }
    
    int adjustedY = y - HEADER_HEIGHT + scrollOffset;
    int rowIndex = adjustedY / (ROW_HEIGHT + ROW_SPACING);
    
    int yInRow = adjustedY % (ROW_HEIGHT + ROW_SPACING);
    if (yInRow >= ROW_HEIGHT) {
        return -1;
    }
    
    if (rowIndex >= 0 && rowIndex < (int)passwords.size()) {
        return rowIndex;
    }
    
    return -1;
}

void PasswordsScreen::refreshPasswordList() {
    if (!vault) {
        Serial.println("[PASSWORDS] No vault storage available");
        return;
    }
    
    passwords = vault->getAllPasswords();
    totalCount = passwords.size();
    
    Serial.printf("[PASSWORDS] Loaded %d passwords from vault\n", totalCount);
    
    calculateMaxScroll();
}

void PasswordsScreen::calculateMaxScroll() {
    int contentHeight = passwords.size() * (ROW_HEIGHT + ROW_SPACING);
    int visibleHeight = SCREEN_HEIGHT - HEADER_HEIGHT;
    
    maxScrollOffset = std::max(0, contentHeight - visibleHeight);
    
    if (scrollOffset > maxScrollOffset) {
        scrollOffset = maxScrollOffset;
    }
}

char PasswordsScreen::getInitialLetter(const char* title) {
    if (title && title[0] != '\0') {
        return toupper(title[0]);
    }
    return '?';
}

uint16_t PasswordsScreen::getAvatarColor(const char* title) {
    if (!title || title[0] == '\0') {
        return COLOR_BRASS;
    }
    
    uint8_t hash = toupper(title[0]) % 6;
    
    const uint16_t colors[] = {
        COLOR_BRASS,
        0xFD20,
        0xFBE0,
        0x07E0,
        0x05FF,
        0xF81F
    };
    
    return colors[hash];
}

void PasswordsScreen::drawHeart(int centerX, int centerY, int size, uint16_t color, bool filled) {
    int radius = size / 3;
    int leftCircleX = centerX - radius;
    int rightCircleX = centerX + radius;
    int circleY = centerY - radius / 2;
    
    int bottomX = centerX;
    int bottomY = centerY + size;
    
    if (filled) {
        tft->fillCircle(leftCircleX, circleY, radius, color);
        tft->fillCircle(rightCircleX, circleY, radius, color);
        tft->fillTriangle(
            leftCircleX - radius, circleY,
            rightCircleX + radius, circleY,
            bottomX, bottomY,
            color
        );
    }
}
