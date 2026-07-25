#include "CalibrationWizardScreen.h"

// External reference to touch controller
extern XPT2046_Touchscreen ts;

// Calibration point positions (screen coordinates)
#define MARGIN 30
#define CROSSHAIR_SIZE 20
#define CROSSHAIR_THICKNESS 2

// Back button position (top-right corner, same as TouchTest)
#define BACK_BTN_X 240
#define BACK_BTN_Y 0
#define BACK_BTN_W 80
#define BACK_BTN_H 40

// Colors
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_TEXT_DIM TFT_DARKGREY
#define COLOR_CROSSHAIR_RED TFT_RED
#define COLOR_CROSSHAIR_GREEN TFT_GREEN
#define COLOR_SUCCESS TFT_GREEN

CalibrationWizardScreen::CalibrationWizardScreen(TFT_eSPI* tft, TouchManager* touchMgr)
    : display(tft),
      touch(touchMgr),
      currentState(CAL_INTRO),
      currentPoint(0),
      samplesAtCurrentPoint(0),
      needsExit(false),  // Initialize exit flag
      stateStartTime(0),  // Initialize timer
      forceFullRedraw(true),  // Force initial draw
      screenIsDirty(true) {
    
    // Initialize calibration data
    calData.samplesCollected = 0;
    for (int i = 0; i < POINT_COUNT; i++) {
        calData.rawX[i] = 0;
        calData.rawY[i] = 0;
        calData.screenX[i] = 0;
        calData.screenY[i] = 0;
    }
}

void CalibrationWizardScreen::begin() {
    DEBUG_LOG("CalibrationWizard initialized - A1 Architecture");
    reset();
}

void CalibrationWizardScreen::reset() {
    currentState = CAL_INTRO;
    currentPoint = 0;
    samplesAtCurrentPoint = 0;
    calData.samplesCollected = 0;
    needsExit = false;  // Reset exit flag
    forceFullRedraw = true;
    screenIsDirty = true;
}

// ============================================================================
// STATE MANAGEMENT - Sets forceFullRedraw on state transitions
// ============================================================================

void CalibrationWizardScreen::setState(CalibrationState newState) {
    if (currentState != newState) {
        currentState = newState;
        stateStartTime = millis();  // Record state entry time for timers
        forceFullRedraw = true;  // THE STANDARD: force full redraw on transition
        screenIsDirty = true;     // Also ensure dynamic content renders
        
        DEBUG_LOGF("CalibrationWizard: setState -> %d (forceFullRedraw=true)\n", newState);
    }
}

// ============================================================================
// UPDATE METHOD - Data updates and logic (NO DRAWING)
// ============================================================================

void CalibrationWizardScreen::update() {
    // A2-STYLE TIMER: Handle POINT_FLASH state transition
    if (currentState == CAL_POINT_FLASH) {
        // Flash green for 300ms, then move to next point
        if (millis() - stateStartTime >= 300) {
            DEBUG_LOGF("Flash complete, advancing from point %d\n", currentPoint);
            
            // Move to next point
            currentPoint++;
            samplesAtCurrentPoint = 0;
            
            // Check if all points collected
            if (currentPoint >= POINT_COUNT) {
                calculateCalibration();
                setState(CAL_COMPLETE);
            } else {
                setState(CAL_CALIBRATING);
            }
        }
    }
}

// ============================================================================
// DRAW METHOD - THE STANDARD
// ============================================================================

void CalibrationWizardScreen::draw() {
    // THE STANDARD: Unified render pattern
    if (forceFullRedraw) {
        display->fillScreen(COLOR_BACKGROUND);
        
        // Draw static UI based on current state
        switch (currentState) {
            case CAL_INTRO:
                drawIntroStaticUI();
                break;
                
            case CAL_CALIBRATING:
            case CAL_POINT_FLASH:
                drawCalibratingStaticUI();  // Both use blank screen
                break;
                
            case CAL_COMPLETE:
                drawCompleteStaticUI();
                break;
        }
        
        forceFullRedraw = false;
        screenIsDirty = true;  // Force initial dynamic content draw
        
        DEBUG_LOG("CalibrationWizard: Static UI drawn");
    }
    
    if (screenIsDirty) {
        // Render dynamic content based on current state
        renderDynamicContent();
        
        screenIsDirty = false;
        
        DEBUG_LOG("CalibrationWizard: Dynamic content drawn");
    }
}

// ============================================================================
// STATIC UI DRAWING - Headers, buttons, instructions
// ============================================================================

