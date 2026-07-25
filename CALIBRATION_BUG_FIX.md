# Calibration Screen Not Updating - ROOT CAUSE FIXED

## The Problem
When tapping "Start Calibration", the screen state changed internally but nothing appeared on the display. The logs showed:
```
[CalWizard] ULTRA-SIMPLE Touch: (142,213) in state 0
[FIX] INTRO touch detected - Starting Calibration
[CalWizard] ULTRA-SIMPLE Touch: (142,215) in state 1
[CalWizard] Touch in CALIBRATING state - collecting sample
```

State WAS changing (0 → 1), but the screen stayed frozen on the intro.

## Root Cause
**Two-Level Flag System Mismatch:**

1. **CalibrationWizardScreen** has its own internal flags:
   - `forceFullRedraw` 
   - `screenIsDirty`

2. **ScreenManager** has separate flags:
   - `forceFullRedraw`
   - `screenIsDirty`

3. When CalibrationWizardScreen changed state internally via `setState()`, it set its OWN flags but ScreenManager didn't know about it.

4. ScreenManager's `render()` only called `drawCalibrationDynamicData()` when **ScreenManager's** `screenIsDirty` was true, which never happened for internal wizard state changes.

5. Result: `CalibrationWizardScreen.draw()` was never called after state changes!

## The Fix

### 1. Simplified Touch Handling (CalibrationWizardScreen.cpp)
```cpp
void CalibrationWizardScreen::onTouchEvent(const TouchPoint& point) {
    // DIRECT LOGIC: If we are in INTRO, any touch starts calibration
    if (currentState == CAL_INTRO) {
        Serial.println("[FIX] INTRO touch detected - Starting Calibration");
        setState(CAL_CALIBRATING);  // Sets forceFullRedraw and screenIsDirty
        return;
    }
    // ... rest of logic
}
```

### 2. Always Render Calibration Screen (ScreenManager.cpp)
```cpp
void ScreenManager::render() {
    // ... static UI rendering ...
    
    // SPECIAL CASE: Calibration wizard manages its own internal dirty flags
    // Always call its draw() method so it can handle internal state changes
    if (currentScreen == SCREEN_CALIBRATION) {
        drawCalibrationDynamicData();  // Always render for calibration
    } else if (screenIsDirty) {
        // ... other screens only render when ScreenManager is dirty ...
    }
}
```

### 3. Call Wizard Draw in Both Methods (ScreenManager.cpp)
```cpp
void ScreenManager::drawCalibrationStaticUI() {
    // Create wizard and call draw() to handle forceFullRedraw
    if (calibrationWizard != nullptr) {
        calibrationWizard->draw();
    }
}

void ScreenManager::drawCalibrationDynamicData() {
    // ALWAYS call wizard draw() - it manages its own internal dirty flags
    if (calibrationWizard != nullptr) {
        calibrationWizard->draw();
    }
}
```

## Why This Works

1. **Decoupled Flag Management:** CalibrationWizardScreen manages its own rendering lifecycle independently
2. **Continuous Rendering:** ScreenManager always calls the wizard's draw() method when on calibration screen
3. **Internal State Changes:** The wizard can change states and trigger redraws without ScreenManager knowing
4. **Standard Compliance:** Follows the forceFullRedraw/screenIsDirty pattern but at the wizard level

## Testing Expected Behavior

1. Navigate to Settings → Touch Calibration
2. Tap ANYWHERE on the intro screen → Should immediately show calibrating crosshair
3. Tap crosshair 5 times → Should advance to next point with visual feedback
4. Complete all 5 points → Should show success screen
5. Tap Back button at any time → Should return to Settings

## Files Modified
- `src/CalibrationWizardScreen.cpp` - Simplified touch handling
- `src/ScreenManager.cpp` - Always render calibration, call wizard draw() in both static and dynamic methods
