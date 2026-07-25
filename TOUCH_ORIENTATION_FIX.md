# Touch Orientation Fix - Implementation Details

## Problem Identified
The display was rotating correctly, but touch input coordinates were not inverting to match the rotated display. This caused touch events to register at incorrect positions in Landscape Inverted mode.

## Root Cause
The touch coordinate transformation was implemented, but there were potential issues:
1. Touch controller's `setRotation()` might have been conflicting with manual transformation
2. Off-by-one error in coordinate inversion (needed `SCREEN_WIDTH - 1` not `SCREEN_WIDTH`)
3. Debug logging was insufficient to diagnose the transformation

## Solution Implemented

### 1. Touch Controller Rotation Strategy
**Keep touch controller at base rotation (1) always:**
- Touch controller stays at rotation 1 (standard landscape)
- Calibration values remain valid for rotation 1
- Manual coordinate transformation handles rotation 3 (inverted)

```cpp
void TouchManager::begin() {
    touch->begin();
    touch->setRotation(1);  // Fixed at base landscape
    currentRotation = 1;
    // Manual transforms will handle other orientations
}
```

### 2. Corrected Coordinate Inversion
**Fixed off-by-one error and added comprehensive logging:**

```cpp
void TouchManager::applyOrientationTransform(TouchPoint& point) {
    if (currentRotation == 3) {  // ORIENTATION_LANDSCAPE_INVERTED (270°)
        int16_t originalX = point.x;
        int16_t originalY = point.y;
        
        // Correct inversion: subtract from (dimension - 1)
        point.x = SCREEN_WIDTH - 1 - point.x;   // 320 - 1 = 319
        point.y = SCREEN_HEIGHT - 1 - point.y;  // 240 - 1 = 239
        
        DEBUG_LOGF("Touch transform [rotation=%d]: (%d,%d) -> (%d,%d)\n", 
                   currentRotation, originalX, originalY, point.x, point.y);
    } else {
        DEBUG_LOGF("Touch passthrough [rotation=%d]: (%d,%d)\n", 
                   currentRotation, point.x, point.y);
    }
}
```

### 3. Enhanced Debug Logging
**Added detailed logging at each transformation stage:**

```cpp
// In readRawTouch():
DEBUG_LOGF("Touch raw->mapped: sensor(%d,%d) -> screen(%d,%d) [pressure=%d]\n", 
           p.x, p.y, point.x, point.y, p.z);

// In applyOrientationTransform():
DEBUG_LOGF("Touch transform [rotation=%d]: (%d,%d) -> (%d,%d)\n", 
           currentRotation, originalX, originalY, point.x, point.y);
```

## Touch Processing Pipeline

### Standard Landscape (Rotation 1)
```
1. Raw sensor: (3500, 2000) [example values]
2. Map to screen: (280, 160) using calibration
3. Pass through transform: (280, 160) [no change]
4. Constrain to bounds: (280, 160)
5. Deliver to UI: (280, 160)
```

### Inverted Landscape (Rotation 3)
```
1. Raw sensor: (3500, 2000) [same physical touch]
2. Map to screen: (280, 160) using calibration
3. Apply transform: (319-280, 239-160) = (39, 79)
4. Constrain to bounds: (39, 79)
5. Deliver to UI: (39, 79)
```

## Mathematical Verification

### Screen Dimensions
- Width: 320 pixels (0-319)
- Height: 240 pixels (0-239)

### Inversion Formula
```
For rotation 3 (inverted landscape):
  inverted_x = (SCREEN_WIDTH - 1) - original_x
  inverted_y = (SCREEN_HEIGHT - 1) - original_y

Examples:
  Top-left (0, 0) → Bottom-right (319, 239) ✓
  Bottom-right (319, 239) → Top-left (0, 0) ✓
  Center (160, 120) → Center (159, 119) ≈ center ✓
  Top-right (319, 0) → Bottom-left (0, 239) ✓
```

### Why "- 1" is Critical
Screen coordinates are 0-indexed:
- Valid X range: 0 to 319 (not 0 to 320)
- Valid Y range: 0 to 239 (not 0 to 240)

Without the "- 1":
- Touch at (0, 0) would map to (320, 240) → out of bounds!
- Touch at (319, 239) would map to (1, 1) → correct
- **This creates asymmetry and accuracy issues**

With the "- 1":
- Touch at (0, 0) maps to (319, 239) → perfect mirror ✓
- Touch at (319, 239) maps to (0, 0) → perfect mirror ✓
- **Perfect symmetrical inversion**

## Testing Strategy

### Enable Debug Logging
Set `DEV_MODE 1` in Config.h to see transformation logs:
```cpp
#define DEV_MODE 1
```

### Test Procedure

#### 1. Standard Landscape (Rotation 1)
**Settings → Orientation → Select "Landscape"**

Expected behavior:
```
Touch top-left corner → registers at top-left
Touch top-right corner → registers at top-right
Touch bottom-left corner → registers at bottom-left
Touch bottom-right corner → registers at bottom-right
Touch center → registers at center
```

Serial output:
```
Touch passthrough [rotation=1]: (10,15)
Touch passthrough [rotation=1]: (310,15)
Touch passthrough [rotation=1]: (10,225)
Touch passthrough [rotation=1]: (310,225)
Touch passthrough [rotation=1]: (160,120)
```

#### 2. Inverted Landscape (Rotation 3)
**Settings → Orientation → Select "Landscape Inverted"**

Expected behavior:
```
Touch physical top-left → registers at bottom-right
Touch physical top-right → registers at bottom-left
Touch physical bottom-left → registers at top-right
Touch physical bottom-right → registers at top-left
Touch physical center → registers at center
```