void CalibrationWizardScreen::drawIntroStaticUI() {
    // Draw Back button in top-right
    drawBackButton();
    
    // Title
    display->setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    display->setTextSize(2);
    display->setCursor(40, 20);
    display->print("Touch Calibration");
    
    // Instructions
    display->setTextSize(1);
    display->setTextColor(TFT_YELLOW, COLOR_BACKGROUND);
    display->setCursor(20, 60);
    display->print("This wizard will calibrate");
    display->setCursor(20, 75);
    display->print("your touch screen.");
    
    display->setCursor(20, 100);
    display->print("You will tap 5 points:");
    display->setCursor(30, 115);
    display->print("* Corners");
    display->setCursor(30, 130);
    display->print("* Center");
    
    display->setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    display->setCursor(20, 155);
    display->print("Tap each crosshair 5 times");
    display->setCursor(20, 170);
    display->print("as accurately as possible.");
    
    // Start button
    display->fillRect(60, 200, 200, 30, TFT_DARKGREEN);
    display->drawRect(60, 200, 200, 30, TFT_GREEN);
    display->setTextColor(TFT_WHITE, TFT_DARKGREEN);
    display->setTextDatum(MC_DATUM);
    display->drawString("Start Calibration", 160, 215);
    display->setTextDatum(TL_DATUM);
}

void CalibrationWizardScreen::drawCalibratingStaticUI() {
    // CLEAN MINIMALIST DESIGN: Completely blank screen
    // No text, no instructions, no progress - just the crosshair
    // The crosshair will be drawn in renderDynamicContent()
}

void CalibrationWizardScreen::drawBackButton() {
    // Draw Back button in top-right corner (80x40, same as TouchTest)
    display->fillRect(BACK_BTN_X, BACK_BTN_Y, BACK_BTN_W, BACK_BTN_H, TFT_MAROON);
    display->drawRect(BACK_BTN_X, BACK_BTN_Y, BACK_BTN_W, BACK_BTN_H, TFT_RED);
    display->setTextColor(TFT_WHITE, TFT_MAROON);
    display->setTextSize(1);
    display->setTextDatum(MC_DATUM);
    display->drawString("< Back", BACK_BTN_X + BACK_BTN_W/2, BACK_BTN_Y + BACK_BTN_H/2);
    display->setTextDatum(TL_DATUM);
}

void CalibrationWizardScreen::drawCompleteStaticUI() {
    // Draw Back button in top-right
    drawBackButton();
    
    // Title
    display->setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    display->setTextSize(2);
    display->setCursor(30, 20);
    display->print("Calibration Complete!");
    
    // Success icon (checkmark)
    display->fillCircle(160, 100, 40, TFT_DARKGREEN);
    display->drawCircle(160, 100, 40, TFT_GREEN);
    
    // Draw checkmark
    display->drawLine(135, 100, 150, 115, TFT_WHITE);
    display->drawLine(136, 100, 151, 115, TFT_WHITE);
    display->drawLine(150, 115, 185, 80, TFT_WHITE);
    display->drawLine(151, 115, 186, 80, TFT_WHITE);
    
    // Done button
    display->fillRect(90, 200, 140, 30, TFT_NAVY);
    display->drawRect(90, 200, 140, 30, TFT_BLUE);
    display->setTextColor(TFT_WHITE, TFT_NAVY);
    display->setTextDatum(MC_DATUM);
    display->drawString("Done", 160, 215);
    display->setTextDatum(TL_DATUM);
}

// ============================================================================
// DYNAMIC CONTENT RENDERING - Only changing elements
// ============================================================================

void CalibrationWizardScreen::renderDynamicContent() {
    switch (currentState) {
        case CAL_INTRO:
            // No dynamic content in intro
            break;
            
        case CAL_CALIBRATING:
        case CAL_POINT_FLASH:
            // Draw progress text at top
            drawProgressText();
            // Draw crosshair (red or green based on state)
            drawCalibrationPoint();
            break;
            
        case CAL_COMPLETE:
            drawSuccessMessage();
            break;
    }
}

void CalibrationWizardScreen::drawCalibrationPoint() {
    // Get current calibration point coordinates
    int16_t x, y;
    getPointCoordinates(currentPoint, x, y);
    
    // A2-STYLE COLOR LOGIC: Green on POINT_FLASH, Red during normal calibration
    uint16_t crosshairColor = (currentState == CAL_POINT_FLASH) 
                              ? COLOR_CROSSHAIR_GREEN 
                              : COLOR_CROSSHAIR_RED;
    
    // A2 CIRCULAR TARGET DESIGN:
    // Outer circle (Target ring) - White
    display->drawCircle(x, y, 20, COLOR_TEXT);
    display->drawCircle(x, y, 19, COLOR_TEXT);
    
    // Inner circle (Solid center) - Colored (Red or Green)
    display->drawCircle(x, y, 15, crosshairColor);
    display->fillCircle(x, y, 8, crosshairColor);
    
    // Crosshair lines (Extending outwards from the circle)
    display->drawLine(x - 30, y, x - 22, y, crosshairColor);  // Left
    display->drawLine(x + 22, y, x + 30, y, crosshairColor);  // Right
    display->drawLine(x, y - 30, x, y - 22, crosshairColor);  // Top
    display->drawLine(x, y + 22, x, y + 30, crosshairColor);  // Bottom
}

