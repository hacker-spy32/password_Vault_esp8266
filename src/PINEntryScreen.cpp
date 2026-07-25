#include "PINEntryScreen.h"

// Design System Colors
#define COLOR_INK 0x0841          // #0A0C10 - Background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Borders and separators
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_STEEL 0x7BCF        // #778899 - Steel/silver accent
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Primary text
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
#define COLOR_ERROR 0xF800        // Red for errors

PINEntryScreen::PINEntryScreen(TFT_eSPI* display, TouchManager* touchMgr, PINManager* pinMgr)
    : tft(display),
      touch(touchMgr),
      pinManager(pinMgr),
      forceFullRedraw(true),
      screenIsDirty(true),
      unlocked(false),
      showingError(false),
      pinIndex(0),
      errorAnimationStart(0),
      lastTapTime(0) {
    memset(enteredPIN, 0, sizeof(enteredPIN));
}

void PINEntryScreen::begin() {
    Serial.println("========================================");
    Serial.println("[PIN_ENTRY] Screen initialized - RECTANGULAR GRID LAYOUT");
    Serial.printf("[PIN_ENTRY] Layout: KEY_WIDTH=%d, KEY_HEIGHT=%d, GRID_ORIGIN_X=%d\n",
                 KEY_WIDTH, KEY_HEIGHT, GRID_ORIGIN_X);
    Serial.printf("[PIN_ENTRY] Grid dimensions: %dx%d buttons, spacing: X=%d Y=%d\n",
                 3, 4, KEY_SPACING_X, KEY_SPACING_Y);
    Serial.printf("[PIN_ENTRY] Keypad bounds: X=%d-%d, Y=%d-%d\n",
                 GRID_ORIGIN_X,
                 GRID_ORIGIN_X + (3 * KEY_WIDTH + 2 * KEY_SPACING_X),
                 KEYPAD_START_Y,
                 KEYPAD_START_Y + (4 * KEY_HEIGHT + 3 * KEY_SPACING_Y));
    Serial.println("========================================");
    
    unlocked = false;
    showingError = false;
    pinIndex = 0;
    lastTapTime = 0;
    memset(enteredPIN, 0, sizeof(enteredPIN));
    forceFullRedraw = true;
    screenIsDirty = true;
}

void PINEntryScreen::reset() {
    Serial.println("========================================");
    Serial.println("[PIN_ENTRY] *** RESET CALLED ***");
    Serial.printf("[PIN_ENTRY] Previous state: unlocked=%s, pinIndex=%d\n", 
                 unlocked ? "YES" : "NO", pinIndex);
    
    unlocked = false;
    showingError = false;
    pinIndex = 0;
    lastTapTime = 0;
    memset(enteredPIN, 0, sizeof(enteredPIN));
    forceFullRedraw = true;
    screenIsDirty = true;
    
    Serial.println("[PIN_ENTRY] New state: unlocked=NO, pinIndex=0, forceFullRedraw=YES");
    Serial.println("[PIN_ENTRY] Screen ready for touch input");
    Serial.println("========================================");
}

void PINEntryScreen::lock() {
    Serial.println("[PIN_ENTRY] Locking device");
    reset();
}

void PINEntryScreen::update() {
    // Check if error animation is complete
    if (showingError && (millis() - errorAnimationStart) > ERROR_DISPLAY_DURATION) {
        Serial.println("[PIN_ERROR] Animation complete - resetting to empty state");
        showingError = false;
        clearPIN();  // Clear PIN after error animation
        forceFullRedraw = true;  // Force full redraw to clear error text
        screenIsDirty = true;
    } else if (showingError) {
        // Animation in progress - keep redrawing for smooth animation
        screenIsDirty = true;
    }
}

