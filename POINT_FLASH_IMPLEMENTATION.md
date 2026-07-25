# A2-Style POINT_FLASH Implementation - Complete

## Feature: Green Flash Success Feedback

When a calibration point is successfully captured (5 samples collected), the crosshair briefly flashes **GREEN** for 300ms before moving to the next point. This provides instant visual confirmation without cluttering the UI.

## State Flow

```
CAL_INTRO
    ↓ (user taps Start)
CAL_CALIBRATING (Red crosshair)
    ↓ (5 samples collected)
CAL_POINT_FLASH (Green crosshair, 300ms)
    ↓ (timer expires)
CAL_CALIBRATING (Red crosshair, next point)
    ↓ (repeat until all 5 points done)
CAL_COMPLETE (Success screen)
```

## Implementation Details

### 1. New State Added
```cpp
enum CalibrationState {
    CAL_INTRO,
    CAL_CALIBRATING,
    CAL_POINT_FLASH,    // NEW: Green flash after successful point
    CAL_COMPLETE
};
```

### 2. Timer Variable Added
```cpp
unsigned long stateStartTime;  // Tracks when state was entered
```

### 3. setState() Records Entry Time
```cpp
void CalibrationWizardScreen::setState(CalibrationState newState) {
    if (currentState != newState) {
        currentState = newState;
        stateStartTime = millis();  // Record time for timer logic
        forceFullRedraw = true;
        screenIsDirty = true;
    }
}
```

### 4. update() Handles Timer Transition
```cpp
void CalibrationWizardScreen::update() {
    // A2-STYLE TIMER: Handle POINT_FLASH state transition
    if (currentState == CAL_POINT_FLASH) {
        // Flash green for 300ms, then move to next point
        if (millis() - stateStartTime >= 300) {
            currentPoint++;
            samplesAtCurrentPoint = 0;
            
            if (currentPoint >= POINT_COUNT) {
                calculateCalibration();
                setState(CAL_COMPLETE);
            } else {
                setState(CAL_CALIBRATING);
            }
        }
    }
}
```

### 5. Color Logic in drawCalibrationPoint()
```cpp
void CalibrationWizardScreen::drawCalibrationPoint() {
    int16_t x, y;
    getPointCoordinates(currentPoint, x, y);
    
    // A2-STYLE: Green on POINT_FLASH, Red during calibration
    uint16_t crosshairColor = (currentState == CAL_POINT_FLASH) 
                              ? COLOR_CROSSHAIR_GREEN 
                              : COLOR_CROSSHAIR_RED;
    
    // Draw A2 circular target with selected color
    // ... (circular target code)
}
```

### 6. collectSample() Triggers Flash
```cpp
void CalibrationWizardScreen::collectSample(int16_t rawX, int16_t rawY) {
    // ... collect and store sample ...
    
    if (samplesAtCurrentPoint >= SAMPLES_PER_POINT) {
        // ... calculate and store average ...
        
        // A2-STYLE: Trigger green flash
        setState(CAL_POINT_FLASH);  
        // Timer in update() will advance after 300ms
    }
}
```

### 7. draw() Handles Both States
```cpp
void CalibrationWizardScreen::draw() {
    if (forceFullRedraw) {
        display->fillScreen(COLOR_BACKGROUND);
        
        switch (currentState) {
            case CAL_CALIBRATING:
            case CAL_POINT_FLASH:
                drawCalibratingStaticUI();  // Both use blank screen
                break;
            // ... other states ...
        }
    }
    
    if (screenIsDirty) {
        renderDynamicContent();  // Draws crosshair with correct color
    }
}
```

## User Experience Timeline

```
Time    State              What User Sees
────────────────────────────────────────────────────────────
0ms     CAL_CALIBRATING    Red crosshair at Point 0 (30,30)
        [user taps 5 times]
        
2000ms  CAL_POINT_FLASH    Green crosshair at Point 0
                            ✓ Visual confirmation!
        
2300ms  CAL_CALIBRATING    Red crosshair at Point 1 (290,30)
        [user taps 5 times]
        
4300ms  CAL_POINT_FLASH    Green crosshair at Point 1
                            ✓ Point accepted!
        
4600ms  CAL_CALIBRATING    Red crosshair at Point 2 (290,210)
        ... (continues for all 5 points)
```

## Benefits

1. **Instant Feedback** - User knows immediately when point is captured
2. **No Text Clutter** - Feedback is visual, not textual
3. **Smooth Flow** - Automatic transition to next point
4. **Professional Feel** - Polished UX matching A2 design
5. **Clear States** - Red = "tap here", Green = "got it!"

## Technical Advantages

1. **Event-Driven** - Timer runs in update(), no blocking delays
2. **State-Based Rendering** - Color determined by state, not flags
3. **Clean Separation** - Logic in update(), drawing in draw()
4. **Predictable Timing** - Exactly 300ms flash duration
5. **Extensible** - Easy to add sound/haptic feedback later

## Debug Output Example

```
Sample collected: Point 0, Sample 5/5
Point 0 calibrated: raw(340,450) -> screen(30,30)
CalibrationWizard: setState -> 2 (forceFullRedraw=true)  ← POINT_FLASH
Flash complete, advancing from point 0
CalibrationWizard: setState -> 1 (forceFullRedraw=true)  ← CALIBRATING
```

## Files Modified

- `include/CalibrationWizardScreen.h`
  - Added `CAL_POINT_FLASH` state
  - Added `stateStartTime` variable

- `src/CalibrationWizardScreen.cpp`
  - Updated `setState()` to record entry time
  - Implemented timer logic in `update()`
  - Updated `drawCalibrationPoint()` with color logic
  - Modified `collectSample()` to trigger flash
  - Updated `draw()` switch for POINT_FLASH
  - Modified `advanceToNextPoint()` logic
