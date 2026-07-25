#ifndef CALIBRATIONWIZARDSCREEN_H
#define CALIBRATIONWIZARDSCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Calibration wizard states
enum CalibrationState {
    CAL_INTRO,
    CAL_CALIBRATING,
    CAL_POINT_FLASH,    // NEW: Green flash after successful point capture
    CAL_COMPLETE
};

// Calibration point indices
enum CalibrationPoint {
    POINT_TOP_LEFT = 0,
    POINT_TOP_RIGHT = 1,
    POINT_BOTTOM_RIGHT = 2,
    POINT_BOTTOM_LEFT = 3,
    POINT_CENTER = 4,
    POINT_COUNT = 5
};

// Calibration data structure
struct CalibrationData {
    int16_t rawX[POINT_COUNT];
    int16_t rawY[POINT_COUNT];
    int16_t screenX[POINT_COUNT];
    int16_t screenY[POINT_COUNT];
    uint8_t samplesCollected;
};

class CalibrationWizardScreen {
public:
    CalibrationWizardScreen(TFT_eSPI* tft, TouchManager* touchMgr);
    
    // Initialize wizard
    void begin();
    
    // Update logic (NO DRAWING)
    void update();
    
    // Draw function (ONLY DRAWING) - THE STANDARD
    void draw();
    
    // Handle touch events (NO DRAWING)
    void onTouchEvent(const TouchPoint& point);
    
    // Check if calibration is complete
    bool isComplete() const { return currentState == CAL_COMPLETE; }
    
    // Check if user wants to exit
    bool needsToExit() const { return needsExit; }
    
    // Get calibration results
    CalibrationData getCalibrationData() const { return calData; }
    
    // Reset wizard to beginning
    void reset();
    
private:
    TFT_eSPI* display;
    TouchManager* touch;
    
    // State management
    CalibrationState currentState;
    uint8_t currentPoint;
    uint8_t samplesAtCurrentPoint;
    bool needsExit;  // Set to true when user wants to exit
    unsigned long stateStartTime;  // Timer for state transitions (e.g., POINT_FLASH)
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Calibration data
    CalibrationData calData;
    
    // Sample collection buffer
    static const uint8_t SAMPLES_PER_POINT = 5;
    int16_t sampleBufferX[SAMPLES_PER_POINT];
    int16_t sampleBufferY[SAMPLES_PER_POINT];
    
    // State management
    void setState(CalibrationState newState);
    
    // Static UI drawing (called once per state)
    void drawIntroStaticUI();
    void drawCalibratingStaticUI();
    void drawCompleteStaticUI();
    void drawBackButton();  // Draw Back button in top-right
    
    // Dynamic data drawing (called when dirty)
    void renderDynamicContent();
    void drawCalibrationPoint();
    void drawProgressText();
    void drawSuccessMessage();
    
    // Data collection (NO DRAWING)
    void collectSample(int16_t rawX, int16_t rawY);
    void advanceToNextPoint();
    void calculateCalibration();
    
    // Helper functions
    void getPointCoordinates(uint8_t pointIndex, int16_t& x, int16_t& y);
    
    // Button detection
    enum Button {
        BTN_NONE,
        BTN_BACK,
        BTN_START,
        BTN_DONE
    };
    Button getButtonAt(int16_t x, int16_t y);
    void handleButtonPress(Button button);
};

#endif // CALIBRATIONWIZARDSCREEN_H
