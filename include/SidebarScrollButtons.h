#ifndef SIDEBARSCROLLBUTTONS_H
#define SIDEBARSCROLLBUTTONS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

/**
 * SidebarScrollButtons - Aesthetic Right-Side Vertical Scroll Navigation Bar
 * 
 * Provides sleek Up (▲) and Down (▼) buttons on the right edge of list screens.
 * Designed specifically for hardware displays where touch dragging is unreliable.
 */
class SidebarScrollButtons {
public:
    SidebarScrollButtons(TFT_eSPI* display);
    
    void begin();
    
    // Draw the sidebar panel and scroll buttons
    void draw(int currentScrollOffset, int maxScrollOffset);
    
    // Process touch events, returns true if scroll changed
    bool handleTouch(const TouchPoint& point, int& currentScrollOffset, int maxScrollOffset, int step = 50);
    
    // Check if touch point is within the sidebar region
    bool isTouchOnSidebar(int16_t x, int16_t y) const;

private:
    TFT_eSPI* tft;
    
    // Layout constants
    static const int SIDEBAR_X = 280;          // Left X edge of sidebar (320 - 40)
    static const int SIDEBAR_Y = 58;           // Top Y edge (below 56px header)
    static const int SIDEBAR_WIDTH = 36;       // Width of sidebar panel
    static const int SIDEBAR_HEIGHT = 174;     // Total height (232 - 58)
    
    static const int UP_BTN_Y = 60;            // Up button top Y
    static const int UP_BTN_HEIGHT = 82;       // Up button height
    
    static const int DOWN_BTN_Y = 146;         // Down button top Y
    static const int DOWN_BTN_HEIGHT = 82;     // Down button height
    
    // Colors (Unified Dark Theme)
    static const uint16_t COLOR_INK = 0x0841;       // #0A0C10
    static const uint16_t COLOR_SURFACE = 0x18C3;   // #141821
    static const uint16_t COLOR_LINE = 0x2945;      // #262C3A
    static const uint16_t COLOR_BRASS = 0xFD60;     // #E8B564
    static const uint16_t COLOR_IVORY = 0xFFDE;     // #F4EFE4
    static const uint16_t COLOR_MUTED = 0x8C51;     // #8A8FA3
    
    // Button state
    bool upPressed;
    bool downPressed;
    
    // Helper drawing methods
    void drawUpButton(bool enabled, bool pressed);
    void drawDownButton(bool enabled, bool pressed);
    void drawUpChevron(int centerX, int centerY, uint16_t color);
    void drawDownChevron(int centerX, int centerY, uint16_t color);
};

#endif // SIDEBARSCROLLBUTTONS_H
