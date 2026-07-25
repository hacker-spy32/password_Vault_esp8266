#ifndef ADDPASSWORDMANUALSCREEN_H
#define ADDPASSWORDMANUALSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "TouchKeyboard.h"
#include "PasswordEntry.h"
#include "Config.h"

// Input field IDs
enum InputField {
    FIELD_NONE = -1,
    FIELD_TITLE = 0,
    FIELD_USER = 1,
    FIELD_PASS = 2
};

class AddPasswordManualScreen {
public:
    AddPasswordManualScreen(TFT_eSPI* tft, TouchManager* touchMgr);
    
    // Initialize screen
    void begin();
    
    // Update logic (NO DRAWING)
    void update();
    
    // Draw function (ONLY DRAWING) - THE STANDARD
    void draw();
    
    // Handle touch events (NO DRAWING)
    void onTouchEvent(const TouchPoint& point);
    
    // Check if user wants to exit/cancel
    bool needsToExit() const { return needsExit; }
    
    // Check if user saved the password
    bool wasPasswordSaved() const { return passwordSaved; }
    
    // Get the entered password data
    const PasswordEntry& getPasswordData() const { return entryData; }
    
    // Reset screen state
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    TouchKeyboard* keyboard;  // On-screen keyboard
    
    bool needsExit;
    bool passwordSaved;
    
    // Rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Anti-flicker: Track what changed
    bool fieldContentChanged;
    bool keyboardVisibilityChanged;
    InputField focusedField;
    InputField lastFocusedField;
    
    // Password entry data
    PasswordEntry entryData;
    
    // Password visibility toggle
    bool passwordVisible;
    
    // Keyboard state
    bool keyboardActive;
    
    // Layout constants - pixel-perfect measurements for 320x240 screen
    static const int STATUS_BAR_HEIGHT = 18;
    static const int HEADER_HEIGHT = 28;
    static const int TOTAL_TOP_HEIGHT = 46;   // Status bar + Header (18 + 28)
    static const int FIELD_HEIGHT = 40;
    static const int FIELD_SPACING = 6;
    static const int FIELD_START_Y = 48;
    static const int BUTTON_HEIGHT = 38;
    static const int BUTTON_SPACING = 12;
    static const int BUTTON_BOTTOM_MARGIN = 8;
    static const int SIDE_MARGIN = 12;
    
    // Dynamic field positioning (when keyboard is active)
    int getFieldYPosition(InputField field);
    
    // Keyboard integration
    void activateKeyboardForField(InputField field);
    void deactivateKeyboard();
    char* getCurrentFieldBuffer();
    SuggestionContext getContextForField(InputField field);
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawInputField(int y, const char* label, const char* value, bool isFocused, bool maskValue);
    void drawActionButtons();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    InputField getFieldAtPosition(int16_t x, int16_t y) const;
    bool isSaveButtonPressed(int16_t x, int16_t y) const;
    bool isCancelButtonPressed(int16_t x, int16_t y) const;
    
    // Field editing (placeholder - will need keyboard implementation)
    void editField(InputField field);
};

#endif // ADDPASSWORDMANUALSCREEN_H