void CalibrationWizardScreen::drawProgressText() {
    // PROGRESS UI: Show clicks and points remaining at top-center
    // This gives user feedback without cluttering the targeting area
    
    char line1[20];
    char line2[20];
    
    // Line 1: Current sample count for this point
    snprintf(line1, sizeof(line1), "Clicks: %d/5", samplesAtCurrentPoint);
    
    // Line 2: Points remaining (5 total points)
    snprintf(line2, sizeof(line2), "Points left: %d", POINT_COUNT - currentPoint);
    
    // Draw centered text at top of screen
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT_DIM, COLOR_BACKGROUND);
    display->setTextDatum(TC_DATUM);  // Top-Center alignment
    
    display->drawString(line1, SCREEN_WIDTH / 2, 15);
    display->drawString(line2, SCREEN_WIDTH / 2, 30);
    
    display->setTextDatum(TL_DATUM);  // Reset to Top-Left default
}

void CalibrationWizardScreen::drawSuccessMessage() {
    // Static message (drawn once via static UI)
    // Dynamic content: calibration results summary
    static bool resultDrawn = false;
    
    if (!resultDrawn) {
        display->setTextSize(1);
        display->setTextColor(TFT_CYAN, COLOR_BACKGROUND);
        display->setCursor(40, 155);
        display->print("Calibration data saved");
        
        display->setTextColor(TFT_WHITE, COLOR_BACKGROUND);
        display->setCursor(40, 170);
        display->print("Points collected: ");
        display->print(calData.samplesCollected);
        
        resultDrawn = true;
    }
}

// ============================================================================
// TOUCH EVENT HANDLING (NO DRAWING)
// ============================================================================

void CalibrationWizardScreen::onTouchEvent(const TouchPoint& point) {
    Serial.printf("[CalWizard] Touch: (%d,%d) in state %d\n", point.x, point.y, currentState);
    
    // 1. MODAL LOCK: During CALIBRATING or POINT_FLASH, ONLY collect samples
    //    This prevents accidental Back button presses and ensures focus
    if (currentState == CAL_CALIBRATING || currentState == CAL_POINT_FLASH) {
        Serial.println("[CalWizard] MODAL LOCK - Only sample collection allowed");
        collectSample(point.x, point.y);
        return;  // STOP: No other logic runs during calibration
    }
    
    // 2. UI LAYER: Always prioritize explicit buttons (Back, Start, Done)
    //    This ensures buttons are respected before state shortcuts
    Button button = getButtonAt(point.x, point.y);
    if (button != BTN_NONE) {
        handleButtonPress(button);
        return;  // Exit: Button handled, do nothing else
    }
    
    // 3. STATE LAYER: Only if no button was hit, check for state shortcuts
    if (currentState == CAL_INTRO) {
        // "Tap anywhere to start" shortcut
        Serial.println("[CalWizard] INTRO touch - Starting Calibration");
        setState(CAL_CALIBRATING);
        return;
    }
    
    if (currentState == CAL_COMPLETE) {
        // "Tap anywhere to exit" shortcut
        Serial.println("[CalWizard] COMPLETE state - Any touch exits");
        needsExit = true;
    }
}

// ============================================================================
// BUTTON DETECTION AND HANDLING
// ============================================================================

CalibrationWizardScreen::Button CalibrationWizardScreen::getButtonAt(int16_t x, int16_t y) {
    // Check Back button (visible in INTRO and COMPLETE states)
    if (currentState == CAL_INTRO || currentState == CAL_COMPLETE) {
        if (x >= BACK_BTN_X && x <= (BACK_BTN_X + BACK_BTN_W) && 
            y >= BACK_BTN_Y && y <= (BACK_BTN_Y + BACK_BTN_H)) {
            return BTN_BACK;
        }
    }
    
    // Check Start button (visible in INTRO state)
    if (currentState == CAL_INTRO) {
        // Start button at (60, 200, 200x30)
        if (x >= 60 && x <= 260 && y >= 200 && y <= 230) {
            return BTN_START;
        }
    }
    
    // Check Done button (visible in COMPLETE state)
    if (currentState == CAL_COMPLETE) {
        // Done button at (90, 200, 140x30)
        if (x >= 90 && x <= 230 && y >= 200 && y <= 230) {
            return BTN_DONE;
        }
    }
    
    return BTN_NONE;
}

