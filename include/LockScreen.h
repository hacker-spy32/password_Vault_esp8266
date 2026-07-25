#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

/**
 * LockScreen - Initial lock screen with "Swipe to Unlock"
 * Displays before PIN entry screen
 * Any touch unlocks and proceeds to PIN entry
 */
class LockScreen {
public:
    LockScreen(TFT_eSPI* display, TouchManager* touchMgr);
    
    // Lifecycle
    void begin();
    void reset();
    
    // Update and render
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // State
    bool shouldUnlock() const { return unlockRequested; }
    void clearUnlockRequest() { unlockRequested = false; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    
    // Screen state
    bool forceFullRedraw;
    bool screenIsDirty;
    bool unlockRequested;
    
    // Drawing methods
    void drawStaticUI();
};

#endif // LOCKSCREEN_H
