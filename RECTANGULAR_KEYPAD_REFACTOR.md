# PIN Entry Screen - Rectangular Grid Refactor

## Overview
Complete redesign from cramped Apple Watch circular buttons to a modern, highly readable rectangular grid layout optimized for the 320x240 ILI9341 display.

## Layout Improvements

### Before (Apple Watch Circular)
```
KEY_SIZE: 38px (circular)
KEY_SPACING: 6px
KEYPAD_START_Y: 70px
Touch Target: π × (19px)² ≈ 1,134px²
Layout: Cramped, difficult to tap accurately
```

### After (Rectangular Grid)
```
KEY_WIDTH: 80px
KEY_HEIGHT: 34px
KEY_SPACING_X: 14px
KEY_SPACING_Y: 6px
KEYPAD_START_Y: 84px
Touch Target: 80 × 34 = 2,720px² (140% larger!)
Layout: Clean, modern, easy to tap
```

## New Layout Constants

### Indicator Dots
```cpp
DOTS_Y = 32          // Moved higher for better spacing
DOT_RADIUS = 8       // Same size
DOT_SPACING = 36     // Wider spacing (was 24)
```

### Feedback Text
```cpp
FEEDBACK_Y = 64      // Dedicated area for "WRONG PASSWORD"
```

### Rectangular Keypad Grid
```cpp
KEYPAD_START_Y = 84
KEY_WIDTH = 80       // Wide buttons (was 38 circular)
KEY_HEIGHT = 34      // Comfortable height
KEY_SPACING_X = 14   // Horizontal gap between buttons
KEY_SPACING_Y = 6    // Vertical gap between rows
GRID_ORIGIN_X = 26   // Auto-centered: (320 - 268) / 2
```

### Grid Dimensions
```
Total Width:  3 × 80 + 2 × 14 = 268px
Total Height: 4 × 34 + 3 × 6  = 154px
Start X: (320 - 268) / 2 = 26px
End X: 26 + 268 = 294px
Start Y: 84px
End Y: 84 + 154 = 238px ✓ Fits perfectly in 240px height!
```

## Button Layout (3×4 Grid)

```
┌────────────────────────────────────┐
│      ENTER MASTER PIN              │  Y: 12
│                                    │
│      ○  ○  ○  ○                    │  Y: 32 (indicator dots)
│                                    │
│      WRONG PASSWORD                │  Y: 64 (error text)
│                                    │
│  ┌──────┐ ┌──────┐ ┌──────┐       │  Y: 84 (row 1)
│  │  1   │ │  2   │ │  3   │       │
│  └──────┘ └──────┘ └──────┘       │
│                                    │
│  ┌──────┐ ┌──────┐ ┌──────┐       │  Y: 124 (row 2)
│  │  4   │ │  5   │ │  6   │       │
│  └──────┘ └──────┘ └──────┘       │
│                                    │
│  ┌──────┐ ┌──────┐ ┌──────┐       │  Y: 164 (row 3)
│  │  7   │ │  8   │ │  9   │       │
│  └──────┘ └──────┘ └──────┘       │
│                                    │
│  ┌──────┐ ┌──────┐ ┌──────┐       │  Y: 204 (row 4)
│  │  C   │ │  0   │ │  <   │       │
│  └──────┘ └──────┘ └──────┘       │
└────────────────────────────────────┘
   X: 26    106      186      266
```

## Button Rendering

### Modern Rounded Rectangles
```cpp
// Clean, modern aesthetic
tft->fillRoundRect(x, y, KEY_WIDTH, KEY_HEIGHT, 6, COLOR_SURFACE);
tft->drawRoundRect(x, y, KEY_WIDTH, KEY_HEIGHT, 6, COLOR_LINE);

// Centered text (size 2)
tft->setTextDatum(MC_DATUM);
tft->drawString(label, x + KEY_WIDTH/2, y + KEY_HEIGHT/2);
```

