#ifndef TOUCHMANAGER_H
#define TOUCHMANAGER_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include "Config.h"

// Touch state machine states
enum TouchState {
    TOUCH_IDLE,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_UP
};

// Touch point structure with screen coordinates
struct TouchPoint {
    int16_t x;
    int16_t y;
    uint16_t pressure;
    bool valid;
};

// Forward declaration
class SettingsManager;

class TouchManager {
public:
    TouchManager(XPT2046_Touchscreen* touchDevice);
    
    // Initialize touch manager
    void begin();
    
    // Set settings manager for orientation awareness
    void setSettingsManager(SettingsManager* settingsMgr);
    
    // Update touch state (call in main loop)
    void update();
    
    // Get current touch state
    TouchState getState() const { return currentState; }
    
    // Get current touch point (screen coordinates)
    TouchPoint getPoint() const { return currentPoint; }
    
    // Check if touch is currently active
    bool isTouched() const { return currentState != TOUCH_IDLE; }
    
    // Update touch controller rotation (should match display rotation)
    void updateRotation(uint8_t rotation);
    
private:
    XPT2046_Touchscreen* touch;
    SettingsManager* settings;
    TouchState currentState;
    TouchPoint currentPoint;
    TouchPoint lastPoint;
    unsigned long lastTouchTime;
    unsigned long debounceDelay;
    uint8_t currentRotation;  // Track current rotation mode
    
    // Read and validate raw touch data
    bool readRawTouch(TouchPoint& point);
    
    // Map raw coordinates to screen coordinates
    void mapToScreen(TouchPoint& point);
    
    // Apply coordinate inversion for inverted orientation
    void applyOrientationTransform(TouchPoint& point);
    
    // Validate pressure to filter ghost touches
    bool isValidPressure(uint16_t pressure);
};

#endif // TOUCHMANAGER_H
