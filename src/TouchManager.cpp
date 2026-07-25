#include "TouchManager.h"
#include "SettingsManager.h"

TouchManager::TouchManager(XPT2046_Touchscreen* touchDevice) 
    : touch(touchDevice),
      settings(nullptr),
      currentState(TOUCH_IDLE),
      lastTouchTime(0),
      debounceDelay(50),
      currentRotation(1) {  // Default to landscape
    
    currentPoint = {0, 0, 0, false};
    lastPoint = {0, 0, 0, false};
}

void TouchManager::begin() {
    touch->begin();
    // Keep touch controller at base landscape orientation (1)
    // We'll manually transform coordinates for other orientations
    touch->setRotation(1);
    currentRotation = 1;
    currentState = TOUCH_IDLE;
    DEBUG_LOG("TouchManager initialized (base rotation 1, manual transforms for other modes)");
}

void TouchManager::setSettingsManager(SettingsManager* settingsMgr) {
    settings = settingsMgr;
    
    // Apply current orientation from settings
    if (settings != nullptr) {
        uint8_t orientation = settings->getOrientation();
        updateRotation(orientation);
    }
}

void TouchManager::updateRotation(uint8_t rotation) {
    currentRotation = rotation;
    // Keep touch controller at base rotation 1
    // Manual coordinate transformation will handle other orientations
    DEBUG_LOGF("TouchManager: Rotation tracking updated to %d (touch controller stays at 1)\n", rotation);
}

void TouchManager::update() {
    TouchPoint point;
    bool touchDetected = readRawTouch(point);
    
    unsigned long currentTime = millis();
    
    switch (currentState) {
        case TOUCH_IDLE:
            if (touchDetected) {
                currentState = TOUCH_DOWN;
                currentPoint = point;
                lastPoint = point;
                lastTouchTime = currentTime;
            }
            break;
            
        case TOUCH_DOWN:
            if (touchDetected) {
                // Check if touch has moved significantly
                if (abs(point.x - lastPoint.x) > 5 || abs(point.y - lastPoint.y) > 5) {
                    currentState = TOUCH_MOVE;
                }
                currentPoint = point;
                lastPoint = point;
            } else {
                // Touch released
                currentState = TOUCH_UP;
                lastTouchTime = currentTime;
            }
            break;
            
        case TOUCH_MOVE:
            if (touchDetected) {
                currentPoint = point;
                lastPoint = point;
            } else {
                // Touch released
                currentState = TOUCH_UP;
                lastTouchTime = currentTime;
            }
            break;
            
        case TOUCH_UP:
            // Return to idle after debounce delay
            if (currentTime - lastTouchTime > debounceDelay) {
                currentState = TOUCH_IDLE;
                currentPoint.valid = false;
            }
            break;
    }
}

bool TouchManager::readRawTouch(TouchPoint& point) {
    // Ensure TFT CS is HIGH before reading touch
    digitalWrite(TFT_CS, HIGH);
    
    // Check if touch controller detects a touch
    if (!touch->touched()) {
        point.valid = false;
        return false;
    }
    
    // Get raw touch data
    TS_Point p = touch->getPoint();
    
    // Validate pressure to filter ghost touches (reject 4095)
    if (!isValidPressure(p.z)) {
        point.valid = false;
        return false;
    }
    
    // Store pressure
    point.pressure = p.z;
    
    // Map raw touch sensor coordinates to screen coordinates using calibration
    point.x = map(p.x, TOUCH_CAL_X_MIN, TOUCH_CAL_X_MAX, 0, SCREEN_WIDTH);
    point.y = map(p.y, TOUCH_CAL_Y_MIN, TOUCH_CAL_Y_MAX, 0, SCREEN_HEIGHT);
    
    DEBUG_LOGF("Touch raw->mapped: sensor(%d,%d) -> screen(%d,%d) [pressure=%d]\n", 
               p.x, p.y, point.x, point.y, p.z);
    
    // Apply orientation transformation BEFORE constraining
    // This is where landscape inverted flips the coordinates
    applyOrientationTransform(point);
    
    // Constrain to screen bounds after transformation
    point.x = constrain(point.x, 0, SCREEN_WIDTH - 1);
    point.y = constrain(point.y, 0, SCREEN_HEIGHT - 1);
    
    point.valid = true;
    return true;
}

void TouchManager::applyOrientationTransform(TouchPoint& point) {
    // CRITICAL: For inverted landscape (rotation 3), invert the coordinates
    // This ensures touch registration matches the visual display
    // We check currentRotation which is synchronized with the display orientation
    
    if (currentRotation == 3) {  // ORIENTATION_LANDSCAPE_INVERTED (270°)
        int16_t originalX = point.x;
        int16_t originalY = point.y;
        
        // Invert both axes relative to screen dimensions
        point.x = SCREEN_WIDTH - 1 - point.x;
        point.y = SCREEN_HEIGHT - 1 - point.y;
        
        DEBUG_LOGF("Touch transform [rotation=%d]: (%d,%d) -> (%d,%d)\n", 
                   currentRotation, originalX, originalY, point.x, point.y);
    } else {
        DEBUG_LOGF("Touch passthrough [rotation=%d]: (%d,%d)\n", 
                   currentRotation, point.x, point.y);
    }
}

void TouchManager::mapToScreen(TouchPoint& point) {
    // Map raw coordinates to screen coordinates using calibration values
    point.x = map(point.x, TOUCH_CAL_X_MIN, TOUCH_CAL_X_MAX, 0, SCREEN_WIDTH);
    point.y = map(point.y, TOUCH_CAL_Y_MIN, TOUCH_CAL_Y_MAX, 0, SCREEN_HEIGHT);
    
    // Constrain to screen bounds
    point.x = constrain(point.x, 0, SCREEN_WIDTH - 1);
    point.y = constrain(point.y, 0, SCREEN_HEIGHT - 1);
}

bool TouchManager::isValidPressure(uint16_t pressure) {
    // Filter out ghost touches (4095) and invalid low pressure
    return (pressure >= TOUCH_MIN_PRESSURE && pressure <= TOUCH_MAX_PRESSURE);
}
