#include "SettingsListScreen.h"
#include <algorithm>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_STEEL 0x7BCF        // #778899 - Steel/silver accent
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata

SettingsListScreen::SettingsListScreen(TFT_eSPI* display, TouchManager* touchMgr, SettingsManager* settings)
    : tft(display),
      touch(touchMgr),
      settingsManager(settings),
      sidebarButtons(nullptr),
      needsExit(false),
      selectedAction(SETTINGS_ROW_NONE),
      forceFullRedraw(true),
      screenIsDirty(true),
      autoLockTimeout(15),
      twoTapSleepEnabled(false),
      scrollOffset(0),
      maxScrollOffset(0) {
    sidebarButtons = new SidebarScrollButtons(display);
}

void SettingsListScreen::begin() {
    Serial.println("[SETTINGS_LIST] Screen initialized");
    needsExit = false;
    selectedAction = SETTINGS_ROW_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
    
    // Load settings from SettingsManager
    if (settingsManager) {
        autoLockTimeout = settingsManager->getAutoLockTimeout();
        twoTapSleepEnabled = settingsManager->getTwoTapSleepEnabled();
        Serial.printf("[SETTINGS_LIST] Loaded settings: AutoLock=%ds, TwoTapSleep=%s\n",
                     autoLockTimeout, twoTapSleepEnabled ? "ON" : "OFF");
    } else {
        // Fallback defaults
        autoLockTimeout = 15;
        twoTapSleepEnabled = false;
    }
    
    calculateMaxScroll();
    
    if (sidebarButtons) {
        sidebarButtons->begin();
    }
}

void SettingsListScreen::reset() {
    needsExit = false;
    selectedAction = SETTINGS_ROW_NONE;
    forceFullRedraw = true;
    screenIsDirty = true;
    scrollOffset = 0;
}

void SettingsListScreen::update() {
    // No continuous updates needed
}

void SettingsListScreen::draw() {
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
    
    // Right sidebar scroll buttons (same rail as Passwords/Favorites)
    if (sidebarButtons) {
        sidebarButtons->draw(scrollOffset, maxScrollOffset);
    }
}

void SettingsListScreen::drawStaticUI() {
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
    tft->drawString("Settings", SCREEN_WIDTH / 2, 32);
    
    // Border separator
    tft->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    tft->setTextDatum(TL_DATUM);
    
    Serial.println("[SETTINGS_LIST] Static UI drawn");
}

void SettingsListScreen::drawDynamicData() {
    // Clear content area only (excluding the sidebar strip at x=280, like PasswordsScreen)
    tft->fillRect(0, HEADER_HEIGHT, 278, SCREEN_HEIGHT - HEADER_HEIGHT, COLOR_INK);
    
    const char* timeoutStr = getAutoLockTimeoutString();
    
    struct RowDef { const char* label; const char* value; const char* icon; bool hasToggle; bool toggleState; };
    RowDef rows[ROW_COUNT] = {
        { "Auto-Lock",    timeoutStr, "v",  false, false },
        { "Manual Sleep", nullptr,    nullptr, true, twoTapSleepEnabled },
        { "Change PIN",   nullptr,    ">",  false, false },
        { "Advanced",     nullptr,    ">",  false, false },
        { "About",        nullptr,    ">",  false, false },
    };
    
    int y = ROWS_START_Y - scrollOffset;
    
    for (int i = 0; i < ROW_COUNT; i++) {
        // Only draw rows that are actually visible (perf + avoids drawing under the header/sidebar)
        if (y + ROW_HEIGHT > HEADER_HEIGHT && y < SCREEN_HEIGHT) {
            drawSettingsRow(y, rows[i].label, rows[i].value, rows[i].icon, rows[i].hasToggle, rows[i].toggleState);
        }
        y += ROW_HEIGHT + ROW_SPACING;
    }
}

