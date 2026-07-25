#include "FavoritesScreen.h"
#include <algorithm>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Favorite heart color / One-time badge

FavoritesScreen::FavoritesScreen(TFT_eSPI* display, TouchManager* touchMgr, VaultStorage* vaultStorage)
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

void FavoritesScreen::begin() {
    Serial.println("[FAVORITES] Screen initialized");
    needsExit = false;
    selectedPasswordId = 0;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
    
    refreshFavoritesList();
    
    if (sidebarButtons) {
        sidebarButtons->begin();
    }
}

void FavoritesScreen::reset() {
    needsExit = false;
    selectedPasswordId = 0;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
    
    refreshFavoritesList();
}

void FavoritesScreen::update() {
    fabVisible = (maxScrollOffset > 0);
}

void FavoritesScreen::draw() {
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
    
    if (sidebarButtons) {
        sidebarButtons->draw(scrollOffset, maxScrollOffset);
    }
}

void FavoritesScreen::drawStaticUI() {
    // Status bar (24px)
    tft->fillRect(0, 0, SCREEN_WIDTH, 24, COLOR_INK);
    tft->fillCircle(12, 12, 3, COLOR_CORAL);
    
    // Header (32px)
    tft->fillRect(0, 24, SCREEN_WIDTH, 32, COLOR_INK);
    
    // Back button
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 16, 32);
    
    // Title with heart icon
    tft->setTextDatum(TC_DATUM);
    drawHeart(SCREEN_WIDTH / 2 - 40, 40, 10, COLOR_CORAL, true);
    
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->drawString("Favorites", SCREEN_WIDTH / 2 + 10, 32);
    
    // Count badge (top-right, leaving room for sidebar)
    char countStr[16];
    snprintf(countStr, sizeof(countStr), "%d", totalCount);
    tft->setTextSize(1);
    tft->setTextColor(COLOR_CORAL, COLOR_INK);
    tft->setTextDatum(TR_DATUM);
    tft->drawString(countStr, SCREEN_WIDTH - 48, 34);
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    tft->setTextDatum(TL_DATUM);
    
    Serial.println("[FAVORITES] Static UI drawn");
}

void FavoritesScreen::drawDynamicData() {
    tft->fillRect(0, HEADER_HEIGHT, 278, SCREEN_HEIGHT - HEADER_HEIGHT, COLOR_INK);
    
    if (favorites.empty()) {
        drawEmptyState();
        return;
    }
    
    int y = HEADER_HEIGHT - scrollOffset;
    
    for (size_t i = 0; i < favorites.size(); i++) {
        if (y + ROW_HEIGHT > HEADER_HEIGHT && y < SCREEN_HEIGHT) {
            drawPasswordRow(y, favorites[i], i);
        }
        
        y += ROW_HEIGHT + ROW_SPACING;
    }
}

void FavoritesScreen::drawPasswordRow(int y, const PasswordEntry& entry, int index) {
    int rowWidth = 256;
    
    tft->fillRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 10, COLOR_SURFACE);
    tft->drawRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 10, COLOR_CORAL);
    
    int avatarX = SIDE_MARGIN + 8;
    int avatarY = y + (ROW_HEIGHT - AVATAR_SIZE) / 2;
    
    uint16_t avatarColor = getAvatarColor(entry.title);
    tft->fillCircle(avatarX + AVATAR_SIZE / 2, avatarY + AVATAR_SIZE / 2, AVATAR_SIZE / 2, avatarColor);
    
    char initial[2] = {getInitialLetter(entry.title), '\0'};
    tft->setTextColor(COLOR_INK, avatarColor);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(initial, avatarX + AVATAR_SIZE / 2, avatarY + AVATAR_SIZE / 2);
    
    int textX = avatarX + AVATAR_SIZE + 10;
    tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    
    char shortTitle[12];
    strncpy(shortTitle, entry.title, sizeof(shortTitle) - 1);
    shortTitle[sizeof(shortTitle) - 1] = '\0';
    tft->drawString(shortTitle, textX, y + 12);
    
    tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
    tft->setTextSize(1);
    char shortUser[16];
    strncpy(shortUser, entry.user, sizeof(shortUser) - 1);
    shortUser[sizeof(shortUser) - 1] = '\0';
    tft->drawString(shortUser, textX, y + 36);
    
    if (entry.isOneTime) {
        int badgeX = SIDE_MARGIN + rowWidth - 58;
        int badgeY = y + 12;
        tft->fillRoundRect(badgeX, badgeY, 40, 16, 4, COLOR_CORAL);
        tft->setTextColor(COLOR_INK, COLOR_CORAL);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("1-TIME", badgeX + 20, badgeY + 8);
    }
    
    int heartX = SIDE_MARGIN + rowWidth - 12;
    int heartY = y + ROW_HEIGHT / 2;
    drawHeart(heartX, heartY, 8, COLOR_CORAL, true);
    
    tft->setTextDatum(TL_DATUM);
}