void CalibrationWizardScreen::handleButtonPress(Button button) {
    switch (button) {
        case BTN_BACK:
            Serial.println("[CalWizard] Back button pressed");
            needsExit = true;
            break;
            
        case BTN_START:
            Serial.println("[CalWizard] Start button pressed - Beginning calibration");
            setState(CAL_CALIBRATING);
            break;
            
        case BTN_DONE:
            Serial.println("[CalWizard] Done button pressed - Exiting");
            needsExit = true;
            break;
            
        case BTN_NONE:
            // Should never reach here
            break;
    }
}

// ============================================================================
// DATA COLLECTION (NO DRAWING) - Pure state updates
// ============================================================================

void CalibrationWizardScreen::collectSample(int16_t rawX, int16_t rawY) {
    // Read raw touch data directly from sensor
    digitalWrite(TFT_CS, HIGH);  // Ensure TFT_CS high before touch read
    
    if (!ts.touched()) {
        return;  // Ignore if no touch detected
    }
    
    TS_Point p = ts.getPoint();
    
    // Store sample in buffer
    sampleBufferX[samplesAtCurrentPoint] = p.x;
    sampleBufferY[samplesAtCurrentPoint] = p.y;
    samplesAtCurrentPoint++;
    
    DEBUG_LOGF("Sample collected: Point %d, Sample %d/%d\n", 
               currentPoint, samplesAtCurrentPoint, SAMPLES_PER_POINT);
    
    // INCREMENTAL UPDATE: Only set screenIsDirty to update progress text
    // NO drawing here - draw() will handle it
    screenIsDirty = true;
    
    // Check if we've collected enough samples for this point
    if (samplesAtCurrentPoint >= SAMPLES_PER_POINT) {
        // Calculate average of samples
        int32_t avgX = 0, avgY = 0;
        for (uint8_t i = 0; i < SAMPLES_PER_POINT; i++) {
            avgX += sampleBufferX[i];
            avgY += sampleBufferY[i];
        }
        avgX /= SAMPLES_PER_POINT;
        avgY /= SAMPLES_PER_POINT;
        
        // Store calibration point
        int16_t screenX, screenY;
        getPointCoordinates(currentPoint, screenX, screenY);
        
        calData.rawX[currentPoint] = avgX;
        calData.rawY[currentPoint] = avgY;
        calData.screenX[currentPoint] = screenX;
        calData.screenY[currentPoint] = screenY;
        calData.samplesCollected++;
        
        DEBUG_LOGF("Point %d calibrated: raw(%d,%d) -> screen(%d,%d)\n",
                   currentPoint, avgX, avgY, screenX, screenY);
        
        // A2-STYLE: Trigger green flash instead of immediately advancing
        setState(CAL_POINT_FLASH);  // Timer in update() will advance after 300ms
    }
    
    // Small delay to prevent multiple samples from single touch
    delay(100);
}

void CalibrationWizardScreen::advanceToNextPoint() {
    // A2-STYLE: No longer used - flash logic moved to collectSample
    // Keeping this function for compatibility but it's now called from update()
    samplesAtCurrentPoint = 0;
    
    if (currentPoint >= POINT_COUNT) {
        // All points collected - calculate calibration
        calculateCalibration();
        setState(CAL_COMPLETE);  // Triggers forceFullRedraw
    } else {
        // Move to next point
        setState(CAL_CALIBRATING);  // Re-enter calibrating state for new point
    }
}

void CalibrationWizardScreen::calculateCalibration() {
    // Simple 2-point calibration using top-left and bottom-right
    // More sophisticated algorithms can be implemented here
    
    DEBUG_LOG("Calculating calibration parameters...");
    
    // For now, just store the raw data
    // Caller can use getCalibrationData() to retrieve and apply
    
    DEBUG_LOG("Calibration calculation complete");
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void CalibrationWizardScreen::getPointCoordinates(uint8_t pointIndex, int16_t& x, int16_t& y) {
    // CLEAN POSITIONING: Use exact MARGIN for 4 corner points
    switch (pointIndex) {
        case POINT_TOP_LEFT:
            x = MARGIN;
            y = MARGIN;
            break;
            
        case POINT_TOP_RIGHT:
            x = SCREEN_WIDTH - MARGIN;
            y = MARGIN;
            break;
            
        case POINT_BOTTOM_RIGHT:
            x = SCREEN_WIDTH - MARGIN;
            y = SCREEN_HEIGHT - MARGIN;
            break;
            
        case POINT_BOTTOM_LEFT:
            x = MARGIN;
            y = SCREEN_HEIGHT - MARGIN;
            break;
            
        case POINT_CENTER:
            x = SCREEN_WIDTH / 2;
            y = SCREEN_HEIGHT / 2;
            break;
            
        default:
            x = SCREEN_WIDTH / 2;
            y = SCREEN_HEIGHT / 2;
            break;
    }
}
