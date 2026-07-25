#include "PasswordEditScreen.h"
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
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Danger/cancel

PasswordEditScreen::PasswordEditScreen(TFT_eSPI* tft, TouchManager* touchMgr)
    : display(tft),
      touch(touchMgr),
      keyboard(nullptr),
      needsExit(false),
      passwordSaved(false),
      forceFullRedraw(true),
      screenIsDirty(true),
      fieldContentChanged(false),
      keyboardVisibilityChanged(false),
      lastFocusedField(EDIT_FIELD_NONE),
      focusedField(EDIT_FIELD_NONE),
      passwordVisible(false),
      keyboardActive(false) {
    
    memset(&entryData, 0, sizeof(PasswordEntry));
    keyboard = new TouchKeyboard(tft, touchMgr);
}

void PasswordEditScreen::begin() {
    Serial.println("[PASSWORD_EDIT] Screen initialized");
    needsExit = false;
    passwordSaved = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    fieldContentChanged = false;
    keyboardVisibilityChanged = false;
    focusedField = EDIT_FIELD_NONE;
    lastFocusedField = EDIT_FIELD_NONE;
    passwordVisible = false;
    keyboardActive = false;
    
    if (keyboard) {
        keyboard->begin();
    }
}

void PasswordEditScreen::reset() {
    needsExit = false;
    passwordSaved = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    focusedField = EDIT_FIELD_NONE;
    passwordVisible = false;
    keyboardActive = false;
    
    if (keyboard) {
        keyboard->hide();
    }
}

void PasswordEditScreen::setPasswordData(const PasswordEntry& entry) {
    entryData = entry;
    forceFullRedraw = true;
    Serial.printf("[PASSWORD_EDIT] Editing password: %s\n", entryData.title);
}

void PasswordEditScreen::update() {
    if (keyboardActive && keyboard) {
        char* currentBuffer = getCurrentFieldBuffer();
        keyboard->update(currentBuffer);
    }
}

void PasswordEditScreen::draw() {
    if (forceFullRedraw) {
        display->fillScreen(COLOR_INK);
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
        keyboardVisibilityChanged = true;
    }
    
    if (screenIsDirty || fieldContentChanged || (focusedField != lastFocusedField)) {
        drawDynamicData();
        screenIsDirty = false;
        fieldContentChanged = false;
        lastFocusedField = focusedField;
    }
    
    if (keyboardActive && keyboard) {
        keyboard->draw(keyboardVisibilityChanged);
        keyboardVisibilityChanged = false;
    }
}