void SettingsListScreen::drawSettingsRow(int y, const char* label, const char* value, const char* rightIcon, bool hasToggle, bool toggleState) {
    int rowWidth = ROW_WIDTH;
    
    // Row background
    tft->fillRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 8, COLOR_SURFACE);
    tft->drawRoundRect(SIDE_MARGIN, y, rowWidth, ROW_HEIGHT, 8, COLOR_LINE);
    
    // Label (left side) - vertically centered
    tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    int textY = y + (ROW_HEIGHT - 16) / 2;  // Center text (16px is text height for size 2)
    tft->drawString(label, SIDE_MARGIN + 12, textY);
    
    // Right content
    if (hasToggle) {
        // Draw toggle switch (centered vertically)
        int toggleX = SIDE_MARGIN + rowWidth - 56;
        int toggleY = y + (ROW_HEIGHT - 24) / 2;
        drawToggleSwitch(toggleX, toggleY, toggleState);
    } else if (value != nullptr) {
        // Draw value text (e.g., "15s") - vertically centered
        tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
        tft->setTextSize(2);
        tft->setTextDatum(TR_DATUM);
        tft->drawString(value, SIDE_MARGIN + rowWidth - 50, textY);
        
        // Draw dropdown indicator if rightIcon is "v"
        if (rightIcon && rightIcon[0] == 'v') {
            int dropdownX = SIDE_MARGIN + rowWidth - 30;
            int dropdownY = y + ROW_HEIGHT / 2;
            drawDropdownIndicator(dropdownX, dropdownY);
        }
    } else if (rightIcon != nullptr) {
        // Draw chevron icon (>) - centered
        tft->setTextColor(COLOR_MUTED, COLOR_SURFACE);
        tft->setTextSize(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(rightIcon, SIDE_MARGIN + rowWidth - 20, y + ROW_HEIGHT / 2);
    }
    
    tft->setTextDatum(TL_DATUM);
}

void SettingsListScreen::drawToggleSwitch(int x, int y, bool enabled) {
    // Toggle switch: 48px wide, 24px tall
    int switchWidth = 48;
    int switchHeight = 24;
    
    // Background track
    uint16_t trackColor = enabled ? COLOR_BRASS : COLOR_MUTED;
    tft->fillRoundRect(x, y, switchWidth, switchHeight, 12, trackColor);
    
    // Knob (circle)
    int knobRadius = 10;
    int knobY = y + switchHeight / 2;
    int knobX = enabled ? (x + switchWidth - knobRadius - 4) : (x + knobRadius + 4);
    
    tft->fillCircle(knobX, knobY, knobRadius, COLOR_IVORY);
    tft->drawCircle(knobX, knobY, knobRadius, COLOR_LINE);
}

void SettingsListScreen::drawDropdownIndicator(int x, int y) {
    // Draw small downward chevron (v)
    int size = 6;
    
    // Draw as two lines forming a V
    tft->drawLine(x - size, y - 3, x, y + 3, COLOR_MUTED);
    tft->drawLine(x, y + 3, x + size, y - 3, COLOR_MUTED);
    
    // Thicken the lines
    tft->drawLine(x - size, y - 2, x, y + 4, COLOR_MUTED);
    tft->drawLine(x, y + 4, x + size, y - 2, COLOR_MUTED);
}

