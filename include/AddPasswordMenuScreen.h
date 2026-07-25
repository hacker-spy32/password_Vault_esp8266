#ifndef ADDPASSWORDMENUSCREEN_H
#define ADDPASSWORDMENUSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Add password method options
enum AddPasswordMethod {
    METHOD_NONE = -1,
    METHOD_MANUAL = 0,
    METHOD_WEBUI_CONTROL = 1  // Web UI server control (renamed from index 2 to 1)
};

class AddPasswordMenuScreen {
public:
    AddPasswordMenuScreen(TFT_eSPI* tft, TouchManager* touchMgr);
    
    // Initialize screen
    void begin();
    
    // Update logic (NO DRAWING)
    void update();
    
    // Draw function (ONLY DRAWING) - THE STANDARD
    void draw();
    
    // Handle touch events (NO DRAWING)
    void onTouchEvent(const TouchPoint& point);
    
    // Check if user wants to exit
    bool needsToExit() const { return needsExit; }
    
    // Get selected method
    AddPasswordMethod getSelectedMethod() const { return selectedMethod; }
    
    // Clear selection after handling
    void clearSelection() { selectedMethod = METHOD_NONE; }
    
    // Reset screen state
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    
    bool needsExit;
    AddPasswordMethod selectedMethod;
    
    // Rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Layout constants - pixel-perfect measurements
    static const int STATUS_BAR_HEIGHT = 24;
    static const int HEADER_HEIGHT = 32;
    static const int TOTAL_TOP_HEIGHT = 56;   // Status bar + Header
    static const int ROW_HEIGHT = 72;
    static const int ROW_SPACING = 12;
    static const int SIDE_MARGIN = 16;
    static const int CONTENT_START_Y = 64;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    int getMenuItemAtPosition(int16_t x, int16_t y) const;
};

#endif // ADDPASSWORDMENUSCREEN_H
