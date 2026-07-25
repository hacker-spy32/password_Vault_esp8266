#include "AddPasswordManualScreen.h"
#include <string.h>

// ============================================================================
// DESIGN SYSTEM - EXACT COLOR TOKENS (RGB565 format for TFT)
// ============================================================================
#define COLOR_INK 0x0841          // #0A0C10 - Base background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_BRASS_DARK 0xE4C0   // #c99a4e - Darker brass for gradient effect
#define COLOR_BRASS_TEXT 0x10A2   // #1a1305 - Dark text on brass buttons
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Danger/cancel actions

// ============================================================================
// LAYOUT CONSTANTS - EXACT PIXEL MEASUREMENTS
// ============================================================================
#define STATUS_BAR_HEIGHT 18      // Top status bar
#define HEADER_HEIGHT 28          // Navigation header
#define TOTAL_TOP_HEIGHT 46       // Status bar + Header (18 + 28)

#define FIELD_BORDER_RADIUS 6     // Rounded corners for field containers
#define FIELD_HEIGHT 40           // Height of each input field block
#define FIELD_SPACING 6           // Gap between fields
#define SIDE_MARGIN 12            // Left/right screen margins
#define FIELD_START_Y 48          // Where first field begins

#define BUTTON_HEIGHT 38          // Action button height
#define BUTTON_SPACING 12         // Gap between buttons
#define BUTTON_BOTTOM_MARGIN 8    // Margin from screen bottom

AddPasswordManualScreen::AddPasswordManualScreen(TFT_eSPI* tft, TouchManager* touchMgr)
    : display(tft),
      touch(touchMgr),
      keyboard(nullptr),
      needsExit(false),
      passwordSaved(false),
      forceFullRedraw(true),
      screenIsDirty(true),
      fieldContentChanged(false),
      keyboardVisibilityChanged(false),
      lastFocusedField(FIELD_NONE),
      focusedField(FIELD_NONE),
      passwordVisible(false),
      keyboardActive(false) {
    
    // Initialize empty password data
    memset(&entryData, 0, sizeof(PasswordEntry));
    
    // Create keyboard
    keyboard = new TouchKeyboard(tft, touchMgr);
}

void AddPasswordManualScreen::begin() {
    DEBUG_LOG("AddPasswordManualScreen initialized");
    needsExit = false;
    passwordSaved = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    fieldContentChanged = false;
    keyboardVisibilityChanged = false;
    focusedField = FIELD_NONE;
    lastFocusedField = FIELD_NONE;
    passwordVisible = false;
    keyboardActive = false;
    
    // Clear entry data
    memset(&entryData, 0, sizeof(PasswordEntry));
    
    // Initialize keyboard
    if (keyboard) {
        keyboard->begin();
    }
}

void AddPasswordManualScreen::reset() {
    needsExit = false;
    passwordSaved = false;
    forceFullRedraw = true;
    screenIsDirty = true;
    focusedField = FIELD_NONE;
    passwordVisible = false;
    keyboardActive = false;
    
    memset(&entryData, 0, sizeof(PasswordEntry));
    
    if (keyboard) {
        keyboard->hide();
    }
}

void AddPasswordManualScreen::update() {
    // Update keyboard with current input for suggestions
    if (keyboardActive && keyboard) {
        char* currentBuffer = getCurrentFieldBuffer();
        keyboard->update(currentBuffer);
    }
}

void AddPasswordManualScreen::draw() {
    // THE STANDARD: Unified render pattern with anti-flicker optimization
    if (forceFullRedraw) {
        display->fillScreen(COLOR_INK);  // Deep black background
        drawStaticUI();
        forceFullRedraw = false;
        screenIsDirty = true;
        keyboardVisibilityChanged = true;  // Force keyboard redraw after full clear
    }
    
    // Only redraw fields if content or focus changed
    if (screenIsDirty || fieldContentChanged || (focusedField != lastFocusedField)) {
        drawDynamicData();
        screenIsDirty = false;
        fieldContentChanged = false;
        lastFocusedField = focusedField;
    }
    
    // Draw keyboard with smart redraw
    if (keyboardActive && keyboard) {
        // Force draw on visibility change, otherwise let keyboard decide
        keyboard->draw(keyboardVisibilityChanged);
        keyboardVisibilityChanged = false;
    }
}

