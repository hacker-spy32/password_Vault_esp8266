#ifndef CHANGEPINSCREEN_H
#define CHANGEPINSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "PINManager.h"
#include "Config.h"

// PIN Change States
enum ChangePINState {
    PIN_CHANGE_ENTER_CURRENT,    // Enter current PIN
    PIN_CHANGE_ENTER_NEW,         // Enter new PIN
    PIN_CHANGE_CONFIRM_NEW,       // Confirm new PIN
    PIN_CHANGE_SUCCESS,           // Success message
    PIN_CHANGE_ERROR              // Error message
};

// Change PIN Screen - Guided 3-step process for changing PIN
class ChangePINScreen {
public:
    ChangePINScreen(TFT_eSPI* display, TouchManager* touchMgr, PINManager* pinMgr);
    
    // Screen lifecycle
    void begin();
    void reset();
    
    // Update and render
    void update();
    void draw();
    
    // Touch handling
    void handleTouch(const TouchPoint& point);
    
    // State queries
    bool isComplete() const { return state == PIN_CHANGE_SUCCESS; }
    bool shouldExit() const { return needsExit; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    PINManager* pinManager;
    
    // Screen state
    bool forceFullRedraw;
    bool screenIsDirty;
    bool needsExit;
    
    ChangePINState state;
    
    // PIN entry state
    char currentPIN[5];
    char newPIN[5];
    char confirmPIN[5];
    uint8_t pinIndex;
    
    // Error/success message display
    unsigned long messageDisplayStart;
    static const unsigned long MESSAGE_DISPLAY_DURATION = 2000;  // 2 seconds
    const char* errorMessage;
    
    // Touch Debounce
    unsigned long lastTapTime;
    static const unsigned long TAP_DEBOUNCE_MS = 250;  // 250ms (0.25s) keypad debounce
    
    // Layout constants - Optimized for 320x240 screen with header
    static const int DOT_COUNT = 4;
    static const int DOT_RADIUS = 8;
    static const int DOT_SPACING = 24;
    static const int DOTS_Y = 58;          // Indicator dots position
    
    static const int KEYPAD_START_Y = 80;  // Keypad starts lower after header
    static const int KEY_SIZE = 36;        // Button size
    static const int KEY_SPACING = 5;      // Reduced spacing
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawIndicatorDots();
    void drawKeypad();
    void drawKey(int x, int y, const char* label, bool pressed);
    void drawMessage(const char* message, uint16_t color);
    
    // Touch handling
    int getKeyAtPosition(int x, int y);
    
    // PIN entry logic
    void addDigit(char digit);
    void removeDigit();
    void validateCurrentPIN();
    void validateNewPIN();
    void clearCurrentEntry();
    
    // Get current prompt text based on state
    const char* getPromptText() const;
};

#endif // CHANGEPINSCREEN_H