Serial output:
```
Touch transform [rotation=3]: (10,15) -> (309,224)
Touch transform [rotation=3]: (310,15) -> (9,224)
Touch transform [rotation=3]: (10,225) -> (309,14)
Touch transform [rotation=3]: (310,225) -> (9,14)
Touch transform [rotation=3]: (160,120) -> (159,119)
```

### Specific Tests

#### Test 1: Settings Button (Home Screen)
**Standard Landscape:**
- Settings button: bottom-right area
- Touch bottom-right → Settings screen opens ✓

**Inverted Landscape:**
- Settings button: now at top-left (visually rotated)
- Touch physical top-left → Settings screen opens ✓

#### Test 2: Touch Test Screen
**Standard Landscape:**
- Touch and hold anywhere
- Dot appears at touch location ✓
- Coordinates match visual position ✓

**Inverted Landscape:**
- Touch and hold anywhere
- Dot appears at touch location (inverted) ✓
- Coordinates match visual position ✓

#### Test 3: Orientation Screen Buttons
**In Inverted Mode:**
- "Landscape" button (now at bottom visually)
- Touch bottom button → switches to standard landscape ✓
- Display rotates AND touch remaps ✓

#### Test 4: Navigation in Inverted Mode
Test all screens in inverted landscape:
- [ ] Home screen - all 4 buttons work
- [ ] Settings screen - all buttons work
- [ ] Touch Test screen - dot follows touch
- [ ] Calibration screen - targets register correctly
- [ ] Orientation screen - both buttons work
- [ ] Back buttons - all work correctly

## Files Modified

### src/TouchManager.cpp
**Changes:**
1. Fixed `begin()` - touch controller stays at rotation 1
2. Fixed `updateRotation()` - tracks rotation without changing touch controller
3. Fixed `applyOrientationTransform()` - corrected inversion formula (added - 1)
4. Enhanced `readRawTouch()` - added comprehensive debug logging

## Troubleshooting

### If touch is still incorrect:

#### Symptom: Touch registers in wrong quadrant
**Check:** Serial monitor for transformation logs
```
Touch transform [rotation=3]: (X1,Y1) -> (X2,Y2)
```
- Verify X2 = 319 - X1
- Verify Y2 = 239 - Y1

#### Symptom: Touch is inverted but with offset
**Check:** Calibration values in Config.h
```cpp
#define TOUCH_CAL_X_MIN 249
#define TOUCH_CAL_X_MAX 3810
#define TOUCH_CAL_Y_MIN 388
#define TOUCH_CAL_Y_MAX 3686
```
- Re-calibrate using Settings → Calibrate Touch
- Test in standard landscape first
- Then test inverted landscape

#### Symptom: Touch works in landscape but not inverted
**Check:** currentRotation value
```
DEBUG_LOGF("TouchManager: Rotation tracking updated to %d\n", rotation);
```
- Should show "3" when in inverted mode
- If shows "1", settings not being applied

#### Symptom: No transformation happening
**Check:** Settings manager connection
```cpp
// In main.cpp or ScreenManager::begin()
touch->setSettingsManager(settingsManager);
```
- Verify this is called during initialization
- Verify settings manager is not nullptr

## Verification Checklist

### Pre-Upload Verification
- [ ] DEV_MODE enabled in Config.h
- [ ] Code compiles without warnings
- [ ] Touch controller stays at rotation 1
- [ ] Transformation uses (dimension - 1)

### Post-Upload Verification
- [ ] Serial monitor shows debug logs
- [ ] Standard landscape touch works
- [ ] Inverted landscape touch works
- [ ] All buttons work in both modes
- [ ] Touch Test dot follows finger in both modes
- [ ] No phantom touches or crashes

### Complete Integration Test
- [ ] Boot in standard landscape (default)
- [ ] Navigate to Settings → Orientation
- [ ] Switch to Inverted Landscape
- [ ] Test all screens and buttons
- [ ] Switch back to Standard Landscape
- [ ] Test all screens and buttons
- [ ] Power cycle - verify persistence
- [ ] Boot in saved orientation

## Expected Serial Output

### During Initialization
```
TouchManager initialized (base rotation 1, manual transforms for other modes)
SettingsManager: Settings loaded
TouchManager: Rotation tracking updated to 1
Hardware: Display orientation set to 1
ScreenManager initialized - Global Standard Active
```

### When Switching to Inverted
```
Orientation: Landscape Inverted selected
SettingsManager: Orientation set to 3
SettingsManager: Settings saved
Display rotation set to Landscape Inverted (3)
TouchManager: Rotation tracking updated to 3 (touch controller stays at 1)
Orientation changed applied: 3
```

### When Touching in Inverted Mode
```
Touch raw->mapped: sensor(3500,2000) -> screen(280,160) [pressure=1500]
Touch transform [rotation=3]: (280,160) -> (39,79)
[DEBUG_INPUT] X: 39, Y: 79, Type: TOUCH_DOWN, Screen: HOME
[DEBUG_BUTTON] HIT: 'Settings' on HOME screen
```

## Success Criteria

✅ **Touch accuracy in standard landscape:** < 5 pixel error
✅ **Touch accuracy in inverted landscape:** < 5 pixel error
✅ **All UI elements respond correctly:** All buttons work in both modes
✅ **No coordinate drift:** Multiple touches at same location register consistently
✅ **Clean serial logs:** Clear transformation tracking visible
✅ **Persistence works:** Orientation survives reboot

---

**Fix Date:** Current session
**Issue:** Touch coordinates not inverting with display rotation
**Solution:** Keep touch at base rotation 1, manually transform for rotation 3
**Status:** Ready for testing
