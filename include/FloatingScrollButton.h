#ifndef FLOATINGSCROLLBUTTON_H
#define FLOATINGSCROLLBUTTON_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Floating Action Button (FAB) for scroll navigation
// Renders a circular button with down arrow at bottom-right corner
class FloatingScrollButton {
public:
    FloatingScrollButton(TFT_eSPI* display);
    
    // Initialize the button
    void begin();
    
    // Update button state based on touch input
    void update(const TouchPoint& point, TouchState state);
    
    // Draw the button (call this LAST in your draw cycle)
    void draw();
    
    // Check if button was tapped (returns true once per tap)
    bool wasTapped();
    
    // Reset tap flag
    void clearTap();
    
    // Get button state for external rendering decisions
    bool isPressed() const { return buttonPressed; }
    
    // Check if touch point is on the button
    bool isTouchOnButton(int16_t x, int16_t y) const;
    
private:
    TFT_eSPI* tft;
    
    // Button geometry
    static const int BUTTON_RADIUS = 20;        // Circle radius
    static const int BUTTON_INSET_X = 15;       // Distance from right edge
    static const int BUTTON_INSET_Y = 15;       // Distance from bottom edge
    static const int CHEVRON_SIZE = 8;          // Half-width of V chevron
    
    // Button position (calculated from screen size)
    int buttonCenterX;
    int buttonCenterY;
    
    // Button state
    bool buttonPressed;      // Currently being pressed
    bool buttonWasTapped;    // Tap detected (consume with wasTapped())
    bool needsRedraw;        // Button needs redraw
    
    // Colors
    static const uint16_t COLOR_IDLE = TFT_WHITE;
    static const uint16_t COLOR_PRESSED = 0xFD60;  // #E8B564 (warm amber/gold)
    static const uint16_t COLOR_BORDER = TFT_DARKGREY;
    
    // Drawing methods
    void drawButton(uint16_t fillColor);
    void drawChevron(uint16_t color);
};

#endif // FLOATINGSCROLLBUTTON_H
