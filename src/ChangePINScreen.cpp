#include "ChangePINScreen.h"

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_STEEL 0x7BCF        // #778899 - Steel/silver accent
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_ERROR 0xF800        // Red for errors
#define COLOR_SUCCESS 0x07E0      // Green for success

ChangePINScreen::ChangePINScreen(TFT_eSPI* display, TouchManager* touchMgr, PINManager* pinMgr)
    : tft(display),
      touch(touchMgr),
      pinManager(pinMgr),
      forceFullRedraw(true),
      screenIsDirty(true),
      needsExit(false),
      state(PIN_CHANGE_ENTER_CURRENT),
      pinIndex(0),
      messageDisplayStart(0),
      errorMessage(nullptr),
      lastTapTime(0) {
    memset(currentPIN, 0, sizeof(currentPIN));
    memset(newPIN, 0, sizeof(newPIN));
    memset(confirmPIN, 0, sizeof(confirmPIN));
}

void ChangePINScreen::begin() {
    Serial.println("[CHANGE_PIN] Screen initialized");
    state = PIN_CHANGE_ENTER_CURRENT;
    pinIndex = 0;
    needsExit = false;
    lastTapTime = 0;
    memset(currentPIN, 0, sizeof(currentPIN));
    memset(newPIN, 0, sizeof(newPIN));
    memset(confirmPIN, 0, sizeof(confirmPIN));
    forceFullRedraw = true;
    screenIsDirty = true;
    errorMessage = nullptr;
}

void ChangePINScreen::reset() {
    begin();
}

void ChangePINScreen::update() {
    // Check if message display is complete
    if ((state == PIN_CHANGE_SUCCESS || state == PIN_CHANGE_ERROR) &&
        (millis() - messageDisplayStart) > MESSAGE_DISPLAY_DURATION) {
        
        if (state == PIN_CHANGE_SUCCESS) {
            needsExit = true;  // Exit back to settings
        } else {
            // Reset to appropriate state on error
            state = PIN_CHANGE_ENTER_CURRENT;
            clearCurrentEntry();
            forceFullRedraw = true;
        }
    }
}

void ChangePINScreen::draw() {
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

void ChangePINScreen::drawStaticUI() {
    // Header with back button
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 16, 16);
    
    // Title
    tft->setTextDatum(TC_DATUM);
    tft->drawString("Change PIN", SCREEN_WIDTH / 2, 16);
    
    // Draw keypad if not showing message
    if (state != PIN_CHANGE_SUCCESS && state != PIN_CHANGE_ERROR) {
        drawKeypad();
    }
    
    Serial.println("[CHANGE_PIN] Static UI drawn");
}

void ChangePINScreen::drawDynamicData() {
    // Clear prompt area
    tft->fillRect(0, 40, SCREEN_WIDTH, 30, COLOR_INK);
    
    // Clear dots area
    tft->fillRect(0, DOTS_Y - 12, SCREEN_WIDTH, 24, COLOR_INK);
    
    if (state == PIN_CHANGE_SUCCESS) {
        drawMessage("PIN CHANGED!", COLOR_SUCCESS);
    } else if (state == PIN_CHANGE_ERROR) {
        drawMessage(errorMessage != nullptr ? errorMessage : "ERROR!", COLOR_ERROR);
    } else {
        // Draw current prompt
        tft->setTextColor(COLOR_IVORY, COLOR_INK);
        tft->setTextSize(1);
        tft->setTextDatum(TC_DATUM);
        tft->drawString(getPromptText(), SCREEN_WIDTH / 2, 42);
        
        // Draw indicator dots
        drawIndicatorDots();
    }
}

void ChangePINScreen::drawIndicatorDots() {
    int totalWidth = (DOT_COUNT * DOT_RADIUS * 2) + ((DOT_COUNT - 1) * DOT_SPACING);
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    for (int i = 0; i < DOT_COUNT; i++) {
        int x = startX + (i * (DOT_RADIUS * 2 + DOT_SPACING)) + DOT_RADIUS;
        int y = DOTS_Y;
        
        if (i < pinIndex) {
            tft->fillCircle(x, y, DOT_RADIUS, COLOR_BRASS);
        } else {
            tft->drawCircle(x, y, DOT_RADIUS, COLOR_LINE);
            tft->fillCircle(x, y, DOT_RADIUS - 2, COLOR_INK);
        }
    }
}

