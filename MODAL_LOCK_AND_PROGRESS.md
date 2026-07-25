# Modal Lock & Progress UI Implementation

## Feature 1: Modal Lock During Calibration

### Problem Solved
Previously, users could accidentally press the Back button (top-right corner) during calibration, causing frustration and data loss. The touch input during calibration needs to be dedicated exclusively to sample collection.

### Solution: Modal Lock
When in `CAL_CALIBRATING` or `CAL_POINT_FLASH` state, the screen becomes **modal** - only sample collection is allowed. All other touch handling (buttons, navigation) is disabled.

### Implementation

```cpp
void CalibrationWizardScreen::onTouchEvent(const TouchPoint& point) {
    // 1. MODAL LOCK: During CALIBRATING or POINT_FLASH, ONLY collect samples
    if (currentState == CAL_CALIBRATING || currentState == CAL_POINT_FLASH) {
        Serial.println("[CalWizard] MODAL LOCK - Only sample collection allowed");
        collectSample(point.x, point.y);
        return;  // STOP: No other logic runs during calibration
    }
    
    // 2. INTRO STATE: Any touch starts calibration
    if (currentState == CAL_INTRO) {
        setState(CAL_CALIBRATING);
        return;
    }
    
    // 3. COMPLETE STATE: Back button and any touch exits
    if (currentState == CAL_COMPLETE) {
        // Check Back button or any touch
        needsExit = true;
    }
}
```

### Benefits

1. **Prevents Accidental Exit** - Back button is disabled during calibration
2. **Focus on Task** - User can tap anywhere without worry
3. **Data Protection** - Can't lose progress by accident
4. **Cleaner Logic** - State-based modal behavior is clear
5. **Better UX** - User knows they're in a focused mode

### State-Based Navigation Table

| State            | Any Touch        | Back Button      | Notes                    |
|------------------|------------------|------------------|--------------------------|
| CAL_INTRO        | Start Calibration| (Not shown)      | Welcome screen           |
| CAL_CALIBRATING  | Collect Sample   | **DISABLED**     | **MODAL LOCK**           |
| CAL_POINT_FLASH  | Collect Sample   | **DISABLED**     | **MODAL LOCK**           |
| CAL_COMPLETE     | Exit             | Exit             | Normal navigation resume |

---

## Feature 2: Progress UI at Top Center

### Design
Two lines of text centered at the top of the screen during calibration:
- **Line 1**: `Clicks: 3/5` - Shows current sample count for active point
- **Line 2**: `Points left: 4` - Shows remaining calibration points

### Visual Layout

```
┌──────────────────────────────────────┐
│          Clicks: 3/5                 │  ← Line 1 (Y=15)
│        Points left: 4                │  ← Line 2 (Y=30)
│                                      │
│                                      │
│                                      │
│              ◎                       │  ← Crosshair
│                                      │
│                                      │
│                                      │
│                                      │
└──────────────────────────────────────┘
```

### Implementation

```cpp
void CalibrationWizardScreen::drawProgressText() {
    char line1[20];
    char line2[20];
    
    // Format progress strings
    snprintf(line1, sizeof(line1), "Clicks: %d/5", samplesAtCurrentPoint);
    snprintf(line2, sizeof(line2), "Points left: %d", POINT_COUNT - currentPoint);
    
    // Draw centered at top
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT_DIM, COLOR_BACKGROUND);
    display->setTextDatum(TC_DATUM);  // Top-Center alignment
    
    display->drawString(line1, SCREEN_WIDTH / 2, 15);
    display->drawString(line2, SCREEN_WIDTH / 2, 30);
    
    display->setTextDatum(TL_DATUM);  // Reset to default
}
```

### Color Choice

- **COLOR_TEXT_DIM** (TFT_DARKGREY) is used instead of white
- Provides information without drawing attention from the crosshair
- Still readable but not distracting

### Why Top Center?

1. **Safe Zone** - Far from crosshair targets in corners
2. **Natural Eye Movement** - Users look up to check progress
3. **No Interference** - Doesn't overlap with touch areas
4. **Conventional** - Standard location for status information
5. **Always Visible** - Top area rarely obscured by hand/stylus

### Dynamic Updates

The progress text updates automatically:
- `samplesAtCurrentPoint` increments from 0 to 5
- `currentPoint` advances from 0 to 4
- Text re-renders on each touch via `screenIsDirty = true`

---

## Combined User Experience

