# Calibration Wizard - Clean Minimalist Design (A2 Style)

## Design Philosophy
The calibrating state now follows your A2 project's clean, focused approach:
- **Blank screen** with only the crosshair visible
- **No text, no progress counters, no instructions** during calibration
- **Pure focus** on the crosshair for maximum accuracy
- **Visual feedback** through crosshair position changes only

## What Changed

### 1. CALIBRATING State - Completely Stripped UI
**Before:** Title, instructions, progress text, crosshair
**After:** Only the crosshair (red) on a black screen

```cpp
void CalibrationWizardScreen::drawCalibratingStaticUI() {
    // CLEAN MINIMALIST DESIGN: Completely blank screen
    // No text, no instructions, no progress - just the crosshair
}
```

### 2. Crosshair Color Logic (Ready for Flash State)
```cpp
void CalibrationWizardScreen::drawCalibrationPoint() {
    // Determine crosshair color based on state
    uint16_t crosshairColor = COLOR_CROSSHAIR_RED;  // Default: Red
    
    // Note: POINT_FLASH state would be handled here:
    // if (currentState == CAL_POINT_FLASH) {
    //     crosshairColor = COLOR_CROSSHAIR_GREEN;
    // }
    
    // Draw crosshair with selected color
}
```

### 3. Exact Corner Positioning
Points now positioned at exact MARGIN from edges (no offsets):

```cpp
Point 0 (Top-Left):     (30, 30)
Point 1 (Top-Right):    (290, 30)
Point 2 (Bottom-Right): (290, 210)
Point 3 (Bottom-Left):  (30, 210)
Point 4 (Center):       (160, 120)
```

**Before:** Had offsets like `MARGIN + 50` and `MARGIN - 30`
**After:** Clean `MARGIN` positioning for perfect corner alignment

### 4. Removed Progress Text
```cpp
void CalibrationWizardScreen::drawProgressText() {
    // REMOVED: Clean minimalist design has no progress text
    // User feedback is provided only by the crosshair changing position
}
```

### 5. Updated Color Definitions
```cpp
#define COLOR_CROSSHAIR_RED TFT_RED
#define COLOR_CROSSHAIR_GREEN TFT_GREEN
```
Separated red and green colors for future flash state implementation.

## User Experience Flow

### INTRO Screen (Unchanged - Perfect!)
```
┌─────────────────────────────────────┐
│  Touch Calibration            < Back│
│                                     │
│  This wizard will calibrate         │
│  your touch screen.                 │
│                                     │
│  You will tap 5 points:             │
│    * Corners                        │
│    * Center                         │
│                                     │
│  Tap each crosshair 5 times         │
│  as accurately as possible.         │
│                                     │
│  ┌──────────────────────────┐      │
│  │   Start Calibration      │      │
│  └──────────────────────────┘      │
└─────────────────────────────────────┘
```

### CALIBRATING Screen (New - Minimalist!)
```
┌─────────────────────────────────────┐
│                                     │
│                                     │
│    ┼  ← Red crosshair here          │
│                                     │
│                                     │
│                                     │
│         (completely blank)          │
│                                     │
│                                     │
│                                     │
│                                     │
└─────────────────────────────────────┘
```

Just a red crosshair on pure black. Nothing else.

### COMPLETE Screen (Unchanged)
```
┌─────────────────────────────────────┐
│  Calibration Complete!        < Back│
│                                     │
│          ┌───────┐                  │
│          │   ✓   │  ← Green circle  │
│          └───────┘                  │
│                                     │
│  Calibration data saved             │
│  Points collected: 5                │
│                                     │
│       ┌──────────┐                  │
│       │   Done   │                  │
│       └──────────┘                  │
└─────────────────────────────────────┘
```

## Future Enhancement: Flash State
The code is ready for a "flash" state where the crosshair turns green briefly when a point is accepted:

```cpp
// In onTouchEvent after successful sample:
setState(CAL_POINT_FLASH);
delay(200);  // Brief green flash
advanceToNextPoint();  // Move to next point
```

This would provide instant visual confirmation without cluttering the UI.

## Technical Benefits

1. **Zero visual distractions** - User focuses 100% on precision tapping
2. **Maximum screen real estate** - Crosshair can be positioned at exact edges
3. **Clean state transitions** - Black screen clears instantly, crosshair appears
4. **Ready for animations** - Flash state infrastructure in place
5. **Matches A2 design** - Proven successful UX pattern

## Files Modified
- `src/CalibrationWizardScreen.cpp`
  - Stripped `drawCalibratingStaticUI()` to blank
  - Removed all progress text rendering
  - Updated `getPointCoordinates()` for exact MARGIN positioning
  - Added red/green color logic to `drawCalibrationPoint()`
  - Simplified `renderDynamicContent()` for calibrating state
