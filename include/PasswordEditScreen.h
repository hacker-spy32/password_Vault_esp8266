#ifndef PASSWORDEDITSCREEN_H
#define PASSWORDEDITSCREEN_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "TouchKeyboard.h"
#include "PasswordEntry.h"
#include "Config.h"

// Input field enumeration
enum EditInputField {
    EDIT_FIELD_NONE = 0,
    EDIT_FIELD_TITLE,
    EDIT_FIELD_USER,
    EDIT_FIELD_PASS
};

class PasswordEditScreen {
public:
    PasswordEditScreen(TFT_eSPI* display, TouchManager* touch);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Set which password to edit
    void setPasswordData(const PasswordEntry& entry);
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Exit management
    bool needsToExit() const { return needsExit; }
    bool wasPasswordSaved() const { return passwordSaved; }
    const PasswordEntry& getPasswordData() const { return entryData; }
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    TouchKeyboard* keyboard;
    
    // State
    bool needsExit;
    bool passwordSaved;
    bool forceFullRedraw;
    bool screenIsDirty;
    bool fieldContentChanged;
    bool keyboardVisibilityChanged;
    
    // Keyboard state
    EditInputField lastFocusedField;
    EditInputField focusedField;
    bool passwordVisible;
    bool keyboardActive;
    
    // Password data being edited
    PasswordEntry entryData;
    
    // Layout constants - pixel-perfect measurements for 320x240 screen
    static const int STATUS_BAR_HEIGHT = 18;
    static const int HEADER_HEIGHT = 28;
    static const int TOTAL_TOP_HEIGHT = 46;
    static const int FIELD_HEIGHT = 40;
    static const int FIELD_SPACING = 6;
    static const int FIELD_START_Y = 48;
    static const int SIDE_MARGIN = 12;
    static const int BUTTON_HEIGHT = 38;
    static const int BUTTON_BOTTOM_MARGIN = 8;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawInputField(int y, const char* label, const char* value, bool isFocused, bool maskValue);
    void drawActionButtons();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    bool isSaveButtonPressed(int16_t x, int16_t y) const;
    bool isCancelButtonPressed(int16_t x, int16_t y) const;
    EditInputField getFieldAtPosition(int16_t x, int16_t y) const;
    
    // Field editing
    void editField(EditInputField field);
    
    // Keyboard integration
    void activateKeyboardForField(EditInputField field);
    void deactivateKeyboard();
    char* getCurrentFieldBuffer();
    SuggestionContext getContextForField(EditInputField field);
    int getFieldYPosition(EditInputField field);
};

#endif