### Calibration Flow with Modal Lock & Progress

```
┌─────────────────────────────────────┐
│  Touch Calibration          < Back  │
│                                     │
│  Instructions...                    │
│  ┌──────────────────────────┐      │
│  │   Start Calibration      │      │
│  └──────────────────────────┘      │
└─────────────────────────────────────┘
         ↓ User taps Start

┌─────────────────────────────────────┐
│          Clicks: 0/5                │  ← Progress visible
│        Points left: 5               │
│                                     │
│                                     │
│  ◎ ← Red crosshair                  │
│  (30,30)                            │
│                                     │
│                                     │
│   [Back button DISABLED]            │  ← Modal lock active
└─────────────────────────────────────┘
         ↓ User taps 5 times

┌─────────────────────────────────────┐
│          Clicks: 5/5                │
│        Points left: 5               │
│                                     │
│                                     │
│  ◉ ← GREEN flash (300ms)            │
│                                     │
│                                     │
└─────────────────────────────────────┘
         ↓ Auto-advance

┌─────────────────────────────────────┐
│          Clicks: 0/5                │
│        Points left: 4               │  ← Decremented
│                                     │
│                                 ◎   │  ← New position
│                           (290,30)  │
│                                     │
│   [Still MODAL LOCKED]              │
└─────────────────────────────────────┘
         ↓ Repeat until all points done

┌─────────────────────────────────────┐
│  Calibration Complete!       < Back │  ← Back enabled
│                                     │
│          ✓                          │
│                                     │
│  Calibration data saved             │
│       ┌──────────┐                  │
│       │   Done   │                  │
│       └──────────┘                  │
└─────────────────────────────────────┘
```

---

## Technical Details

### Modal Lock Entry Points

```cpp
// Enter modal lock
if (currentState == CAL_CALIBRATING || currentState == CAL_POINT_FLASH) {
    collectSample(point.x, point.y);
    return;  // Early exit prevents all other code
}
```

### Modal Lock Exit Points

1. **Complete all points** → `setState(CAL_COMPLETE)` → Lock released
2. **System error/reset** → `reset()` → Back to INTRO
3. **App navigation** → Screen manager switches away

Note: There is NO manual exit during calibration. This is intentional.

### Progress Text Rendering Flow

```
User taps → collectSample() → screenIsDirty = true 
→ ScreenManager.render() → calibrationWizard->draw()
→ renderDynamicContent() → drawProgressText() + drawCalibrationPoint()
```

### Memory Footprint

- Two 20-byte char buffers (40 bytes total)
- Created on stack in `drawProgressText()`
- No heap allocation, no memory leaks
- Efficient sprintf formatting

---

## Debug Output Example

```
[CalWizard] Touch: (45,50) in state 1
[CalWizard] MODAL LOCK - Only sample collection allowed
Sample collected: Point 0, Sample 1/5

[CalWizard] Touch: (48,52) in state 1
[CalWizard] MODAL LOCK - Only sample collection allowed
Sample collected: Point 0, Sample 2/5

[CalWizard] Touch: (250,25) in state 1  ← Top-right corner!
[CalWizard] MODAL LOCK - Only sample collection allowed  ← Back button ignored!
Sample collected: Point 0, Sample 3/5
```

Even tapping the Back button location registers as a sample, not navigation.

---

## Configuration Constants

```cpp
#define COLOR_TEXT_DIM TFT_DARKGREY  // Progress text color (subtle)

// Progress text positions
Y position line 1: 15 pixels from top
Y position line 2: 30 pixels from top
X position: SCREEN_WIDTH / 2 (160px - centered)
```

---

## Files Modified

- `src/CalibrationWizardScreen.cpp`
  - Added `COLOR_TEXT_DIM` constant
  - Refactored `onTouchEvent()` with modal lock logic
  - Implemented progress display in `drawProgressText()`
  - Updated `renderDynamicContent()` to call `drawProgressText()`

---

## Benefits Summary

### Modal Lock
✅ Prevents accidental exits
✅ Protects calibration data
✅ Clearer user intent
✅ Simpler error handling
✅ More professional feel

### Progress UI
✅ Clear feedback on progress
✅ Reduces user anxiety ("How many more?")
✅ Subtle, non-intrusive design
✅ Updates in real-time
✅ Complements modal lock (user knows they're locked in)

Together, these features create a focused, professional calibration experience that guides users through the process without distraction or confusion.