void AddPasswordManualScreen::drawStaticUI() {
    // ========================================================================
    // STATUS BAR - 24px height at top
    // ========================================================================
    display->fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_INK);
    
    // PassGuard brand dot (left side)
    display->fillCircle(12, 12, 3, COLOR_BRASS);
    
    // System status icons (right side)
    display->setTextColor(COLOR_MUTED, COLOR_INK);
    display->setTextSize(1);
    display->setTextDatum(TR_DATUM);
    display->drawString("WiFi", SCREEN_WIDTH - 12, 8);
    
    // ========================================================================
    // HEADER - 32px height with border-bottom separator
    // ========================================================================
    display->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_INK);
    
    // Left-pointing back arrow
    display->setTextColor(COLOR_IVORY, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    display->drawString("<", 16, STATUS_BAR_HEIGHT + 8);
    
    // Title 'Add Password' - centered
    display->setTextColor(COLOR_IVORY, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(TC_DATUM);
    display->drawString("Add Password", SCREEN_WIDTH / 2, STATUS_BAR_HEIGHT + 8);
    
    // Border-bottom separator
    display->drawFastHLine(0, TOTAL_TOP_HEIGHT - 1, SCREEN_WIDTH, COLOR_LINE);
    
    // Reset text datum
    display->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPasswordManual - Static UI drawn (Pixel-Perfect)");
}

void AddPasswordManualScreen::drawDynamicData() {
    // Clear content area (below header)
    display->fillRect(0, TOTAL_TOP_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - TOTAL_TOP_HEIGHT, COLOR_INK);
    
    // Calculate field positions (dynamic when keyboard is active)
    int titleFieldY = getFieldYPosition(FIELD_TITLE);
    int userFieldY = getFieldYPosition(FIELD_USER);
    int passFieldY = getFieldYPosition(FIELD_PASS);
    
    // Draw visible fields
    if (titleFieldY >= 0) {
        drawInputField(titleFieldY, "TITLE", entryData.title, focusedField == FIELD_TITLE, false);
    }
    if (userFieldY >= 0) {
        drawInputField(userFieldY, "USER", entryData.user, focusedField == FIELD_USER, false);
    }
    if (passFieldY >= 0) {
        drawInputField(passFieldY, "PASS", entryData.pass, focusedField == FIELD_PASS, !passwordVisible);
    }
    
    // Draw action buttons (hide when keyboard is active)
    if (!keyboardActive) {
        drawActionButtons();
    }
    
    DEBUG_LOG("AddPasswordManual - Pixel-perfect rendering complete");
}

void AddPasswordManualScreen::drawInputField(int y, const char* label, const char* value, bool isFocused, bool maskValue) {
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    // If y is negative or will be cut off, skip drawing (field is off-screen)
    if (y < 0) return;
    
    // When keyboard is active, use compact field height
    int actualFieldHeight = keyboardActive ? 44 : FIELD_HEIGHT;  // 44px when keyboard active, 40px normally
    
    // ========================================================================
    // FIELD CONTAINER - Surface background with Line border
    // ========================================================================
    uint16_t borderColor = isFocused ? COLOR_BRASS : COLOR_LINE;
    display->fillRoundRect(SIDE_MARGIN, y, fieldWidth, actualFieldHeight, FIELD_BORDER_RADIUS, COLOR_SURFACE);
    display->drawRoundRect(SIDE_MARGIN, y, fieldWidth, actualFieldHeight, FIELD_BORDER_RADIUS, borderColor);
    
    if (isFocused) {
        // Double border effect for focused state
        display->drawRoundRect(SIDE_MARGIN + 1, y + 1, fieldWidth - 2, actualFieldHeight - 2, FIELD_BORDER_RADIUS, borderColor);
    }
    
    // ========================================================================
    // UPPERCASE BRASS-COLORED LABEL (small, at top of field)
    // ========================================================================
    display->setTextColor(COLOR_BRASS, COLOR_SURFACE);
    display->setTextSize(1);
    display->setTextDatum(TL_DATUM);
    display->drawString(label, SIDE_MARGIN + 10, y + 4);
    
    // ========================================================================
    // INPUT VALUE OR PLACEHOLDER (monospace style, below label)
    // ========================================================================
    display->setTextSize(2);
    display->setTextDatum(TL_DATUM);
    
    if (value[0] == '\0') {
        // Empty - show muted dots placeholder
        display->setTextColor(COLOR_MUTED, COLOR_SURFACE);
        display->drawString("...", SIDE_MARGIN + 10, y + 18);
    } else if (maskValue) {
        // Masked password - use bullet/dot characters (monospace style)
        int len = strlen(value);
        String masked = "";
        for (int i = 0; i < len && i < 32; i++) {
            masked += "*";  // Asterisk for password masking
        }
        display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        display->drawString(masked, SIDE_MARGIN + 10, y + 18);
    } else {
        // Normal text - monospace-style rendering
        display->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        display->drawString(value, SIDE_MARGIN + 10, y + 18);
    }
    
    display->setTextDatum(TL_DATUM);
}

void AddPasswordManualScreen::drawActionButtons() {
    // ========================================================================
    // BUTTON FOOTER - Side-by-side at bottom
    // ========================================================================
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_BOTTOM_MARGIN;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - BUTTON_SPACING) / 2;
    
    int cancelX = SIDE_MARGIN;
    int saveX = SIDE_MARGIN + buttonWidth + BUTTON_SPACING;
    
    // ========================================================================
    // CANCEL BUTTON - Coral outlined danger button (left side)
    // ========================================================================
    // Outline style: transparent/ink fill with coral border
    display->fillRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_INK);
    display->drawRoundRect(cancelX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_CORAL);
    display->drawRoundRect(cancelX + 1, buttonY + 1, buttonWidth - 2, BUTTON_HEIGHT - 2, 6, COLOR_CORAL);
    
    display->setTextColor(COLOR_CORAL, COLOR_INK);
    display->setTextSize(2);
    display->setTextDatum(MC_DATUM);
    display->drawString("Cancel", cancelX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    // ========================================================================
    // SAVE BUTTON - Solid brass gradient with dark text (right side)
    // ========================================================================
    // Gradient effect simulated with solid brass
    display->fillRoundRect(saveX, buttonY, buttonWidth, BUTTON_HEIGHT, 6, COLOR_BRASS);
    
    // Subtle gradient simulation: darker brass on bottom half
    for (int i = 0; i < BUTTON_HEIGHT / 2; i++) {
        int gradY = buttonY + BUTTON_HEIGHT / 2 + i;
        uint16_t gradColor = (i % 2 == 0) ? COLOR_BRASS : COLOR_BRASS_DARK;
        display->drawFastHLine(saveX + 6, gradY, buttonWidth - 12, gradColor);
    }
    
    // Dark text on brass button (#1a1305)
    display->setTextColor(COLOR_BRASS_TEXT, COLOR_BRASS);
    display->setTextSize(2);
    display->setTextDatum(MC_DATUM);
    display->drawString("Save", saveX + buttonWidth / 2, buttonY + BUTTON_HEIGHT / 2);
    
    display->setTextDatum(TL_DATUM);
}

void AddPasswordManualScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[ADD_MANUAL] Touch at (%d,%d), keyboardActive=%d\n", point.x, point.y, keyboardActive);
    
    // If keyboard is active, handle keyboard/suggestion touches first
    if (keyboardActive && keyboard) {
        // Check suggestion touches
        int suggestionIndex = keyboard->handleSuggestionTouch(point);
        if (suggestionIndex >= 0) {
            // User tapped a suggestion
            const SuggestionChip* suggestions = keyboard->getSuggestions();
            if (suggestions[suggestionIndex].active) {
                char* buffer = getCurrentFieldBuffer();
                if (buffer) {
                    // For title suggestions, replace entire field
                    if (focusedField == FIELD_TITLE) {
                        strncpy(buffer, suggestions[suggestionIndex].text, 31);
                        buffer[31] = '\0';
                    }
                    // For email suggestions, append domain
                    else if (focusedField == FIELD_USER) {
                        // Find @ position
                        char* atPos = strchr(buffer, '@');
                        if (atPos) {
                            // Replace from @ onward
                            strncpy(atPos, suggestions[suggestionIndex].text, 31 - (atPos - buffer));
                            buffer[31] = '\0';
                        }
                    }
                    
                    fieldContentChanged = true;
                    keyboardVisibilityChanged = true;  // Update keyboard/suggestions
                    DEBUG_LOGF("Applied suggestion: %s\n", suggestions[suggestionIndex].text);
                }
            }
            return;
        }
        
        // Check keyboard touches
        int keyCode = keyboard->handleTouch(point);
        Serial.printf("[ADD_MANUAL] Keyboard returned keyCode=%d\n", keyCode);
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
                } else if (keyCode == -2) {
                    // Enter/Done - close keyboard
                    deactivateKeyboard();
                    focusedField = FIELD_NONE;
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
                    // Mode change (Shift toggled) - force keyboard redraw
                    keyboardVisibilityChanged = true;
                    forceFullRedraw = true;  // Redraw entire keyboard with new case
                } else if (keyCode == -5) {
                    // Save button pressed on keyboard - close keyboard and return to form
                    // This just saves the current field and returns to the form view
                    Serial.println("[ADD_MANUAL] Keyboard Save button - closing keyboard");
                    deactivateKeyboard();
                    focusedField = FIELD_NONE;
                    keyboardVisibilityChanged = true;
                    return;
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
        DEBUG_LOG("AddPasswordManual: Back button pressed");
        if (keyboardActive) {
            deactivateKeyboard();
            focusedField = FIELD_NONE;
            keyboardVisibilityChanged = true;
        } else {
            needsExit = true;
        }
        return;
    }
    
    // Check cancel button
    if (isCancelButtonPressed(point.x, point.y)) {
        DEBUG_LOG("AddPasswordManual: Cancel button pressed");
        needsExit = true;
        return;
    }
    
    // Check save button
    if (isSaveButtonPressed(point.x, point.y)) {
        DEBUG_LOG("AddPasswordManual: Save button pressed");
        // TODO: Validate fields before saving
        passwordSaved = true;
        needsExit = true;
        return;
    }
    
    // Check input fields (only if keyboard not active)
    if (!keyboardActive) {
        InputField field = getFieldAtPosition(point.x, point.y);
        if (field != FIELD_NONE) {
            DEBUG_LOGF("AddPasswordManual: Field %d selected\n", field);
            focusedField = field;
            screenIsDirty = true;
            
            // Open keyboard for editing
            editField(field);
        }
    }
}