void FavoritesScreen::drawEmptyState() {
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    
    int centerY = HEADER_HEIGHT + (SCREEN_HEIGHT - HEADER_HEIGHT) / 2;
    drawHeart(130, centerY - 20, 20, COLOR_CORAL, true);
    
    tft->setTextColor(COLOR_MUTED, COLOR_INK);
    tft->setTextSize(2);
    tft->drawString("No favorites yet", 130, centerY + 20);
    
    tft->setTextSize(1);
    tft->drawString("Tap heart on any password", 130, centerY + 46);
    
    tft->setTextDatum(TL_DATUM);
}

void FavoritesScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[FAVORITES] Touch at (%d,%d)\n", point.x, point.y);
    
    if (sidebarButtons && sidebarButtons->isTouchOnSidebar(point.x, point.y)) {
        if (sidebarButtons->handleTouch(point, scrollOffset, maxScrollOffset)) {
            screenIsDirty = true;
        }
        return;
    }
    
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[FAVORITES] Back button pressed");
        needsExit = true;
        return;
    }
    
    int index = getPasswordIndexAtPosition(point.x, point.y);
    if (index >= 0 && index < (int)favorites.size()) {
        selectedPasswordId = favorites[index].id;
        Serial.printf("[FAVORITES] Selected password: %s (ID=%d)\n", 
                     favorites[index].title, selectedPasswordId);
        return;
    }
}

bool FavoritesScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}

int FavoritesScreen::getPasswordIndexAtPosition(int16_t x, int16_t y) const {
    if (y < HEADER_HEIGHT || y >= SCREEN_HEIGHT) {
        return -1;
    }
    
    if (x < SIDE_MARGIN || x > SIDE_MARGIN + 256) {
        return -1;
    }
    
    int adjustedY = y - HEADER_HEIGHT + scrollOffset;
    int rowIndex = adjustedY / (ROW_HEIGHT + ROW_SPACING);
    
    int yInRow = adjustedY % (ROW_HEIGHT + ROW_SPACING);
    if (yInRow >= ROW_HEIGHT) {
        return -1;
    }
    
    if (rowIndex >= 0 && rowIndex < (int)favorites.size()) {
        return rowIndex;
    }
    
    return -1;
}

void FavoritesScreen::refreshFavoritesList() {
    if (!vault) {
        Serial.println("[FAVORITES] No vault storage available");
        return;
    }
    
    // Get all favorites from vault
    std::vector<PasswordEntry> allFavorites = vault->getFavorites();
    
    // Filter out one-time passwords (they should only appear in keypad/web UI)
    favorites.clear();
    for (const auto& entry : allFavorites) {
        if (!entry.isOneTime) {
            favorites.push_back(entry);
        }
    }
    
    totalCount = favorites.size();
    
    Serial.printf("[FAVORITES] Loaded %d favorites from vault (filtered out one-time entries)\n", totalCount);
    
    calculateMaxScroll();
}

void FavoritesScreen::calculateMaxScroll() {
    int contentHeight = favorites.size() * (ROW_HEIGHT + ROW_SPACING);
    int visibleHeight = SCREEN_HEIGHT - HEADER_HEIGHT;
    
    maxScrollOffset = std::max(0, contentHeight - visibleHeight);
    
    if (scrollOffset > maxScrollOffset) {
        scrollOffset = maxScrollOffset;
    }
}

char FavoritesScreen::getInitialLetter(const char* title) {
    if (title && title[0] != '\0') {
        return toupper(title[0]);
    }
    return '?';
}

uint16_t FavoritesScreen::getAvatarColor(const char* title) {
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

void FavoritesScreen::drawHeart(int centerX, int centerY, int size, uint16_t color, bool filled) {
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
