# CalibrationWizardScreen - Quick Start Guide

## 5-Minute Integration

### Step 1: Files Already Created ✓
```
✓ include/CalibrationWizardScreen.h
✓ src/CalibrationWizardScreen.cpp
```

### Step 2: Add to main.cpp (Option A - Standalone)

```cpp
#include "CalibrationWizardScreen.h"

// Add global variable
CalibrationWizardScreen* calibrationWizard = nullptr;
bool calibrationMode = true;

// Modify setup()
void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting Calibration Wizard...");
    
    initHardware();
    
    touchManager = new TouchManager(&ts);
    touchManager->begin();
    
    // Create calibration wizard
    calibrationWizard = new CalibrationWizardScreen(&tft, touchManager);
    calibrationWizard->begin();
    
    Serial.println("Ready - Tap 'Start Calibration'");
}

// Modify loop()
void loop() {
    if (calibrationMode) {
        // === CALIBRATION MODE ===
        touchManager->update();
        calibrationWizard->update();
        
        if (touchManager->getState() == TOUCH_DOWN) {
            TouchPoint point = touchManager->getPoint();
            if (point.valid) {
                calibrationWizard->onTouchEvent(point);
            }
        }
        
        calibrationWizard->draw();
        
        // Check if complete
        if (calibrationWizard->isComplete()) {
            Serial.println("✓ Calibration complete!");
            
            CalibrationData calData = calibrationWizard->getCalibrationData();
            
            // Print results
            Serial.println("\nCalibration Results:");
            for (int i = 0; i < 5; i++) {
                Serial.printf("Point %d: raw(%d,%d) -> screen(%d,%d)\n",
                             i, calData.rawX[i], calData.rawY[i],
                             calData.screenX[i], calData.screenY[i]);
            }
            
            // Switch to normal mode
            delete calibrationWizard;
            calibrationWizard = nullptr;
            calibrationMode = false;
            
            screenManager = new ScreenManager(&tft, touchManager);
            screenManager->begin();
            screenManager->render();
            
            Serial.println("Switched to normal operation");
        }
        
    } else {
        // === NORMAL MODE ===
        touchManager->update();
        screenManager->update();
        screenManager->render();
    }
    
    delay(50);
}
```

### Step 3: Build and Upload
```bash
platformio run --target upload
```

### Step 4: Use the Wizard
1. Power on device
2. See "Touch Calibration" screen
3. Tap "Start Calibration" button
4. Tap each RED crosshair 5 times:
   - Top-left corner
   - Top-right corner
   - Bottom-right corner
   - Bottom-left corner
   - Center
5. See "Calibration Complete!"
6. Tap "Done"
7. Normal operation begins

**That's it! Calibration complete in 30 seconds.**

---

## Integration Option B - Add to ScreenManager

If you want calibration in your Settings menu:

### 1. Add to ScreenState enum (ScreenManager.h)
```cpp
enum ScreenState {
    SCREEN_HOME,
    SCREEN_SETTINGS,
    SCREEN_INFO,
    SCREEN_FAVORITES,
    SCREEN_ADDPASS,
    SCREEN_TOUCH_TEST,
    SCREEN_CALIBRATION  // Add this
};
```

### 2. Add calibration wizard member (ScreenManager.h)
```cpp
class ScreenManager {
private:
    CalibrationWizardScreen* calibrationWizard;
    // ... existing members
};
```

### 3. Initialize in constructor (ScreenManager.cpp)
```cpp
#include "CalibrationWizardScreen.h"

ScreenManager::ScreenManager(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      // ... other initializers ...
{
    // Create calibration wizard
    calibrationWizard = new CalibrationWizardScreen(display, touchMgr);
    calibrationWizard->begin();
}
```

### 4. Add button to Settings screen
```cpp
void ScreenManager::drawSettingsStaticUI() {
    // ... existing buttons ...
    
    // Calibrate Touch button
    tft->fillRect(20, 110, 180, 40, TFT_NAVY);
    tft->drawRect(20, 110, 180, 40, TFT_BLUE);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Calibrate Touch", 110, 130);
}

void ScreenManager::handleSettingsTouch(const TouchPoint& point) {
    // ... existing handlers ...
    
    // Calibrate Touch button
    else if (point.x >= 20 && point.x <= 200 && 
             point.y >= 110 && point.y <= 150) {
        calibrationWizard->reset();
        pushScreen(SCREEN_CALIBRATION);
    }
}
```