void ChangePINScreen::drawKeypad() {
    int keypadWidth = (KEY_SIZE * 3) + (KEY_SPACING * 2);
    int startX = (SCREEN_WIDTH - keypadWidth) / 2;
    int y = KEYPAD_START_Y;
    
    // Row 1: 1, 2, 3
    drawKey(startX, y, "1", false);
    drawKey(startX + KEY_SIZE + KEY_SPACING, y, "2", false);
    drawKey(startX + (KEY_SIZE + KEY_SPACING) * 2, y, "3", false);
    y += KEY_SIZE + KEY_SPACING;
    
    // Row 2: 4, 5, 6
    drawKey(startX, y, "4", false);
    drawKey(startX + KEY_SIZE + KEY_SPACING, y, "5", false);
    drawKey(startX + (KEY_SIZE + KEY_SPACING) * 2, y, "6", false);
    y += KEY_SIZE + KEY_SPACING;
    
    // Row 3: 7, 8, 9
    drawKey(startX, y, "7", false);
    drawKey(startX + KEY_SIZE + KEY_SPACING, y, "8", false);
    drawKey(startX + (KEY_SIZE + KEY_SPACING) * 2, y, "9", false);
    y += KEY_SIZE + KEY_SPACING;
    
    // Row 4: Backspace, 0
    drawKey(startX, y, "<", false);
    drawKey(startX + KEY_SIZE + KEY_SPACING, y, "0", false);
}

void ChangePINScreen::drawKey(int x, int y, const char* label, bool pressed) {
    int centerX = x + KEY_SIZE / 2;
    int centerY = y + KEY_SIZE / 2;
    int radius = KEY_SIZE / 2 - 2;
    
    if (pressed) {
        tft->fillCircle(centerX, centerY, radius, COLOR_BRASS);
        tft->setTextColor(COLOR_INK, COLOR_BRASS);
    } else {
        tft->fillCircle(centerX, centerY, radius, COLOR_SURFACE);
        tft->drawCircle(centerX, centerY, radius, COLOR_LINE);
        tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
    }
    
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(label, centerX, centerY);
}