void PINEntryScreen::draw() {
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

void PINEntryScreen::drawStaticUI() {
    // Title
    tft->setTextColor(COLOR_IVORY, COLOR_INK);
    tft->setTextSize(1);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("ENTER MASTER PIN", SCREEN_WIDTH / 2, 12);
    
    // Draw keypad (static)
    drawKeypad();
    
    Serial.println("[PIN_ENTRY] Static UI drawn (rectangular grid keypad)");
}

void PINEntryScreen::drawDynamicData() {
    // Clear ONLY the indicator dots and error text area (don't touch keypad)
    // Dots are at Y=32, radius=8, so clear Y=24-40 (16px)
    // Error text at Y=64, size 1 (8px tall), so clear Y=60-72 (12px)
    // Total clear region: Y=24 to Y=72 (48px), well above keypad at Y=84
    tft->fillRect(0, 24, SCREEN_WIDTH, 48, COLOR_INK);
    
    // Draw indicator dots
    drawIndicatorDots();
    
    // Draw error message if showing error (only after shake completes)
    if (showingError) {
        unsigned long elapsed = millis() - errorAnimationStart;
        
        // Only show text after initial shake (first 350ms)
        if (elapsed >= ERROR_ANIMATION_DURATION) {
            tft->setTextColor(COLOR_ERROR, COLOR_INK);
            tft->setTextSize(1);
            tft->setTextDatum(TC_DATUM);
            tft->drawString("WRONG PASSWORD", SCREEN_WIDTH / 2, FEEDBACK_Y);
            Serial.println("[PIN_ERROR] Displaying 'WRONG PASSWORD' text");
        }
    }
    
    Serial.printf("[PIN_ENTRY_RENDER] Dynamic data drawn, pinIndex=%d, showingError=%s\n", 
                 pinIndex, showingError ? "YES" : "NO");
}

void PINEntryScreen::drawIndicatorDots() {
    int totalWidth = (DOT_COUNT * DOT_RADIUS * 2) + ((DOT_COUNT - 1) * DOT_SPACING);
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    Serial.printf("[PIN_DOTS] Drawing indicator dots: pinIndex=%d/%d, showingError=%s\n", 
                 pinIndex, DOT_COUNT, showingError ? "YES" : "NO");
    
    // Calculate shake offset if showing error (refined algorithm)
    int shakeOffset = 0;
    if (showingError) {
        unsigned long elapsed = millis() - errorAnimationStart;
        
        // Only shake during the first ERROR_ANIMATION_DURATION (350ms)
        if (elapsed < ERROR_ANIMATION_DURATION) {
            // Phase-based oscillation for rapid left-right shake
            int phase = (elapsed / 40) % 4;
            if (phase == 0) shakeOffset = -SHAKE_AMPLITUDE;
            else if (phase == 1) shakeOffset = SHAKE_AMPLITUDE;
            else if (phase == 2) shakeOffset = -3;
            else shakeOffset = 3;
            
            Serial.printf("[PIN_ERROR] Shake animation: elapsed=%lums, phase=%d, offset=%dpx\n", 
                         elapsed, phase, shakeOffset);
        }
    }
    
    for (int i = 0; i < DOT_COUNT; i++) {
        int x = startX + (i * (DOT_RADIUS * 2 + DOT_SPACING)) + DOT_RADIUS + shakeOffset;
        int y = DOTS_Y;
        
        if (i < pinIndex) {
            // Filled dot (digit entered)
            if (showingError) {
                tft->fillCircle(x, y, DOT_RADIUS, COLOR_ERROR);
                Serial.printf("[PIN_DOTS] Dot %d: FILLED (ERROR/RED) at (%d,%d)\n", i, x, y);
            } else {
                tft->fillCircle(x, y, DOT_RADIUS, COLOR_BRASS);
                Serial.printf("[PIN_DOTS] Dot %d: FILLED (BRASS) at (%d,%d)\n", i, x, y);
            }
        } else {
            // Empty dot (digit not entered)
            tft->drawCircle(x, y, DOT_RADIUS, COLOR_LINE);
            tft->fillCircle(x, y, DOT_RADIUS - 2, COLOR_INK);  // Hollow center
            Serial.printf("[PIN_DOTS] Dot %d: EMPTY at (%d,%d)\n", i, x, y);
        }
    }
}

void PINEntryScreen::drawKeypad() {
    // Modern rectangular grid layout (3 cols x 4 rows)
    // Row 1: 1 2 3
    // Row 2: 4 5 6
    // Row 3: 7 8 9
    // Row 4: C 0 <  (Clear, 0, Backspace)
    
    const char* keys[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "C", "0", "<"};
    
    for (int i = 0; i < 12; i++) {
        int col = i % 3;
        int row = i / 3;
        int bx = GRID_ORIGIN_X + col * (KEY_WIDTH + KEY_SPACING_X);
        int by = KEYPAD_START_Y + row * (KEY_HEIGHT + KEY_SPACING_Y);
        
        // Draw clean rounded rectangle button (modern UI aesthetic)
        tft->fillRoundRect(bx, by, KEY_WIDTH, KEY_HEIGHT, 6, COLOR_SURFACE);
        tft->drawRoundRect(bx, by, KEY_WIDTH, KEY_HEIGHT, 6, COLOR_LINE);
        
        // Center button label
        tft->setTextColor(COLOR_IVORY, COLOR_SURFACE);
        tft->setTextSize(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(keys[i], bx + KEY_WIDTH / 2, by + KEY_HEIGHT / 2);
    }
    
    Serial.println("[PIN_KEYPAD] Rectangular grid drawn: 3x4 buttons, 80x34px each");
}

void PINEntryScreen::handleTouch(const TouchPoint& point) {
    Serial.println("========================================");
    Serial.printf("[PIN_ENTRY_TOUCH] handleTouch called: x=%d, y=%d, valid=%s\n", 
                 point.x, point.y, point.valid ? "YES" : "NO");
    
    if (unlocked) {
        Serial.println("[PIN_ENTRY_TOUCH] *** BLOCKED: Already unlocked, ignoring touch ***");
        Serial.println("========================================");
        return;  // Already unlocked, ignore touches
    }
    
    if (showingError) {
        Serial.println("[PIN_ENTRY_TOUCH] *** BLOCKED: Error animation in progress, ignoring touch ***");
        Serial.println("========================================");
        return;  // Ignore input during error animation
    }
    
    int keyIndex = getKeyAtPosition(point.x, point.y);
    Serial.printf("[PIN_ENTRY_TOUCH] Key index detected: %d\n", keyIndex);
    
    if (keyIndex == -1) {
        Serial.println("[PIN_ENTRY_TOUCH] *** NO KEY HIT - touch outside button area ***");
        Serial.println("========================================");
        return;
    }
    
    // Tap Debounce Filter: reject touches within 180ms of previous key hit
    unsigned long now = millis();
    if (lastTapTime > 0 && (now - lastTapTime < TAP_DEBOUNCE_MS)) {
        Serial.printf("[PIN_ENTRY_TOUCH] *** DEBOUNCE: Ignoring rapid tap (%lu ms < %lu ms) ***\n", 
                      now - lastTapTime, TAP_DEBOUNCE_MS);
        Serial.println("========================================");
        return;
    }
    lastTapTime = now;
    
    if (keyIndex >= 0 && keyIndex <= 9) {
        // Digit key pressed (0-9)
        Serial.printf("[PIN_ENTRY_TOUCH] *** DIGIT KEY PRESSED: %d ***\n", keyIndex);
        addDigit('0' + keyIndex);
    } else if (keyIndex == -2) {
        // Backspace key pressed
        Serial.println("[PIN_ENTRY_TOUCH] *** BACKSPACE KEY PRESSED ***");
        removeDigit();
    } else if (keyIndex == -3) {
        // Clear key pressed
        Serial.println("[PIN_ENTRY_TOUCH] *** CLEAR KEY PRESSED ***");
        clearPIN();
        screenIsDirty = true;
        drawDynamicData();
    }
    
    Serial.println("========================================");
}

int PINEntryScreen::getKeyAtPosition(int x, int y) {
    Serial.printf("[PIN_HIT_TEST] Touch: (%d, %d), Grid bounds: X=%d-%d, Y=%d-%d\n",
                 x, y, 
                 GRID_ORIGIN_X, 
                 GRID_ORIGIN_X + (3 * KEY_WIDTH + 2 * KEY_SPACING_X),
                 KEYPAD_START_Y, 
                 KEYPAD_START_Y + (4 * KEY_HEIGHT + 3 * KEY_SPACING_Y));
    
    // Key mapping: 0-8 = digits 1-9, 9 = Clear, 10 = digit 0, 11 = Backspace
    const char* keys[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "C", "0", "<"};
    
    for (int i = 0; i < 12; i++) {
        int col = i % 3;
        int row = i / 3;
        int bx = GRID_ORIGIN_X + col * (KEY_WIDTH + KEY_SPACING_X);
        int by = KEYPAD_START_Y + row * (KEY_HEIGHT + KEY_SPACING_Y);
        
        // Check if touch falls within this button's rectangular bounds
        if (x >= bx && x <= (bx + KEY_WIDTH) && y >= by && y <= (by + KEY_HEIGHT)) {
            char keyChar = keys[i][0];
            Serial.printf("[PIN_HIT_TEST] MATCH: Button %d ('%c') at grid (%d,%d), bounds X=%d-%d Y=%d-%d\n",
                         i, keyChar, col, row, bx, bx + KEY_WIDTH, by, by + KEY_HEIGHT);
            
            // Return appropriate key code
            if (keyChar >= '0' && keyChar <= '9') {
                return keyChar - '0';  // Return 0-9 for digits
            } else if (keyChar == 'C') {
                return -3;  // Clear key
            } else if (keyChar == '<') {
                return -2;  // Backspace key
            }
        }
    }
    
    Serial.println("[PIN_HIT_TEST] REJECT: Touch outside all button areas");
    return -1;  // No key hit
}

void PINEntryScreen::addDigit(char digit) {
    if (pinIndex >= 4) {
        return;  // Already have 4 digits
    }
    
    enteredPIN[pinIndex++] = digit;
    enteredPIN[pinIndex] = '\0';  // Null terminate
    
    Serial.printf("[PIN_ENTRY] Added digit, PIN length now: %d\n", pinIndex);
    
    // CRITICAL: Immediately update visual feedback
    screenIsDirty = true;
    drawDynamicData();  // ← Force immediate redraw of indicator dots
    
    // Auto-validate when 4 digits entered
    if (pinIndex == 4) {
        validatePIN();
    }
}

void PINEntryScreen::removeDigit() {
    if (pinIndex == 0) {
        return;  // No digits to remove
    }
    
    pinIndex--;
    enteredPIN[pinIndex] = '\0';
    
    Serial.printf("[PIN_ENTRY] Removed digit, PIN length now: %d\n", pinIndex);
    
    // CRITICAL: Immediately update visual feedback
    screenIsDirty = true;
    drawDynamicData();  // ← Force immediate redraw of indicator dots
}

void PINEntryScreen::validatePIN() {
    Serial.printf("[PIN_ENTRY] Validating PIN: %s\n", enteredPIN);
    
    if (pinManager->validatePIN(enteredPIN)) {
        // Correct PIN
        Serial.println("[PIN_ENTRY] ✓ PIN CORRECT - Unlocking!");
        unlocked = true;
        screenIsDirty = true;
    } else {
        // Wrong PIN - trigger error animation
        Serial.println("[PIN_ENTRY] ✗ PIN INCORRECT - Starting shake animation + error text");
        showError();
        screenIsDirty = true;
    }
}

void PINEntryScreen::showError() {
    showingError = true;
    errorAnimationStart = millis();
    screenIsDirty = true;
    Serial.printf("[PIN_ERROR] Error state activated at t=%lu ms\n", errorAnimationStart);
    Serial.printf("[PIN_ERROR] Animation sequence:\n");
    Serial.printf("[PIN_ERROR]   0-400ms: Horizontal shake (amplitude=%dpx)\n", SHAKE_AMPLITUDE);
    Serial.printf("[PIN_ERROR]   400-1500ms: Display 'WRONG PASSWORD' text\n");
    Serial.printf("[PIN_ERROR]   1500ms+: Reset to empty state\n");
}

void PINEntryScreen::clearPIN() {
    pinIndex = 0;
    memset(enteredPIN, 0, sizeof(enteredPIN));
    Serial.println("[PIN_ENTRY] PIN cleared");
}
