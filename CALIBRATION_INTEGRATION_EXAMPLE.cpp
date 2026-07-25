/**
 * CALIBRATION_INTEGRATION_EXAMPLE.cpp
 * 
 * This file demonstrates how to integrate CalibrationWizardScreen
 * into your existing project with ScreenManager.
 * 
 * Three integration approaches:
 * 1. Standalone mode (run once on first boot)
 * 2. Settings menu option (user-triggered)
 * 3. Combined with ScreenManager as a screen state
 */

#include <Arduino.h>
#include "Hardware.h"
#include "TouchManager.h"
#include "ScreenManager.h"
#include "CalibrationWizardScreen.h"

// ============================================================================
// APPROACH 1: STANDALONE MODE (Run on First Boot)
// ============================================================================

/**
 * Use this approach to run calibration wizard automatically on first boot
 * before entering normal screen manager operation.
 */

// Global managers
TouchManager* touchManager = nullptr;
ScreenManager* screenManager = nullptr;
CalibrationWizardScreen* calibrationWizard = nullptr;

bool needsCalibration = false;
bool calibrationComplete = false;

void setup_approach1() {
    Serial.begin(115200);
    Serial.println("\nStarting with Calibration Wizard...");
    
    // Initialize hardware
    initHardware();
    
    // Create touch manager
    touchManager = new TouchManager(&ts);
    touchManager->begin();
    
    // Check if calibration needed (e.g., from EEPROM flag)
    needsCalibration = !isCalibrationDataValid();
    
    if (needsCalibration) {
        Serial.println("No valid calibration - starting wizard");
        
        // Create calibration wizard
        calibrationWizard = new CalibrationWizardScreen(&tft, touchManager);
        calibrationWizard->begin();
    } else {
        Serial.println("Calibration data loaded - starting normal mode");
        
        // Create screen manager
        screenManager = new ScreenManager(&tft, touchManager);
        screenManager->begin();
        screenManager->render();
    }
    
    Serial.println("System ready");
}

void loop_approach1() {
    if (needsCalibration && !calibrationComplete) {
        // === CALIBRATION MODE ===
        
        // Update touch
        touchManager->update();
        
        // Update calibration wizard (no drawing)
        calibrationWizard->update();
        
        // Handle touch events
        if (touchManager->getState() == TOUCH_DOWN) {
            TouchPoint point = touchManager->getPoint();
            if (point.valid) {
                calibrationWizard->onTouchEvent(point);
            }
        }
        
        // Render calibration wizard (only drawing)
        calibrationWizard->draw();
        
        // Check if calibration complete
        if (calibrationWizard->isComplete()) {
            Serial.println("Calibration wizard complete!");
            
            // Get calibration data
            CalibrationData calData = calibrationWizard->getCalibrationData();
            
            // Save to EEPROM/SPIFFS
            saveCalibrationData(calData);
            
            // Apply calibration
            applyCalibration(calData);
            
            // Clean up wizard
            delete calibrationWizard;
            calibrationWizard = nullptr;
            calibrationComplete = true;
            
            // Start normal screen manager
            screenManager = new ScreenManager(&tft, touchManager);
            screenManager->begin();
            
            Serial.println("Switching to normal mode");
        }
        
    } else {
        // === NORMAL MODE ===
        
        touchManager->update();
        screenManager->update();
        screenManager->render();
    }
    
    delay(50);
}

// ============================================================================
// APPROACH 2: SETTINGS MENU OPTION (User-Triggered)
// ============================================================================

/**
 * Add calibration wizard as an option in Settings screen.
 * User can re-calibrate at any time.
 */

// Add to ScreenManager.h enum:
enum ScreenState {
    SCREEN_HOME,
    SCREEN_SETTINGS,
    SCREEN_INFO,
    SCREEN_FAVORITES,
    SCREEN_ADDPASS,
    SCREEN_TOUCH_TEST,
    SCREEN_CALIBRATION  // Add this
};

// In ScreenManager class, add:
class ScreenManager {
private:
    CalibrationWizardScreen* calibrationWizard;
    
    void drawCalibratingStaticUI();
    void drawCalibratingDynamicData();
    void updateCalibrationData();
    void handleCalibrationTouch(const TouchPoint& point);
};