### Visual Style
- **Border Radius**: 6px (subtle rounding)
- **Fill Color**: COLOR_SURFACE (#141821 - dark gray)
- **Border Color**: COLOR_LINE (#262C3A - light gray)
- **Text Color**: COLOR_IVORY (#F4EFE4 - off-white)
- **Text Size**: 2 (readable without being huge)

## Touch Hit-Testing

### Rectangular Bounds Check
```cpp
// Simple and accurate
if (x >= bx && x <= (bx + KEY_WIDTH) && 
    y >= by && y <= (by + KEY_HEIGHT)) {
    // Button hit!
}
```

### Key Mapping
```cpp
const char* keys[12] = {
    "1", "2", "3",  // Row 1
    "4", "5", "6",  // Row 2
    "7", "8", "9",  // Row 3
    "C", "0", "<"   // Row 4 (Clear, Zero, Backspace)
};

// Return codes:
// 0-9:  Digit keys
// -2:   Backspace (<)
// -3:   Clear (C)
// -1:   No hit
```

## Shake Animation Improvements

### Refined Algorithm
```cpp
// Phase-based oscillation (40ms per phase)
int phase = (elapsed / 40) % 4;
if (phase == 0) shakeOffset = -6;   // Left
else if (phase == 1) shakeOffset = 6;   // Right
else if (phase == 2) shakeOffset = -3;  // Left (reduced)
else shakeOffset = 3;                   // Right (reduced)

// Duration: 350ms (was 400ms)
// Amplitude: ±6px (was ±12px - more controlled)
```

## Added Features

### Clear Button
- **Position**: Bottom-left (row 4, col 0)
- **Label**: "C"
- **Function**: Clears all entered digits immediately
- **Behavior**: Resets PIN buffer and indicator dots

### Error State Blocking
- Touch input **blocked** during error animation
- Prevents user from spamming incorrect PINs
- Auto-unblocks after 1.5 second animation completes

## Build Metrics

### Memory Usage
- **RAM**: 54.8% (44,896 bytes / 81,920 bytes)
- **Flash**: 36.0% (376,163 bytes / 1,044,464 bytes)
- **Status**: ✓ SUCCESS

### Code Changes
- Removed circular button rendering logic
- Removed circular distance calculations
- Simplified hit-testing (rectangular bounds only)
- Added Clear button handler
- Refined shake animation timing

## Testing Checklist

### Visual Testing
- [ ] All 12 buttons visible and properly aligned
- [ ] Buttons have rounded corners (6px radius)
- [ ] Text centered in each button
- [ ] Indicator dots properly spaced (36px apart)
- [ ] "WRONG PASSWORD" appears at Y=64

### Touch Testing
- [ ] Tap each digit 0-9 (should fill dots immediately)
- [ ] Tap Clear button (should reset all dots)
- [ ] Tap Backspace (should remove last dot)
- [ ] Verify no touch response during shake animation

### Animation Testing
- [ ] Enter wrong PIN (not 1234)
- [ ] Dots shake ±6px for 350ms
- [ ] "WRONG PASSWORD" appears after shake
- [ ] Auto-reset after 1.5 seconds
- [ ] Touch blocked during animation

### Edge Cases
- [ ] Rapid tapping (should queue correctly)
- [ ] Tap between buttons (should not register)
- [ ] Tap outside keypad area (should not register)

## Serial Output Examples

### Initialization
```
[PIN_ENTRY] Screen initialized - RECTANGULAR GRID LAYOUT
[PIN_ENTRY] Layout: KEY_WIDTH=80, KEY_HEIGHT=34, GRID_ORIGIN_X=26
[PIN_ENTRY] Grid dimensions: 3x4 buttons, spacing: X=14 Y=6
[PIN_ENTRY] Keypad bounds: X=26-294, Y=84-238
[PIN_KEYPAD] Rectangular grid drawn: 3x4 buttons, 80x34px each
```

### Button Tap
```
[PIN_ENTRY_TOUCH] handleTouch called: x=66, y=124, valid=YES
[PIN_HIT_TEST] Touch: (66, 124), Grid bounds: X=26-294, Y=84-238
[PIN_HIT_TEST] MATCH: Button 3 ('4') at grid (0,1), bounds X=26-106 Y=124-158
[PIN_ENTRY_TOUCH] *** DIGIT KEY PRESSED: 4 ***
```

### Clear Button
```
[PIN_ENTRY_TOUCH] handleTouch called: x=66, y=204, valid=YES
[PIN_HIT_TEST] MATCH: Button 9 ('C') at grid (0,3), bounds X=26-106 Y=204-238
[PIN_ENTRY_TOUCH] *** CLEAR KEY PRESSED ***
[PIN_ENTRY] PIN cleared
```

### Error Animation
```
[PIN_ENTRY] ✗ PIN INCORRECT - Starting shake animation + error text
[PIN_ERROR] Error state activated at t=12345 ms
[PIN_ERROR] Animation sequence:
[PIN_ERROR]   0-350ms: Horizontal shake (amplitude=6px)
[PIN_ERROR]   350-1500ms: Display 'WRONG PASSWORD' text
[PIN_ERROR]   1500ms+: Reset to empty state
[PIN_ERROR] Shake animation: elapsed=40ms, phase=1, offset=6px
[PIN_ERROR] Shake animation: elapsed=80ms, phase=2, offset=-3px
...
[PIN_ERROR] Displaying 'WRONG PASSWORD' text
...
[PIN_ERROR] Animation complete - resetting to empty state
```

## Benefits Summary

✓ **140% larger touch targets** (2,720px² vs 1,134px²)  
✓ **Easier to tap accurately** (rectangular vs circular)  
✓ **Modern, clean aesthetic** (rounded rectangles)  
✓ **Better spacing** (14px horizontal gaps)  
✓ **Clear button added** (quick reset)  
✓ **Improved error handling** (blocks input during animation)  
✓ **Refined shake** (more controlled ±6px)  
✓ **Better visual hierarchy** (dedicated error text area)

## Next Steps

1. **Flash firmware** to device
2. **Test touch accuracy** with new rectangular buttons
3. **Verify visual layout** matches design
4. **Test error animation** with wrong PIN
5. **Validate Clear button** functionality