void SettingsListScreen::onTouchEvent(const TouchPoint& point) {
    Serial.println("========================================");
    Serial.printf("[SETTINGS_LIST] RAW TOUCH EVENT: X=%d, Y=%d\n", point.x, point.y);
    Serial.println("========================================");
    
    // Check right sidebar scroll buttons first (same pattern as PasswordsScreen)
    if (sidebarButtons && sidebarButtons->isTouchOnSidebar(point.x, point.y)) {
        if (sidebarButtons->handleTouch(point, scrollOffset, maxScrollOffset)) {
            screenIsDirty = true;
        }
        return;
    }
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[SETTINGS_LIST] >>> BACK BUTTON HIT <<<");
        needsExit = true;
        return;
    }
    Serial.println("[SETTINGS_LIST] Back button check: MISS");
    
    // Get row at touch position
    SettingsRow row = getRowAtPosition(point.x, point.y);
    
    Serial.printf("[SETTINGS_LIST] Row detection result: %d\n", (int)row);
    
    if (row == SETTINGS_ROW_NONE) {
        Serial.println("[SETTINGS_LIST] No row hit - touch ignored");
        return;
    }
    
    // Handle row-specific interactions
    switch (row) {
        case SETTINGS_ROW_AUTO_LOCK:
            Serial.println("[SETTINGS_LIST] Row: AUTO_LOCK");
            // Check if dropdown area was tapped
            if (isDropdownPressed(point.x, point.y, row)) {
                Serial.println("[SETTINGS_LIST] >>> Auto-Lock dropdown tapped <<<");
                cycleAutoLockTimeout();
                screenIsDirty = true;
            } else {
                Serial.println("[SETTINGS_LIST] Auto-Lock row tapped (not dropdown area)");
            }
            break;
            
        case SETTINGS_ROW_TWO_TAP_SLEEP:
            Serial.println("[SETTINGS_LIST] Row: TWO_TAP_SLEEP");
            // Check if toggle switch was tapped
            if (isToggleSwitchPressed(point.x, point.y, row)) {
                Serial.println("[SETTINGS_LIST] >>> Two Tap Sleep toggle tapped <<<");
                toggleTwoTapSleep();
                screenIsDirty = true;
            } else {
                Serial.println("[SETTINGS_LIST] Two Tap Sleep row tapped (not toggle area)");
            }
            break;
            
        case SETTINGS_ROW_CHANGE_PIN:
            Serial.println("[SETTINGS_LIST] Row: CHANGE_PIN");
            Serial.println("[SETTINGS_LIST] >>> Change PIN selected <<<");
            selectedAction = SETTINGS_ROW_CHANGE_PIN;
            break;
            
        case SETTINGS_ROW_ADVANCED:
            Serial.println("[SETTINGS_LIST] Row: ADVANCED");
            Serial.println("[SETTINGS_LIST] >>> Advanced selected <<<");
            selectedAction = SETTINGS_ROW_ADVANCED;
            break;
            
        case SETTINGS_ROW_ABOUT:
            Serial.println("[SETTINGS_LIST] Row: ABOUT");
            Serial.println("[SETTINGS_LIST] >>> About selected <<<");
            selectedAction = SETTINGS_ROW_ABOUT;
            break;
            
        default:
            Serial.println("[SETTINGS_LIST] Unknown row detected");
            break;
    }
    Serial.println("========================================");
}

bool SettingsListScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= 24 && y <= 56 && x <= 60);
}

SettingsRow SettingsListScreen::getRowAtPosition(int16_t x, int16_t y) const {
    Serial.printf("[ROW_DETECT] Input: X=%d, Y=%d\n", x, y);
    
    // Check if within row bounds horizontally (rows now stop before the sidebar, like PasswordsScreen)
    if (x < SIDE_MARGIN || x > SIDE_MARGIN + ROW_WIDTH) {
        Serial.printf("[ROW_DETECT] X out of bounds (margin=%d, rowRight=%d)\n", SIDE_MARGIN, SIDE_MARGIN + ROW_WIDTH);
        return SETTINGS_ROW_NONE;
    }
    
    // Calculate which row, accounting for the current scroll position
    int relativeY = y - ROWS_START_Y + scrollOffset;
    Serial.printf("[ROW_DETECT] Relative Y: %d (Y=%d - START=%d + scroll=%d)\n", relativeY, y, ROWS_START_Y, scrollOffset);
    
    if (relativeY < 0) {
        Serial.println("[ROW_DETECT] Touch above rows area");
        return SETTINGS_ROW_NONE;
    }
    
    int rowIndex = relativeY / (ROW_HEIGHT + ROW_SPACING);
    Serial.printf("[ROW_DETECT] Calculated rowIndex: %d (relY=%d / (H=%d + S=%d))\n", 
                 rowIndex, relativeY, ROW_HEIGHT, ROW_SPACING);
    
    // Ensure rowIndex is within the actual row count
    if (rowIndex >= ROW_COUNT) {
        Serial.printf("[ROW_DETECT] *** REJECT: rowIndex=%d exceeds max (%d) ***\n", rowIndex, ROW_COUNT - 1);
        return SETTINGS_ROW_NONE;
    }
    
    // Check if within actual row (not spacing)
    int yInRow = relativeY % (ROW_HEIGHT + ROW_SPACING);
    Serial.printf("[ROW_DETECT] Y position in row: %d (must be < %d)\n", yInRow, ROW_HEIGHT);
    
    if (yInRow >= ROW_HEIGHT) {
        Serial.printf("[ROW_DETECT] *** REJECT: Touch in row spacing (yInRow=%d >= %d) ***\n", yInRow, ROW_HEIGHT);
        return SETTINGS_ROW_NONE;
    }
    
    // Map index to row enum
    SettingsRow result = SETTINGS_ROW_NONE;
    switch (rowIndex) {
        case 0: result = SETTINGS_ROW_AUTO_LOCK; break;
        case 1: result = SETTINGS_ROW_TWO_TAP_SLEEP; break;
        case 2: result = SETTINGS_ROW_CHANGE_PIN; break;
        case 3: result = SETTINGS_ROW_ADVANCED; break;
        case 4: result = SETTINGS_ROW_ABOUT; break;
        default: result = SETTINGS_ROW_NONE; break;
    }
    
    const char* rowNames[] = {"NONE", "AUTO_LOCK", "TWO_TAP_SLEEP", "CHANGE_PIN", "ADVANCED", "ABOUT"};
    Serial.printf("[ROW_DETECT] *** RESULT: %s (enum=%d) ***\n", 
                 rowNames[result], (int)result);
    
    return result;
}

