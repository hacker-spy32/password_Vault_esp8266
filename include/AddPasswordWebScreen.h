#ifndef ADDPASSWORDWEBSCREEN_H
#define ADDPASSWORDWEBSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

class AddPasswordWebScreen {
public:
    AddPasswordWebScreen(TFT_eSPI* tft, TouchManager* touchMgr);
    
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
    
    // Reset screen state
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    
    bool needsExit;
    
    // Rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // UI constants
    static const int HEADER_HEIGHT = 40;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
};

#endif // ADDPASSWORDWEBSCREEN_H