void PasswordEditScreen::drawStaticUI() {
    // Status bar
    display->fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_INK);
    display->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // Header
    display->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_INK);
    
    // Back arrow
    display->setTextColor(COLOR_IVORY, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    display->drawString("<", 16, STATUS_BAR_HEIGHT + 8);
    
    // Title
    display->setTextDatum(TC_DATUM);
    display->drawString("Edit Password", SCREEN_WIDTH / 2, STATUS_BAR_HEIGHT + 8);
    
    // Border separator
    display->drawFastHLine(0, TOTAL_TOP_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    display->setTextDatum(TL_DATUM);
    
    Serial.println("[PASSWORD_EDIT] Static UI drawn");
}

void PasswordEditScreen::drawDynamicData() {
    // Clear content area
    display->fillRect(0, TOTAL_TOP_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - TOTAL_TOP_HEIGHT, COLOR_INK);
    
    // Calculate field positions
    int titleFieldY = getFieldYPosition(EDIT_FIELD_TITLE);
    int userFieldY = getFieldYPosition(EDIT_FIELD_USER);
    int passFieldY = getFieldYPosition(EDIT_FIELD_PASS);
    
    // Draw visible fields
    if (titleFieldY >= 0) {
        drawInputField(titleFieldY, "TITLE", entryData.title, focusedField == EDIT_FIELD_TITLE, false);
    }
    if (userFieldY >= 0) {
        drawInputField(userFieldY, "USER", entryData.user, focusedField == EDIT_FIELD_USER, false);
    }
    if (passFieldY >= 0) {
        drawInputField(passFieldY, "PASS", entryData.pass, focusedField == EDIT_FIELD_PASS, !passwordVisible);
    }
    
    // Draw action buttons (hide when keyboard active)
    if (!keyboardActive) {
        drawActionButtons();
    }
}

void PasswordEditScreen::drawInputField(int y, const char* label, const char* value, bool isFocused, bool maskValue) {
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    if (y < 0) return;
    
    int actualFieldHeight = keyboardActive ? 44 : FIELD_HEIGHT;
    
    // Field container
    uint16_t borderColor = isFocused ? COLOR_BRASS : COLOR_LINE;
    display->fillRoundRect(SIDE_MARGIN, y, fieldWidth, actualFieldHeight, 6, COLOR_SURFACE);
    display->drawRoundRect(SIDE_MARGIN, y, fieldWidth, actualFieldHeight, 6, borderColor);
    
    if (isFocused) {
        display->drawRoundRect(SIDE_MARGIN + 1, y + 1, fieldWidth - 2, actualFieldHeight - 2, 6, borderColor);
    }
    
    // Label
    display->setTextColor(COLOR_BRASS, COLOR_SURFACE);
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->drawString(label, SIDE_MARGIN + 10, y + 4);
    
    // Value
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    
    if (value[0] == '\0') {
        display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
        display->drawString("...", SIDE_MARGIN + 10, y + 18);
    } else if (maskValue) {
        int len = strlen(value);
        String masked = "";
        for (int i = 0; i < len && i < 32; i++) {
            masked += "*";
        }
        display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        display->drawString(masked, SIDE_MARGIN + 10, y + 18);
    } else {
        display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        display->drawString(value, SIDE_MARGIN + 10, y + 18);
    }
    
    display->setTextDatum(TL_DATUM);
}

void PasswordEditScreen::drawActionButtons() {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_BOTTOM_MARGIN;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 12) / 2;
    
    int cancelX = SIDE_MARGIN;
    int saveX = SIDE_MARGIN + buttonWidth + 12;
    
    // Cancel button (coral outline)
    display->fillRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_INK);
    display->drawRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_CORAL);
    display->drawRoundRect(cancelX + 1, buttonY + 1, buttonWidth - 2, BUTTON_HEIGHT - 2, 6, COLOR_CORAL);
    display->setTextColor(COLOR_CORAL, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(MC_DATUM);
    display->drawString("Cancel", cancelX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    // Save button (brass filled)
    display->fillRoundRect(saveX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_BRASS);
    display->setTextColor(COLOR_BRASS_TEXT, COLOR_BRASS);
    display->setTextSize(2);
    display->drawString("Save", saveX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    display->setTextDatum(TL_DATUM);
}

void PasswordEditScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[PASSWORD_EDIT] Touch at (%d,%d), keyboardActive=%d\n", point.x, point.y, keyboardActive);
    
    // Handle keyboard touches
    if (keyboardActive && keyboard) {
        int keyCode = keyboard->handleTouch(point);
        if (keyCode != 0) {
            char* buffer = getCurrentFieldBuffer();
            if (buffer) {
                int len = strlen(buffer);
                
                if (keyCode == -1) {
                    // Backspace
                    if (len > 0) {
                        buffer[len - 1] = '\0';
                        fieldContentChanged = true;
                        keyboardVisibilityChanged = true;
                    }
                } else if (keyCode == -2 || keyCode == -5) {
                    // Enter/Done or Save
                    deactivateKeyboard();
                    focusedField = EDIT_FIELD_NONE;
                    keyboardVisibilityChanged = true;
                } else if (keyCode == -3) {
                    // Space
                    if (len < 31) {
                        buffer[len] = ' ';
                        buffer[len + 1] = '\0';
                        fieldContentChanged = true;
                        keyboardVisibilityChanged = true;
                    }
                } else if (keyCode == -4) {
                    // Mode change
                    keyboardVisibilityChanged = true;
                    forceFullRedraw = true;
                } else if (keyCode > 0 && keyCode < 256) {
                    // Regular character
                    if (len < 31) {
                        buffer[len] = (char)keyCode;
                        buffer[len + 1] = '\0';
                        fieldContentChanged = true;
                        keyboardVisibilityChanged = true;
                    }
                }
            }
            return;
        }
    }
    
    // Check back button
    if (isBackButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_EDIT] Back button pressed");
        if (keyboardActive) {
            deactivateKeyboard();
            focusedField = EDIT_FIELD_NONE;
            keyboardVisibilityChanged = true;
        } else {
            needsExit = true;
        }
        return;
    }
    
    // Check cancel button
    if (isCancelButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_EDIT] Cancel button pressed");
        needsExit = true;
        return;
    }
    
    // Check save button
    if (isSaveButtonPressed(point.x, point.y)) {
        Serial.println("[PASSWORD_EDIT] Save button pressed");
        passwordSaved = true;
        needsExit = true;
        return;
    }
    
    // Check input fields (only if keyboard not active)
    if (!keyboardActive) {
        EditInputField field = getFieldAtPosition(point.x, point.y);
        if (field != EDIT_FIELD_NONE) {
            Serial.printf("[PASSWORD_EDIT] Field %d selected\n", field);
            focusedField = field;
            screenIsDirty = true;
            editField(field);
        }
    }
}

