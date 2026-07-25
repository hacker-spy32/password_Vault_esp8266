#include "TouchKeyboard.h"
#include <string.h>
#include <ctype.h>

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Key backgrounds
#define COLOR_LINE 0x2945         // #262C3A - Key borders
#define COLOR_BRASS 0xFD60        // #E8B564 - Special keys
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Secondary text
#define COLOR_CORAL 0xFB2D        // #FF6B6B - Delete/Danger actions

// Popular service names for title suggestions
const char* serviceNames[] = {
    "Amazon", "Apple", "Facebook", "Google", "Instagram",
    "LinkedIn", "Microsoft", "Netflix", "PayPal", "Spotify",
    "Twitter", "YouTube", "GitHub", "Dropbox", "Adobe"
};
const int serviceCount = 15;

// Popular email domains
const char* emailDomains[] = {
    "@gmail.com", "@icloud.com", "@yahoo.com", "@outlook.com",
    "@hotmail.com", "@protonmail.com"
};
const int domainCount = 6;

TouchKeyboard::TouchKeyboard(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      visible(false),
      currentMode(MODE_LOWERCASE),
      context(CONTEXT_NONE),
      lastSuggestionCount(0),
      suggestionsChanged(false),
      lastTapTime(0),
      suggestionCount(0),
      shiftPressed(false),
      capsLockActive(false) {
    
    // Initialize suggestions
    for (int i = 0; i < 3; i++) {
        suggestions[i].text[0] = '\0';
        suggestions[i].active = false;
        lastSuggestionText[i][0] = '\0';
    }
}

void TouchKeyboard::begin() {
    DEBUG_LOG("TouchKeyboard initialized");
    visible = false;
    currentMode = MODE_LOWERCASE;
    context = CONTEXT_NONE;
    suggestionCount = 0;
    lastSuggestionCount = 0;
    suggestionsChanged = false;
    lastTapTime = 0;
}

void TouchKeyboard::show(SuggestionContext ctx) {
    visible = true;
    context = ctx;
    currentMode = MODE_LOWERCASE;
    shiftPressed = false;
    capsLockActive = false;
    lastTapTime = 0;
    clearSuggestions();
    suggestionsChanged = true;  // Force initial draw
    
    DEBUG_LOGF("TouchKeyboard shown with context %d\n", ctx);
}

void TouchKeyboard::hide() {
    visible = false;
    clearSuggestions();
    suggestionsChanged = false;
    
    DEBUG_LOG("TouchKeyboard hidden");
}

int TouchKeyboard::getKeyboardHeight() const {
    // Total height includes suggestions + keyboard + save button
    return SUGGESTION_HEIGHT + KEYBOARD_HEIGHT + SAVE_BUTTON_MARGIN + SAVE_BUTTON_HEIGHT;
}

void TouchKeyboard::update(const char* currentInput) {
    if (!visible) return;
    
    // Store old suggestion state
    int oldCount = suggestionCount;
    
    // Update suggestions based on current input
    updateSuggestions(currentInput);
    
    // Check if suggestions actually changed
    if (suggestionCount != oldCount) {
        suggestionsChanged = true;
    } else {
        // Check if text changed
        suggestionsChanged = false;
        for (int i = 0; i < suggestionCount; i++) {
            if (strcmp(suggestions[i].text, lastSuggestionText[i]) != 0) {
                suggestionsChanged = true;
                break;
            }
        }
    }
    
    // Store current suggestions for next comparison
    lastSuggestionCount = suggestionCount;
    for (int i = 0; i < suggestionCount; i++) {
        strncpy(lastSuggestionText[i], suggestions[i].text, 31);
        lastSuggestionText[i][31] = '\0';
    }
}

void TouchKeyboard::draw(bool forceDraw) {
    if (!visible) return;
    
    // Only redraw if forced or suggestions changed
    if (forceDraw || suggestionsChanged) {
        // Draw suggestion chips at top
        drawSuggestions();
        suggestionsChanged = false;  // Clear flag after drawing
    }
    
    // Keyboard keys are static, only draw on forceDraw
    if (forceDraw) {
        // Draw keyboard below suggestions
        int keyboardY = KEYBOARD_START_Y + SUGGESTION_HEIGHT;
        
        // Draw keyboard background
        tft->fillRect(0, keyboardY, SCREEN_WIDTH, KEYBOARD_HEIGHT, COLOR_INK);
        
        // ========================================================================
        // ALPHABETICAL LAYOUT (ABCD instead of QWERTY)
        // ========================================================================
        
        bool isShiftActive = (currentMode == MODE_UPPERCASE) || shiftPressed;
        
        // Row 1: A B C D E F G
        const char* row1[] = {"A", "B", "C", "D", "E", "F", "G"};
        for (int i = 0; i < 7; i++) {
            int x, y, w, h;
            getKeyPosition(0, i, x, y, w, h);
            y += SUGGESTION_HEIGHT;  // Offset for suggestions
            // Display uppercase if shift active, lowercase otherwise
            char displayChar[2] = {isShiftActive ? row1[i][0] : (char)tolower(row1[i][0]), '\0'};
            drawKey(x, y, w, h, displayChar);
        }
        
        // Row 2: H I J K L M N
        const char* row2[] = {"H", "I", "J", "K", "L", "M", "N"};
        for (int i = 0; i < 7; i++) {
            int x, y, w, h;
            getKeyPosition(1, i, x, y, w, h);
            y += SUGGESTION_HEIGHT;
            char displayChar[2] = {isShiftActive ? row2[i][0] : (char)tolower(row2[i][0]), '\0'};
            drawKey(x, y, w, h, displayChar);
        }
        
        // Row 3: O P Q R S T U
        const char* row3[] = {"O", "P", "Q", "R", "S", "T", "U"};
        for (int i = 0; i < 7; i++) {
            int x, y, w, h;
            getKeyPosition(2, i, x, y, w, h);
            y += SUGGESTION_HEIGHT;
            char displayChar[2] = {isShiftActive ? row3[i][0] : (char)tolower(row3[i][0]), '\0'};
            drawKey(x, y, w, h, displayChar);
        }
        
        // Row 4: [SHIFT] @ [SPACE] V W X Y Z [⌫]
        // Special layout for bottom row with Shift and Backspace
        int bottomRowY = keyboardY + (KEY_HEIGHT + KEY_SPACING) * 3;
        
        // SHIFT key (left side) - shows if active
        tft->fillRoundRect(4, bottomRowY, KEY_WIDTH, KEY_HEIGHT, 4, 
                          isShiftActive ? COLOR_BRASS : COLOR_SURFACE);
        tft->drawRoundRect(4, bottomRowY, KEY_WIDTH, KEY_HEIGHT, 4, 
                          isShiftActive ? COLOR_INK : COLOR_LINE);
        tft->setTextColor(isShiftActive ? COLOR_INK : COLOR_IVORY, 
                         isShiftActive ? COLOR_BRASS : COLOR_SURFACE);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("SHIFT", 4 + KEY_WIDTH / 2, bottomRowY + KEY_HEIGHT / 2);
        tft->setTextDatum(TL_DATUM);
        
        // @ key
        int atX = 4 + KEY_WIDTH + KEY_SPACING;
        drawKey(atX, bottomRowY, KEY_WIDTH, KEY_HEIGHT, "@", true);
        
        // SPACE bar (wider, center)
        int spaceX = atX + KEY_WIDTH + KEY_SPACING;
        int spaceW = KEY_WIDTH * 2;
        drawKey(spaceX, bottomRowY, spaceW, KEY_HEIGHT, "SPACE", true);
        
        // V W X Y Z
        const char* row4[] = {"V", "W", "X", "Y", "Z"};
        int startX = spaceX + spaceW + KEY_SPACING;
        for (int i = 0; i < 5; i++) {
            int x = startX + i * (KEY_WIDTH + KEY_SPACING);
            const char* label = row4[i];
            // Display uppercase if shift is active
            char displayChar[2] = {isShiftActive ? label[0] : (char)tolower(label[0]), '\0'};
            drawKey(x, bottomRowY, KEY_WIDTH, KEY_HEIGHT, displayChar);
        }
        
        // Backspace key (right side) - prominent delete symbol with coral background
        int backX = startX + 5 * (KEY_WIDTH + KEY_SPACING);
        tft->fillRoundRect(backX, bottomRowY, KEY_WIDTH, KEY_HEIGHT, 4, COLOR_CORAL);
        tft->drawRoundRect(backX, bottomRowY, KEY_WIDTH, KEY_HEIGHT, 4, COLOR_LINE);
        tft->setTextColor(COLOR_IVORY, COLOR_CORAL);
        tft->setTextSize(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("<", backX + KEY_WIDTH / 2, bottomRowY + KEY_HEIGHT / 2);
        tft->setTextDatum(TL_DATUM);
        
        // ========================================================================
        // BUTTON BAR - Below keyboard (Backspace + Save buttons side by side)
        // ========================================================================
        int buttonBarY = keyboardY + KEYBOARD_HEIGHT + SAVE_BUTTON_MARGIN;
        int buttonBarWidth = SCREEN_WIDTH - 16;  // 8px margin on each side
        int buttonBarX = 8;
        int buttonGap = 8;
        int backspaceWidth = 80;  // Wide enough for "⌫ Delete" text
        int saveWidth = buttonBarWidth - backspaceWidth - buttonGap;
        
        // Backspace/Delete button (left side, coral)
        tft->fillRoundRect(buttonBarX, buttonBarY, backspaceWidth, SAVE_BUTTON_HEIGHT, 6, COLOR_CORAL);
        tft->setTextColor(COLOR_IVORY, COLOR_CORAL);
        tft->setTextSize(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("< Del", buttonBarX + backspaceWidth / 2, buttonBarY + SAVE_BUTTON_HEIGHT / 2);
        
        // Save button (right side, brass gradient)
        int saveButtonX = buttonBarX + backspaceWidth + buttonGap;
        tft->fillRoundRect(saveButtonX, buttonBarY, saveWidth, SAVE_BUTTON_HEIGHT, 6, COLOR_BRASS);
        
        // Subtle gradient simulation: darker brass on bottom half
        for (int i = 0; i < SAVE_BUTTON_HEIGHT / 2; i++) {
            int gradY = buttonBarY + SAVE_BUTTON_HEIGHT / 2 + i;
            uint16_t gradColor = (i % 2 == 0) ? COLOR_BRASS : 0xE4C0;  // COLOR_BRASS_DARK
            tft->drawFastHLine(saveButtonX + 6, gradY, saveWidth - 12, gradColor);
        }
        
        // Dark text on brass save button
        tft->setTextColor(0x10A2, COLOR_BRASS);  // COLOR_BRASS_TEXT (#1a1305)
        tft->setTextSize(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("Save", saveButtonX + saveWidth / 2, buttonBarY + SAVE_BUTTON_HEIGHT / 2);
        tft->setTextDatum(TL_DATUM);
    }
}

void TouchKeyboard::drawKey(int x, int y, int w, int h, const char* label, bool special) {
    // Draw key background
    uint16_t bgColor = special ? COLOR_BRASS : COLOR_SURFACE;
    uint16_t textColor = special ? COLOR_INK : COLOR_IVORY;
    
    tft->fillRoundRect(x, y, w, h, 4, bgColor);
    tft->drawRoundRect(x, y, w, h, 4, COLOR_LINE);
    
    // Draw label centered
    tft->setTextColor(textColor, bgColor);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(label, x + w / 2, y + h / 2);
    
    tft->setTextDatum(TL_DATUM);
}

void TouchKeyboard::drawSuggestions() {
    int suggestionY = KEYBOARD_START_Y;
    
    // Clear suggestion area
    tft->fillRect(0, suggestionY, SCREEN_WIDTH, SUGGESTION_HEIGHT, COLOR_INK);
    
    if (suggestionCount == 0) return;
    
    // Draw suggestion chips (up to 3)
    int chipWidth = 100;
    int chipSpacing = 8;
    int startX = (SCREEN_WIDTH - (chipWidth * suggestionCount + chipSpacing * (suggestionCount - 1))) / 2;
    
    for (int i = 0; i < suggestionCount; i++) {
        if (!suggestions[i].active) continue;
        
        int chipX = startX + i * (chipWidth + chipSpacing);
        
        // Draw chip background
        tft->fillRoundRect(chipX, suggestionY + 2, chipWidth, SUGGESTION_HEIGHT - 4, 6, COLOR_SURFACE);
        tft->drawRoundRect(chipX, suggestionY + 2, chipWidth, SUGGESTION_HEIGHT - 4, 6, COLOR_BRASS);
        
        // Draw suggestion text
        tft->setTextColor(COLOR_BRASS, COLOR_SURFACE);
        tft->setTextSize(1);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(suggestions[i].text, chipX + chipWidth / 2, suggestionY + SUGGESTION_HEIGHT / 2);
    }
    
    tft->setTextDatum(TL_DATUM);
}

void TouchKeyboard::getKeyPosition(int row, int col, int& x, int& y, int& w, int& h) {
    w = KEY_WIDTH;
    h = KEY_HEIGHT;
    
    // Calculate position with centering
    int totalWidth = KEY_COLS * KEY_WIDTH + (KEY_COLS - 1) * KEY_SPACING;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    x = startX + col * (KEY_WIDTH + KEY_SPACING);
    y = KEYBOARD_START_Y + row * (KEY_HEIGHT + KEY_SPACING);
}

int TouchKeyboard::handleTouch(const TouchPoint& point) {
    if (!visible) return 0;
    
    // Tap Debounce Filter
    unsigned long now = millis();
    if (lastTapTime > 0 && (now - lastTapTime < KEYBOARD_DEBOUNCE_MS)) {
        return 0; // Ignore rapid double-tap
    }
    
    Serial.printf("[KEYBOARD] handleTouch called: point(%d,%d), visible=%d\n", point.x, point.y, visible);
    
    int keyboardY = KEYBOARD_START_Y + SUGGESTION_HEIGHT;
    
    // ========================================================================
    // CHECK BUTTON BAR FIRST (below keyboard)
    // ========================================================================
    int buttonBarY = keyboardY + KEYBOARD_HEIGHT + SAVE_BUTTON_MARGIN;
    int buttonBarWidth = SCREEN_WIDTH - 16;
    int buttonBarX = 8;
    int buttonGap = 8;
    int backspaceWidth = 80;
    int saveWidth = buttonBarWidth - backspaceWidth - buttonGap;
    int saveButtonX = buttonBarX + backspaceWidth + buttonGap;
    
    Serial.printf("[KEYBOARD] ButtonBar Y range: %d-%d (touch Y=%d)\n", buttonBarY, buttonBarY + SAVE_BUTTON_HEIGHT, point.y);
    
    if (point.y >= buttonBarY && point.y <= buttonBarY + SAVE_BUTTON_HEIGHT) {
        Serial.printf("[KEYBOARD] Touch in button bar! X=%d\n", point.x);
        // Check backspace button (left)
        if (point.x >= buttonBarX && point.x <= buttonBarX + backspaceWidth) {
            Serial.println("[KEYBOARD] Backspace button pressed (button bar)");
            lastTapTime = now;
            return -1;  // Backspace code
        }
        // Check save button (right)
        Serial.printf("[KEYBOARD] Save button X range: %d-%d\n", saveButtonX, saveButtonX + saveWidth);
        if (point.x >= saveButtonX && point.x <= saveButtonX + saveWidth) {
            Serial.println("[KEYBOARD] Save button pressed (button bar)");
            lastTapTime = now;
            return -5;  // Save button code
        }
    }
    
    // ========================================================================
    // CHECK KEYBOARD AREA
    // ========================================================================
    // Check if touch is in keyboard area
    if (point.y < keyboardY || point.y >= keyboardY + KEYBOARD_HEIGHT) return 0;
    
    int localY = point.y - keyboardY;
    
    // Determine which row was touched
    int row = localY / (KEY_HEIGHT + KEY_SPACING);
    if (row < 0 || row >= KEY_ROWS) return 0;
    
    // Row 4 (bottom) has special layout with Shift and Backspace
    if (row == 3) {
        // SHIFT key (leftmost)
        if (point.x >= 4 && point.x <= 4 + KEY_WIDTH) {
            // Toggle shift mode
            if (currentMode == MODE_LOWERCASE) {
                currentMode = MODE_UPPERCASE;
                shiftPressed = true;
            } else {
                currentMode = MODE_LOWERCASE;
                shiftPressed = false;
            }
            DEBUG_LOGF("Shift toggled: mode=%d\n", currentMode);
            lastTapTime = now;
            return -4;  // Mode change code (triggers keyboard redraw)
        }
        
        // @ key
        int atX = 4 + KEY_WIDTH + KEY_SPACING;
        if (point.x >= atX && point.x <= atX + KEY_WIDTH) {
            lastTapTime = now;
            return '@';
        }
        
        // SPACE bar
        int spaceX = atX + KEY_WIDTH + KEY_SPACING;
        int spaceW = KEY_WIDTH * 2;
        if (point.x >= spaceX && point.x <= spaceX + spaceW) {
            lastTapTime = now;
            return -3;  // Space code
        }
        
        // V W X Y Z
        int startX = spaceX + spaceW + KEY_SPACING;
        for (int i = 0; i < 5; i++) {
            int keyX = startX + i * (KEY_WIDTH + KEY_SPACING);
            if (point.x >= keyX && point.x <= keyX + KEY_WIDTH) {
                char c = 'V' + i;
                
                // Return uppercase if shift active, lowercase otherwise
                bool shiftActive = (currentMode == MODE_UPPERCASE) || shiftPressed;
                char result = shiftActive ? c : tolower(c);
                
                // Auto-reset shift after character (one-shot shift)
                if (shiftPressed && currentMode == MODE_UPPERCASE) {
                    currentMode = MODE_LOWERCASE;
                    shiftPressed = false;
                }
                
                lastTapTime = now;
                return result;
            }
        }
        
        // Backspace (rightmost)
        int backX = startX + 5 * (KEY_WIDTH + KEY_SPACING);
        if (point.x >= backX && point.x <= backX + KEY_WIDTH) {
            lastTapTime = now;
            return -1;  // Backspace code
        }
        
        return 0;
    }
    
    // Calculate column for rows 0-2
    int totalWidth = KEY_COLS * KEY_WIDTH + (KEY_COLS - 1) * KEY_SPACING;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    if (point.x < startX || point.x > startX + totalWidth) return 0;
    
    int col = (point.x - startX) / (KEY_WIDTH + KEY_SPACING);
    if (col < 0 || col >= KEY_COLS) return 0;
    
    // Map to character
    int keyIndex = row * KEY_COLS + col;
    char c = 'A' + keyIndex;
    
    if (c > 'U') return 0;  // Only A-U in first 3 rows
    
    // Return uppercase if shift active, lowercase otherwise
    bool shiftActive = (currentMode == MODE_UPPERCASE) || shiftPressed;
    char result = shiftActive ? c : tolower(c);
    
    // Auto-reset shift after character (one-shot shift)
    if (shiftPressed && currentMode == MODE_UPPERCASE) {
        currentMode = MODE_LOWERCASE;
        shiftPressed = false;
    }
    
    lastTapTime = now;
    return result;
}

int TouchKeyboard::handleSuggestionTouch(const TouchPoint& point) {
    if (!visible || suggestionCount == 0) return -1;
    
    // Tap Debounce Filter
    unsigned long now = millis();
    if (lastTapTime > 0 && (now - lastTapTime < KEYBOARD_DEBOUNCE_MS)) {
        return -1; // Ignore rapid double-tap
    }
    
    int suggestionY = KEYBOARD_START_Y;
    
    // Check if touch is in suggestion area
    if (point.y < suggestionY || point.y > suggestionY + SUGGESTION_HEIGHT) {
        return -1;
    }
    
    // Calculate suggestion chip positions
    int chipWidth = 100;
    int chipSpacing = 8;
    int startX = (SCREEN_WIDTH - (chipWidth * suggestionCount + chipSpacing * (suggestionCount - 1))) / 2;
    
    for (int i = 0; i < suggestionCount; i++) {
        if (!suggestions[i].active) continue;
        
        int chipX = startX + i * (chipWidth + chipSpacing);
        
        if (point.x >= chipX && point.x <= chipX + chipWidth &&
            point.y >= suggestionY + 2 && point.y <= suggestionY + SUGGESTION_HEIGHT - 2) {
            lastTapTime = now;
            return i;  // Return suggestion index
        }
    }
    
    return -1;
}

char TouchKeyboard::getCharForKey(int keyIndex) {
    if (keyIndex < 0 || keyIndex > 25) return 0;
    
    char c = 'A' + keyIndex;
    return (currentMode == MODE_LOWERCASE) ? tolower(c) : c;
}

void TouchKeyboard::updateSuggestions(const char* input) {
    if (context == CONTEXT_NONE || context == CONTEXT_PASSWORD) {
        clearSuggestions();
        return;
    }
    
    if (context == CONTEXT_TITLE) {
        generateTitleSuggestions(input);
    } else if (context == CONTEXT_EMAIL) {
        generateEmailSuggestions(input);
    }
}

void TouchKeyboard::clearSuggestions() {
    for (int i = 0; i < 3; i++) {
        suggestions[i].text[0] = '\0';
        suggestions[i].active = false;
    }
    suggestionCount = 0;
}

void TouchKeyboard::addSuggestion(const char* text) {
    if (suggestionCount >= 3) return;
    
    strncpy(suggestions[suggestionCount].text, text, 31);
    suggestions[suggestionCount].text[31] = '\0';
    suggestions[suggestionCount].active = true;
    suggestionCount++;
}

void TouchKeyboard::generateTitleSuggestions(const char* input) {
    clearSuggestions();
    
    if (input == nullptr || strlen(input) < 2) return;
    
    // Find matching service names
    int inputLen = strlen(input);
    
    for (int i = 0; i < serviceCount && suggestionCount < 3; i++) {
        // Case-insensitive prefix match
        bool match = true;
        for (int j = 0; j < inputLen; j++) {
            if (tolower(input[j]) != tolower(serviceNames[i][j])) {
                match = false;
                break;
            }
        }
        
        if (match) {
            addSuggestion(serviceNames[i]);
        }
    }
}

void TouchKeyboard::generateEmailSuggestions(const char* input) {
    clearSuggestions();
    
    if (input == nullptr) return;
    
    // Check if user just typed @
    const char* atPos = strchr(input, '@');
    if (atPos != nullptr) {
        // Check if there's anything after @
        const char* afterAt = atPos + 1;
        int afterLen = strlen(afterAt);
        
        if (afterLen == 0) {
            // Just typed @, show all domain suggestions
            for (int i = 0; i < domainCount && i < 3; i++) {
                addSuggestion(emailDomains[i]);
            }
        } else {
            // Partial domain typed, filter suggestions
            for (int i = 0; i < domainCount && suggestionCount < 3; i++) {
                // Check if domain starts with what user typed after @
                const char* domain = emailDomains[i] + 1;  // Skip @ in domain
                
                bool match = true;
                for (int j = 0; j < afterLen; j++) {
                    if (tolower(afterAt[j]) != tolower(domain[j])) {
                        match = false;
                        break;
                    }
                }
                
                if (match) {
                    addSuggestion(emailDomains[i]);
                }
            }
        }
    }
}
