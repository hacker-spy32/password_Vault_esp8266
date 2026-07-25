#ifndef TOUCHKEYBOARD_H
#define TOUCHKEYBOARD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Keyboard mode for different character sets
enum KeyboardMode {
    MODE_LOWERCASE,
    MODE_UPPERCASE,
    MODE_NUMBERS,
    MODE_SYMBOLS
};

// Suggestion context for auto-complete
enum SuggestionContext {
    CONTEXT_NONE,
    CONTEXT_TITLE,      // Service name suggestions
    CONTEXT_EMAIL,      // Email domain suggestions
    CONTEXT_PASSWORD    // No suggestions
};

// Suggestion chip data
struct SuggestionChip {
    char text[32];
    bool active;
};

class TouchKeyboard {
public:
    TouchKeyboard(TFT_eSPI* display, TouchManager* touch);
    
    // Initialize keyboard
    void begin();
    
    // Show keyboard at bottom of screen with context
    void show(SuggestionContext context);
    
    // Hide keyboard
    void hide();
    
    // Check if keyboard is visible
    bool isVisible() const { return visible; }
    
    // Get keyboard height (for field positioning)
    int getKeyboardHeight() const;
    
    // Update keyboard state (call in update loop)
    void update(const char* currentInput);
    
    // Draw keyboard (call in render loop) - now with dirty checking
    void draw(bool forceDraw = false);
    
    // Handle touch input, returns character or special code
    // Returns: ASCII character, or negative for special keys:
    // -1: Backspace, -2: Enter/Done, -3: Space, -4: Mode change, -5: Save button, 0: No input
    // Note: Backspace can be triggered from keyboard key OR from button bar
    int handleTouch(const TouchPoint& point);
    
    // Get active suggestions
    const SuggestionChip* getSuggestions() const { return suggestions; }
    int getSuggestionCount() const { return suggestionCount; }
    
    // Check if a suggestion was tapped (returns suggestion index or -1)
    int handleSuggestionTouch(const TouchPoint& point);
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    
    bool visible;
    KeyboardMode currentMode;
    SuggestionContext context;
    
    // Anti-flicker: Track if suggestions changed
    int lastSuggestionCount;
    char lastSuggestionText[3][32];
    bool suggestionsChanged;
    
    // Touch Debounce
    unsigned long lastTapTime;
    static const unsigned long KEYBOARD_DEBOUNCE_MS = 250;  // 250ms (0.25s) keyboard debounce
    
    // Suggestion engine
    SuggestionChip suggestions[3];  // Up to 3 suggestions
    int suggestionCount;
    
    // Keyboard layout constants - REVISED FOR 240px SCREEN HEIGHT WITH SAVE BUTTON
    static const int SUGGESTION_HEIGHT = 24;     // Suggestions row
    static const int KEYBOARD_START_Y = 60;      // Start Y position (needs gap from field)
    static const int KEY_ROWS = 4;
    static const int KEY_COLS = 7;
    static const int KEY_WIDTH = 44;
    static const int KEY_HEIGHT = 28;
    static const int KEY_SPACING = 2;
    static const int KEYBOARD_HEIGHT = 120;      // Just the keys (4 rows)
    static const int SAVE_BUTTON_HEIGHT = 32;    // Save button below keyboard
    static const int SAVE_BUTTON_MARGIN = 4;     // Gap between keyboard and save button
    // Total: 60 + 24 + 120 + 4 + 32 = 240px (exactly fits 240px screen)
    
    // ABCD layout (alphabetical instead of QWERTY)
    // Row 1: A B C D E F G
    // Row 2: H I J K L M N
    // Row 3: O P Q R S T U
    // Row 4: [SHIFT] @ [SPACE] V W X Y Z [⌫]
    
    // Mode state tracking
    bool shiftPressed;         // Track shift state
    bool capsLockActive;       // Future: caps lock toggle
    
    // Drawing methods
    void drawKey(int x, int y, int w, int h, const char* label, bool special = false);
    void drawSuggestions();
    
    // Layout helpers
    void getKeyPosition(int row, int col, int& x, int& y, int& w, int& h);
    int getKeyAtPosition(int16_t touchX, int16_t touchY);
    
    // Character mapping
    char getCharForKey(int keyIndex);
    
    // Suggestion engine
    void updateSuggestions(const char* input);
    void clearSuggestions();
    void addSuggestion(const char* text);
    
    // Title suggestions (popular services)
    void generateTitleSuggestions(const char* input);
    
    // Email domain suggestions
    void generateEmailSuggestions(const char* input);
};

#endif // TOUCHKEYBOARD_H