bool AddPasswordManualScreen::isBackButtonPressed(int16_t x, int16_t y) const {
    // Back arrow touch zone (left portion of header)
    bool result = (y >= STATUS_BAR_HEIGHT && y <= TOTAL_TOP_HEIGHT && x <= 60);
    
    if (result) {
        Serial.printf("[BACK_BTN] HIT at (%d,%d)\n", x, y);
    }
    
    return result;
}

InputField AddPasswordManualScreen::getFieldAtPosition(int16_t x, int16_t y) const {
    int fieldWidth = SCREEN_WIDTH - (SIDE_MARGIN * 2);
    
    int titleFieldY = FIELD_START_Y;
    int userFieldY = titleFieldY + FIELD_HEIGHT + FIELD_SPACING;
    int passFieldY = userFieldY + FIELD_HEIGHT + FIELD_SPACING;
    
    if (x >= SIDE_MARGIN && x <= SIDE_MARGIN + fieldWidth) {
        if (y >= titleFieldY && y <= titleFieldY + FIELD_HEIGHT) {
            return FIELD_TITLE;
        } else if (y >= userFieldY && y <= userFieldY + FIELD_HEIGHT) {
            return FIELD_USER;
        } else if (y >= passFieldY && y <= passFieldY + FIELD_HEIGHT) {
            return FIELD_PASS;
        }
    }
    
    return FIELD_NONE;
}