void ChangePINScreen::drawMessage(const char* message, uint16_t color) {
    // Draw large centered message
    tft->setTextColor(color, COLOR_INK);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(message, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

void ChangePINScreen::handleTouch(const TouchPoint& point) {
    // Back button (top-left corner)
    if (point.y < 40 && point.x < 60) {
        needsExit = true;
        return;
    }
    
    // Don't process keypad if showing message
    if (state == PIN_CHANGE_SUCCESS || state == PIN_CHANGE_ERROR) {
        return;
    }
    
    int keyIndex = getKeyAtPosition(point.x, point.y);
    
    if (keyIndex >= 0 && keyIndex <= 9) {
        addDigit('0' + keyIndex);
    } else if (keyIndex == -2) {
        removeDigit();
    }
    
    screenIsDirty = true;
}

int ChangePINScreen::getKeyAtPosition(int x, int y) {
    int keypadWidth = (KEY_SIZE * 3) + (KEY_SPACING * 2);
    int startX = (SCREEN_WIDTH - keypadWidth) / 2;
    int startY = KEYPAD_START_Y;
    
    if (y < startY || y > startY + (KEY_SIZE + KEY_SPACING) * 4) {
        return -1;
    }
    
    int row = (y - startY) / (KEY_SIZE + KEY_SPACING);
    if (row > 3) row = 3;
    
    int col = (x - startX) / (KEY_SIZE + KEY_SPACING);
    if (col < 0 || col > 2) {
        return -1;
    }
    
    // Check circular hit test
    int keyX = startX + col * (KEY_SIZE + KEY_SPACING);
    int keyY = startY + row * (KEY_SIZE + KEY_SPACING);
    int centerX = keyX + KEY_SIZE / 2;
    int centerY = keyY + KEY_SIZE / 2;
    int radius = KEY_SIZE / 2 - 2;
    
    int dx = x - centerX;
    int dy = y - centerY;
    if (dx * dx + dy * dy > radius * radius) {
        return -1;
    }
    
    if (row == 0) return col + 1;
    else if (row == 1) return col + 4;
    else if (row == 2) return col + 7;
    else if (row == 3) {
        if (col == 0) return -2;  // Backspace
        else if (col == 1) return 0;
    }
    
    return -1;
}

void ChangePINScreen::addDigit(char digit) {
    if (pinIndex >= 4) {
        return;
    }
    
    pinIndex++;
    
    Serial.printf("[CHANGE_PIN] Added digit, index now: %d\n", pinIndex);
    
    // Auto-validate when 4 digits entered
    if (pinIndex == 4) {
        // Build PIN string based on current state
        if (state == PIN_CHANGE_ENTER_CURRENT) {
            currentPIN[pinIndex - 1] = digit;
            currentPIN[4] = '\0';
            validateCurrentPIN();
        } else if (state == PIN_CHANGE_ENTER_NEW) {
            newPIN[pinIndex - 1] = digit;
            newPIN[4] = '\0';
            // Move to confirmation
            state = PIN_CHANGE_CONFIRM_NEW;
            clearCurrentEntry();
            forceFullRedraw = true;
        } else if (state == PIN_CHANGE_CONFIRM_NEW) {
            confirmPIN[pinIndex - 1] = digit;
            confirmPIN[4] = '\0';
            validateNewPIN();
        }
    } else {
        // Store digit
        if (state == PIN_CHANGE_ENTER_CURRENT) {
            currentPIN[pinIndex - 1] = digit;
        } else if (state == PIN_CHANGE_ENTER_NEW) {
            newPIN[pinIndex - 1] = digit;
        } else if (state == PIN_CHANGE_CONFIRM_NEW) {
            confirmPIN[pinIndex - 1] = digit;
        }
    }
    
    screenIsDirty = true;
}

void ChangePINScreen::removeDigit() {
    if (pinIndex == 0) {
        return;
    }
    
    pinIndex--;
    
    // Clear the digit
    if (state == PIN_CHANGE_ENTER_CURRENT) {
        currentPIN[pinIndex] = '\0';
    } else if (state == PIN_CHANGE_ENTER_NEW) {
        newPIN[pinIndex] = '\0';
    } else if (state == PIN_CHANGE_CONFIRM_NEW) {
        confirmPIN[pinIndex] = '\0';
    }
    
    Serial.printf("[CHANGE_PIN] Removed digit, index now: %d\n", pinIndex);
    
    screenIsDirty = true;
}

void ChangePINScreen::validateCurrentPIN() {
    Serial.printf("[CHANGE_PIN] Validating current PIN: %s\n", currentPIN);
    
    if (pinManager->validatePIN(currentPIN)) {
        Serial.println("[CHANGE_PIN] Current PIN correct - proceed to new PIN");
        state = PIN_CHANGE_ENTER_NEW;
        clearCurrentEntry();
        forceFullRedraw = true;
    } else {
        Serial.println("[CHANGE_PIN] Current PIN incorrect");
        errorMessage = "Wrong PIN!";
        state = PIN_CHANGE_ERROR;
        messageDisplayStart = millis();
        forceFullRedraw = true;
    }
}

void ChangePINScreen::validateNewPIN() {
    Serial.printf("[CHANGE_PIN] New PIN: %s, Confirm: %s\n", newPIN, confirmPIN);
    
    if (strcmp(newPIN, confirmPIN) != 0) {
        Serial.println("[CHANGE_PIN] PINs don't match");
        errorMessage = "PINs Don't Match!";
        state = PIN_CHANGE_ERROR;
        messageDisplayStart = millis();
        memset(newPIN, 0, sizeof(newPIN));
        memset(confirmPIN, 0, sizeof(confirmPIN));
        forceFullRedraw = true;
        return;
    }
    
    // PINs match - save new PIN
    if (pinManager->changePIN(currentPIN, newPIN)) {
        Serial.println("[CHANGE_PIN] PIN changed successfully!");
        state = PIN_CHANGE_SUCCESS;
        messageDisplayStart = millis();
        forceFullRedraw = true;
    } else {
        Serial.println("[CHANGE_PIN] Failed to save new PIN");
        errorMessage = "Save Failed!";
        state = PIN_CHANGE_ERROR;
        messageDisplayStart = millis();
        forceFullRedraw = true;
    }
}

void ChangePINScreen::clearCurrentEntry() {
    pinIndex = 0;
}

const char* ChangePINScreen::getPromptText() const {
    switch (state) {
        case PIN_CHANGE_ENTER_CURRENT:
            return "Enter Current PIN";
        case PIN_CHANGE_ENTER_NEW:
            return "Enter New PIN";
        case PIN_CHANGE_CONFIRM_NEW:
            return "Confirm New PIN";
        default:
            return "";
    }
}