bool PasswordEditScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    return (y >= STATUS_BAR_HEIGHT && y <= TOTAL_TOP_HEIGHT && x <= 60);
}

bool PasswordEditScreen::isSaveButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 12) / 2;
    int saveX = SIDE_MARGIN + buttonWidth + 12;
    
    return (x >= saveX && x <= saveX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

bool PasswordEditScreen::isCancelButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - 16;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - 12) / 2;
    int cancelX = SIDE_MARGIN;
    
    return (x >= cancelX && x <= cancelX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

EditInputField PasswordEditScreen::getFieldAtPosition(int16_t x, int16_t y) const {
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    int titleFieldY = FIELD_START_Y;
    int userFieldY = titleFieldY + FIELD_HEIGHT + FIELD_SPACING;
    int passFieldY = userFieldY + FIELD_HEIGHT + FIELD_SPACING;
    
    if (x >= SIDE_MARGIN && x <= SIDE_MARGIN + fieldWidth) {
        if (y >= titleFieldY && y <= titleFieldY + FIELD_HEIGHT) {
            return EDIT_FIELD_TITLE;
        } else if (y >= userFieldY && y <= userFieldY + FIELD_HEIGHT) {
            return EDIT_FIELD_USER;
        } else if (y >= passFieldY && y <= passFieldY + FIELD_HEIGHT) {
            return EDIT_FIELD_PASS;
        }
    }
    
    return EDIT_FIELD_NONE;
}

void PasswordEditScreen::editField(EditInputField field) {
    Serial.printf("[PASSWORD_EDIT] Activating keyboard for field %d\n", field);
    activateKeyboardForField(field);
    screenIsDirty = true;
}

void PasswordEditScreen::activateKeyboardForField(EditInputField field) {
    if (!keyboard) return;
    
    focusedField = field;
    keyboardActive = true;
    keyboardVisibilityChanged = true;
    
    SuggestionContext context = getContextForField(field);
    keyboard->show(context);
    
    forceFullRedraw = true;
}

void PasswordEditScreen::deactivateKeyboard() {
    if (!keyboard) return;
    
    keyboardActive = false;
    keyboardVisibilityChanged = true;
    keyboard->hide();
    
    forceFullRedraw = true;
}

char* PasswordEditScreen::getCurrentFieldBuffer() {
    switch (focusedField) {
        case EDIT_FIELD_TITLE:
            return entryData.title;
        case EDIT_FIELD_USER:
            return entryData.user;
        case EDIT_FIELD_PASS:
            return entryData.pass;
        default:
            return nullptr;
    }
}

SuggestionContext PasswordEditScreen::getContextForField(EditInputField field) {
    switch (field) {
        case EDIT_FIELD_TITLE:
            return CONTEXT_TITLE;
        case EDIT_FIELD_USER:
            return CONTEXT_EMAIL;
        case EDIT_FIELD_PASS:
            return CONTEXT_PASSWORD;
        default:
            return CONTEXT_NONE;
    }
}

int PasswordEditScreen::getFieldYPosition(EditInputField field) {
    if (!keyboardActive) {
        int titleFieldY = FIELD_START_Y;
        int userFieldY = titleFieldY + FIELD_HEIGHT + FIELD_SPACING;
        int passFieldY = userFieldY + FIELD_HEIGHT + FIELD_SPACING;
        
        switch (field) {
            case EDIT_FIELD_TITLE: return titleFieldY;
            case EDIT_FIELD_USER: return userFieldY;
            case EDIT_FIELD_PASS: return passFieldY;
            default: return FIELD_START_Y;
        }
    } else {
        int keyboardTop = 60;
        int compactFieldHeight = 48;
        int visibleY = keyboardTop - compactFieldHeight;
        
        if (field == focusedField) {
            return visibleY;
        } else {
            return -100; // Off screen
        }
    }
}
