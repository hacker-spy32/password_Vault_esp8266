#ifndef PINENTRYSCREEN_H
#define PINENTRYSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "PINManager.h"
#include "Config.h"

// PIN Entry Screen - Apple Watch-style PIN entry with 4 indicator dots
class PINEntryScreen {
public:
    PINEntryScreen(TFT_eSPI* display, TouchManager* touchMgr, PINManager* pinMgr);
    
    // Screen lifecycle
    void begin();
    void reset();
    
    // Update and render
    void update();
    void draw();
    
    // Touch handling
    void handleTouch(const TouchPoint& point);
    
    // State queries
    bool isUnlocked() const { return unlocked; }
    bool needsRedraw() const { return forceFullRedraw || screenIsDirty; }
    
    // Reset unlock state (for re-locking)
    void lock();
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    PINManager* pinManager;
    
    // Screen state
    bool forceFullRedraw;
    bool screenIsDirty;
    bool unlocked;
    bool showingError;  // Show shake animation on wrong PIN
    
    // PIN entry state
    char enteredPIN[5];  // 4 digits + null terminator
    uint8_t pinIndex;    // Current digit being entered (0-3)
    
    // Error animation - improved timing
    unsigned long errorAnimationStart;
    static const unsigned long ERROR_ANIMATION_DURATION = 350;  // 350ms shake
    static const unsigned long ERROR_DISPLAY_DURATION = 1500;   // 1500ms total (shake + text display)
    static const int SHAKE_AMPLITUDE = 6;   // Refined shake distance
    
    // Touch Debounce
    unsigned long lastTapTime;
    static const unsigned long TAP_DEBOUNCE_MS = 250;  // 250ms (0.25s) delay between registered keypad taps
    
    // Layout constants - Clean rectangular grid for 320x240 ILI9341
    static const int DOT_COUNT = 4;
    static const int DOT_RADIUS = 8;
    static const int DOT_SPACING = 36;
    static const int DOTS_Y = 32;  // Moved higher for better spacing
    
    // Feedback text area
    static const int FEEDBACK_Y = 64;
    
    // Rectangular keypad grid (3 cols x 4 rows)
    static const int KEYPAD_START_Y = 84;
    static const int KEY_WIDTH = 80;       // Wide rectangular buttons
    static const int KEY_HEIGHT = 34;      // Comfortable height
    static const int KEY_SPACING_X = 14;   // Horizontal spacing
    static const int KEY_SPACING_Y = 6;    // Vertical spacing
    
    // Auto-centered grid: Total width = 3*80 + 2*14 = 268px, Start X = (320-268)/2 = 26px
    static const int GRID_ORIGIN_X = 26;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawIndicatorDots();
    void drawKeypad();
    void drawKey(int x, int y, const char* label, bool pressed);
    
    // Touch handling
    int getKeyAtPosition(int x, int y);  // Returns -1 if no key, 0-9 for digits, -2 for backspace
    
    // PIN entry logic
    void addDigit(char digit);
    void removeDigit();
    void validatePIN();
    void showError();  // Trigger shake animation
    void clearPIN();
};

#endif // PINENTRYSCREEN_H