bool SettingsListScreen::isToggleSwitchPressed(int16_t x, int16_t y, SettingsRow row) const {
    // Calculate row Y position (screen-space, i.e. minus scroll offset — matches drawDynamicData)
    int rowIndex = (int)row - 1;  // Convert enum to 0-based index
    int rowY = ROWS_START_Y - scrollOffset + rowIndex * (ROW_HEIGHT + ROW_SPACING);
    
    // Toggle switch is on the right side of the sidebar-aware row width
    int toggleX = SIDE_MARGIN + ROW_WIDTH - 60;
    int toggleY = rowY + (ROW_HEIGHT - 24) / 2;
    
    return (x >= toggleX && x <= toggleX + 48 &&
            y >= toggleY && y <= toggleY + 24);
}

bool SettingsListScreen::isDropdownPressed(int16_t x, int16_t y, SettingsRow row) const {
    // Calculate row Y position (screen-space, i.e. minus scroll offset — matches drawDynamicData)
    int rowIndex = (int)row - 1;  // Convert enum to 0-based index
    int rowY = ROWS_START_Y - scrollOffset + rowIndex * (ROW_HEIGHT + ROW_SPACING);
    
    // Dropdown area is the right portion of the sidebar-aware row width
    int dropdownX = SIDE_MARGIN + ROW_WIDTH - 80;  // Right 80px of row
    
    return (x >= dropdownX && x <= SIDE_MARGIN + ROW_WIDTH &&
            y >= rowY && y <= rowY + ROW_HEIGHT);
}

void SettingsListScreen::cycleAutoLockTimeout() {
    // Cycle through timeout options
    switch (autoLockTimeout) {
        case 5:
            autoLockTimeout = 10;
            break;
        case 10:
            autoLockTimeout = 15;
            break;
        case 15:
            autoLockTimeout = 20;
            break;
        case 20:
            autoLockTimeout = 30;
            break;
        case 30:
            autoLockTimeout = 0;  // Never
            break;
        default:
            autoLockTimeout = 5;  // Reset to 5s
            break;
    }
    
    Serial.printf("[SETTINGS_LIST] Auto-Lock timeout changed to: %s\n", getAutoLockTimeoutString());
    
    // Save to SettingsManager
    if (settingsManager) {
        settingsManager->setAutoLockTimeout(autoLockTimeout);
    }
}

void SettingsListScreen::toggleTwoTapSleep() {
    twoTapSleepEnabled = !twoTapSleepEnabled;
    
    Serial.printf("[SETTINGS_LIST] Two Tap Sleep: %s\n", twoTapSleepEnabled ? "ENABLED" : "DISABLED");
    
    // Save to SettingsManager
    if (settingsManager) {
        settingsManager->setTwoTapSleepEnabled(twoTapSleepEnabled);
    }
}

void SettingsListScreen::calculateMaxScroll() {
    int contentHeight = ROW_COUNT * ROW_HEIGHT + (ROW_COUNT - 1) * ROW_SPACING;
    int visibleHeight = SCREEN_HEIGHT - HEADER_HEIGHT;
    
    maxScrollOffset = std::max(0, contentHeight - visibleHeight);
    
    if (scrollOffset > maxScrollOffset) {
        scrollOffset = maxScrollOffset;
    }
    
    Serial.printf("[SETTINGS_LIST] contentHeight=%d visibleHeight=%d maxScrollOffset=%d\n",
                 contentHeight, visibleHeight, maxScrollOffset);
}

const char* SettingsListScreen::getAutoLockTimeoutString() const {
    switch (autoLockTimeout) {
        case 5: return "5s";
        case 10: return "10s";
        case 15: return "15s";
        case 20: return "20s";
        case 30: return "30s";
        case 0: return "Never";
        default: return "15s";
    }
}