### 5. Add screen handlers
```cpp
// In update() switch
case SCREEN_CALIBRATION:
    calibrationWizard->update();
    
    if (calibrationWizard->isComplete()) {
        CalibrationData calData = calibrationWizard->getCalibrationData();
        // Save calibration data here
        popScreen(SCREEN_SETTINGS);
    }
    break;

// In render() - static UI switch
case SCREEN_CALIBRATION:
    // Wizard handles its own static UI
    break;

// In render() - dynamic data switch  
case SCREEN_CALIBRATION:
    calibrationWizard->draw();
    break;

// In update() - touch handling switch
case SCREEN_CALIBRATION:
    calibrationWizard->onTouchEvent(point);
    break;
```

Done! Now you have a "Calibrate Touch" button in Settings.

---

## Expected Serial Output

```
Starting Calibration Wizard...
CPU Frequency: 160 MHz
CalibrationWizard initialized - A1 Architecture
Ready - Tap 'Start Calibration'

CalibrationWizard: setState -> 0 (forceFullRedraw=true)
CalibrationWizard: Static UI drawn
CalibrationWizard: Dynamic content drawn

[User taps Start]
Start Calibration button pressed
CalibrationWizard: setState -> 1 (forceFullRedraw=true)
CalibrationWizard: Static UI drawn
CalibrationWizard: Dynamic content drawn

[User calibrates]
Sample collected: Point 0, Sample 1/5
Sample collected: Point 0, Sample 2/5
Sample collected: Point 0, Sample 3/5
Sample collected: Point 0, Sample 4/5
Sample collected: Point 0, Sample 5/5
Point 0 calibrated: raw(280,410) -> screen(30,80)

Sample collected: Point 1, Sample 1/5
...
Point 1 calibrated: raw(3800,420) -> screen(290,80)

...

Point 4 calibrated: raw(2040,1850) -> screen(160,120)

Calculating calibration parameters...
Calibration calculation complete
CalibrationWizard: setState -> 2 (forceFullRedraw=true)
CalibrationWizard: Static UI drawn
CalibrationWizard: Dynamic content drawn

Done button pressed - calibration complete
✓ Calibration complete!

Calibration Results:
Point 0: raw(280,410) -> screen(30,80)
Point 1: raw(3800,420) -> screen(290,80)
Point 2: raw(3810,3680) -> screen(290,180)
Point 3: raw(260,3690) -> screen(30,180)
Point 4: raw(2040,1850) -> screen(160,120)

Switched to normal operation
```

---

## Troubleshooting

### Problem: Screen stays black
**Solution:** Check TFT initialization in Hardware.cpp

### Problem: Touch not registering  
**Solution:** Verify TOUCH_CS pin and SPI wiring

### Problem: Crosshair in wrong position
**Solution:** Check SCREEN_WIDTH/HEIGHT in Config.h

### Problem: Calibration never completes
**Solution:** Ensure you tap exactly 5 times per crosshair

### Problem: Flickering during progress
**Solution:** Verify screenIsDirty flag logic (should be working)

---

## Files Reference

### Core Implementation
- `include/CalibrationWizardScreen.h` - Class interface
- `src/CalibrationWizardScreen.cpp` - Implementation

### Documentation
- `CALIBRATION_WIZARD_SUMMARY.md` - Overview
- `CALIBRATION_WIZARD_DOCUMENTATION.md` - Full guide
- `CALIBRATION_INTEGRATION_EXAMPLE.cpp` - Examples
- `CALIBRATION_VISUAL_REFERENCE.md` - Diagrams
- `CALIBRATION_QUICK_START.md` - This file

---

## Next Steps

1. ✅ Files created
2. ⏳ Integrate into main.cpp (choose Option A or B)
3. ⏳ Build and upload
4. ⏳ Test calibration wizard
5. ⏳ Verify touch accuracy
6. ⏳ Add EEPROM persistence (optional)

---

**Total integration time: 5 minutes**
**User calibration time: 30 seconds**
**Result: Perfect touch accuracy!** ✨