// In ScreenManager.cpp constructor:
ScreenManager::ScreenManager(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      currentScreen(SCREEN_HOME),
      forceFullRedraw(true),
      screenIsDirty(true),
      calibrationWizard(nullptr) {
    
    // Create calibration wizard
    calibrationWizard = new CalibrationWizardScreen(display, touchMgr);
    calibrationWizard->begin();
}

// Add "Calibrate Touch" button to Settings screen:
void ScreenManager::drawSettingsStaticUI() {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print("SETTINGS");
    
    // Touch Test button
    tft->fillRect(20, 60, 180, 40, TFT_DARKGREEN);
    tft->drawRect(20, 60, 180, 40, TFT_GREEN);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Touch Test", 110, 80);
    
    // Calibrate Touch button (NEW)
    tft->fillRect(20, 110, 180, 40, TFT_NAVY);
    tft->drawRect(20, 110, 180, 40, TFT_BLUE);
    tft->drawString("Calibrate Touch", 110, 130);
    
    // Back button
    tft->fillRect(20, 180, 100, 40, TFT_MAROON);
    tft->drawRect(20, 180, 100, 40, TFT_RED);
    tft->drawString("Back", 70, 200);
    
    tft->setTextDatum(TL_DATUM);
}

void ScreenManager::handleSettingsTouch(const TouchPoint& point) {
    // Touch Test button
    if (point.x >= 20 && point.x <= 200 && point.y >= 60 && point.y <= 100) {
        pushScreen(SCREEN_TOUCH_TEST);
    }
    // Calibrate Touch button (NEW)
    else if (point.x >= 20 && point.x <= 200 && point.y >= 110 && point.y <= 150) {
        calibrationWizard->reset();  // Reset wizard state
        pushScreen(SCREEN_CALIBRATION);
    }
    // Back button
    else if (point.x >= 20 && point.x <= 120 && point.y >= 180 && point.y <= 220) {
        popScreen(SCREEN_HOME);
    }
}

// Add calibration screen handlers:
void ScreenManager::drawCalibratingStaticUI() {
    // Delegate to calibration wizard
    calibrationWizard->draw();
}

void ScreenManager::drawCalibratingDynamicData() {
    // Already handled by calibration wizard in draw()
}

void ScreenManager::updateCalibrationData() {
    calibrationWizard->update();
    
    // Check if complete
    if (calibrationWizard->isComplete()) {
        CalibrationData calData = calibrationWizard->getCalibrationData();
        saveCalibrationData(calData);
        applyCalibration(calData);
        
        // Return to settings
        popScreen(SCREEN_SETTINGS);
    }
}

void ScreenManager::handleCalibrationTouch(const TouchPoint& point) {
    calibrationWizard->onTouchEvent(point);
}

// Update render() switch:
void ScreenManager::render() {
    if (forceFullRedraw) {
        if (currentScreen != SCREEN_CALIBRATION) {
            tft->fillScreen(TFT_BLACK);
        }
        
        switch (currentScreen) {
            case SCREEN_HOME:
                drawHomeStaticUI();
                break;
            // ... other screens ...
            case SCREEN_CALIBRATION:
                // Wizard handles its own clearing
                break;
        }
        
        forceFullRedraw = false;
        screenIsDirty = true;
    }
    
    if (screenIsDirty) {
        switch (currentScreen) {
            case SCREEN_HOME:
                drawHomeDynamicData();
                break;
            // ... other screens ...
            case SCREEN_CALIBRATION:
                calibrationWizard->draw();
                break;
        }
        
        screenIsDirty = false;
    }
}

// Update update() switch:
void ScreenManager::update() {
    switch (currentScreen) {
        case SCREEN_HOME:
            updateHomeData();
            break;
        // ... other screens ...
        case SCREEN_CALIBRATION:
            updateCalibrationData();
            break;
    }
    
    // Handle touch
    if (touch->getState() == TOUCH_DOWN) {
        TouchPoint point = touch->getPoint();
        
        if (point.valid) {
            switch (currentScreen) {
                case SCREEN_HOME:
                    handleHomeTouch(point);
                    break;
                // ... other screens ...
                case SCREEN_CALIBRATION:
                    handleCalibrationTouch(point);
                    break;
            }
        }
    }
}

// ============================================================================
// APPROACH 3: STANDALONE CLASS WITH CALLBACK
// ============================================================================

/**
 * Keep CalibrationWizardScreen completely separate from ScreenManager.
 * Use a callback to signal completion.
 */

class CalibrationManager {
public:
    CalibrationManager(TFT_eSPI* tft, TouchManager* touch) 
        : wizard(tft, touch), active(false) {}
    
    void begin() {
        wizard.begin();
    }
    
    void start() {
        wizard.reset();
        active = true;
    }
    
    void update() {
        if (!active) return;
        
        wizard.update();
        
        if (wizard.isComplete()) {
            CalibrationData calData = wizard.getCalibrationData();
            
            // Save calibration
            saveCalibrationData(calData);
            applyCalibration(calData);
            
            // Deactivate
            active = false;
            
            // Callback (if needed)
            if (onComplete) {
                onComplete(calData);
            }
        }
    }
    
    void handleTouch(const TouchPoint& point) {
        if (!active) return;
        wizard.onTouchEvent(point);
    }
    
    void draw() {
        if (!active) return;
        wizard.draw();
    }
    
    bool isActive() const { return active; }
    
    // Callback for completion
    void setOnComplete(void (*callback)(CalibrationData)) {
        onComplete = callback;
    }
    
private:
    CalibrationWizardScreen wizard;
    bool active;
    void (*onComplete)(CalibrationData) = nullptr;
};

// Usage in main.cpp:
CalibrationManager* calibrationMgr = nullptr;

void onCalibrationComplete(CalibrationData calData) {
    Serial.println("Calibration complete - callback triggered");
    Serial.printf("Points collected: %d\n", calData.samplesCollected);
    
    // Force screen manager to redraw
    screenManager->pushScreen(SCREEN_HOME);
}

void setup_approach3() {
    Serial.begin(115200);
    initHardware();
    
    touchManager = new TouchManager(&ts);
    touchManager->begin();
    
    screenManager = new ScreenManager(&tft, touchManager);
    screenManager->begin();
    
    calibrationMgr = new CalibrationManager(&tft, touchManager);
    calibrationMgr->begin();
    calibrationMgr->setOnComplete(onCalibrationComplete);
    
    // Start calibration if needed
    if (!isCalibrationDataValid()) {
        calibrationMgr->start();
    }
}

void loop_approach3() {
    touchManager->update();
    
    if (calibrationMgr->isActive()) {
        // Calibration mode - override screen manager
        calibrationMgr->update();
        
        if (touchManager->getState() == TOUCH_DOWN) {
            TouchPoint point = touchManager->getPoint();
            if (point.valid) {
                calibrationMgr->handleTouch(point);
            }
        }
        
        calibrationMgr->draw();
        
    } else {
        // Normal screen manager mode
        screenManager->update();
        
        if (touchManager->getState() == TOUCH_DOWN) {
            TouchPoint point = touchManager->getPoint();
            if (point.valid) {
                // Check for special trigger (e.g., long press)
                if (point.y < 30 && point.x > 280) {
                    // Top-right corner - trigger calibration
                    calibrationMgr->start();
                }
            }
        }
        
        screenManager->render();
    }
    
    delay(50);
}

// ============================================================================
// HELPER FUNCTIONS (Common to all approaches)
// ============================================================================

bool isCalibrationDataValid() {
    // Check EEPROM for valid calibration data
    EEPROM.begin(512);
    
    uint16_t magic;
    EEPROM.get(0, magic);
    
    EEPROM.end();
    
    return (magic == 0xCAFE);
}

void saveCalibrationData(const CalibrationData& calData) {
    struct StoredCal {
        uint16_t magic;
        CalibrationData data;
    };
    
    StoredCal stored;
    stored.magic = 0xCAFE;
    stored.data = calData;
    
    EEPROM.begin(512);
    EEPROM.put(0, stored);
    EEPROM.commit();
    EEPROM.end();
    
    Serial.println("✓ Calibration data saved to EEPROM");
}

void applyCalibration(const CalibrationData& calData) {
    // Calculate min/max from collected data
    int16_t minX = 4095, maxX = 0;
    int16_t minY = 4095, maxY = 0;
    
    for (int i = 0; i < POINT_COUNT; i++) {
        if (calData.rawX[i] < minX) minX = calData.rawX[i];
        if (calData.rawX[i] > maxX) maxX = calData.rawX[i];
        if (calData.rawY[i] < minY) minY = calData.rawY[i];
        if (calData.rawY[i] > maxY) maxY = calData.rawY[i];
    }
    
    Serial.printf("✓ Calibration applied:\n");
    Serial.printf("  X: %d -> %d\n", minX, maxX);
    Serial.printf("  Y: %d -> %d\n", minY, maxY);
    
    // Update Config.h values (requires rebuild) or store in EEPROM
    // For runtime application, modify TouchManager to use these values
}

// ============================================================================
// RECOMMENDATION
// ============================================================================

/**
 * RECOMMENDED APPROACH: #1 (Standalone Mode)
 * 
 * Pros:
 * - Simple integration
 * - Clean separation from ScreenManager
 * - Runs before normal operation
 * - Easy to test
 * 
 * Implementation:
 * 1. Check for valid calibration in setup()
 * 2. If invalid, run CalibrationWizardScreen
 * 3. Save results and switch to ScreenManager
 * 4. Add re-calibration option in Settings (Approach #2)
 * 
 * This gives you:
 * - Automatic calibration on first boot
 * - Manual re-calibration option
 * - Clean code structure
 * - Minimal coupling
 */