bool AddPasswordManualScreen::isSaveButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_BOTTOM_MARGIN;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - BUTTON_SPACING) / 2;
    int saveX = SIDE_MARGIN + buttonWidth + BUTTON_SPACING;
    
    Serial.printf("[SAVE_BTN] Check: touch(%d,%d) vs X=%d-%d, Y=%d-%d\n", 
               x, y, saveX, saveX + buttonWidth, buttonY, buttonY + BUTTON_HEIGHT);
    
    bool result = (x >= saveX && x <= saveX + buttonWidth &&
                   y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
    
    if (result) {
        Serial.println("[SAVE_BTN] HIT!");
    }
    
    return result;
}

bool AddPasswordManualScreen::isCancelButtonPressed(int16_t x, int16_t y) const {
    int buttonY = SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_BOTTOM_MARGIN;
    int buttonWidth = (SCREEN_WIDTH - (SIDE_MARGIN * 2) - BUTTON_SPACING) / 2;
    int cancelX = SIDE_MARGIN;
    
    return (x >= cancelX && x <= cancelX + buttonWidth &&
            y >= buttonY && y <= buttonY + BUTTON_HEIGHT);
}

void AddPasswordManualScreen::editField(InputField field) {
    // Activate keyboard for this field
    DEBUG_LOGF("Activating keyboard for field %d\n", field);
    activateKeyboardForField(field);
    screenIsDirty = true;
}

// ============================================================================
// KEYBOARD INTEGRATION METHODS
// ============================================================================

void AddPasswordManualScreen::activateKeyboardForField(InputField field) {
    if (!keyboard) return;
    
    focusedField = field;
    keyboardActive = true;
    keyboardVisibilityChanged = true;  // Mark keyboard visibility change
    
    // Show keyboard with appropriate context
    SuggestionContext context = getContextForField(field);
    keyboard->show(context);
    
    forceFullRedraw = true;  // Redraw with shifted fields
}

void AddPasswordManualScreen::deactivateKeyboard() {
    if (!keyboard) return;
    
    keyboardActive = false;
    keyboardVisibilityChanged = true;  // Mark keyboard visibility change
    keyboard->hide();
    
    forceFullRedraw = true;  // Restore normal layout
}

char* AddPasswordManualScreen::getCurrentFieldBuffer() {
    switch (focusedField) {
        case FIELD_TITLE:
            return entryData.title;
        case FIELD_USER:
            return entryData.user;
        case FIELD_PASS:
            return entryData.pass;
        default:
            return nullptr;
    }
}

SuggestionContext AddPasswordManualScreen::getContextForField(InputField field) {
    switch (field) {
        case FIELD_TITLE:
            return CONTEXT_TITLE;
        case FIELD_USER:
            return CONTEXT_EMAIL;
        case FIELD_PASS:
            return CONTEXT_PASSWORD;
        default:
            return CONTEXT_NONE;
    }
}

int AddPasswordManualScreen::getFieldYPosition(InputField field) {
    // When keyboard is active, shift fields up to keep focused field visible
    if (!keyboardActive) {
        // Normal positions
        int titleFieldY = FIELD_START_Y;
        int userFieldY = titleFieldY + FIELD_HEIGHT + FIELD_SPACING;
        int passFieldY = userFieldY + FIELD_HEIGHT + FIELD_SPACING;
        
        switch (field) {
            case FIELD_TITLE: return titleFieldY;
            case FIELD_USER: return userFieldY;
            case FIELD_PASS: return passFieldY;
            default: return FIELD_START_Y;
        }
    } else {
        // Keyboard is active - shift focused field to visible area above keyboard
        // Header ends at Y=56, Keyboard starts at Y=60
        // Use compact field height (48px instead of 64px) when keyboard active
        // Position: Y=60 - 48 = Y=12 (starts at Y=12, ends at Y=60)
        
        int keyboardTop = 60;  // Keyboard start (suggestions)
        int compactFieldHeight = 48;  // Reduced from 64px
        
        // Position field to end exactly where keyboard begins
        int visibleY = keyboardTop - compactFieldHeight;  // Y=60-48 = Y=12
        
        // Only show the focused field when keyboard is active
        if (field == focusedField) {
            return visibleY;
        } else {
            // Other fields off-screen or hidden
            return -100;  // Off screen
        }
    }
}
