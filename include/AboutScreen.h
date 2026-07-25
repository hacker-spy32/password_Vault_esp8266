#ifndef ABOUTSCREEN_H
#define ABOUTSCREEN_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

class AboutScreen {
public:
    AboutScreen(TFT_eSPI* display, TouchManager* touch);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Navigation
    bool needsToExit() const { return needsExit; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    
    // State
    bool needsExit;
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Layout constants
    static const int HEADER_HEIGHT = 56;
    static const int SIDE_MARGIN = 20;
    static const int LINE_HEIGHT = 18;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
};

#endif
