# Incorrect PIN Visual Feedback Animation

## Overview
Comprehensive visual feedback sequence for incorrect PIN entry with shake animation and error text.

## Animation Sequence

### Timeline
```
0ms        400ms                    1500ms
|-----------|-------------------------|
  SHAKE      ERROR TEXT DISPLAY       RESET
  
Phase 1: Horizontal Shake (0-400ms)
- 4 indicator dots shake horizontally
- Amplitude: ±12 pixels
- Frequency: ~12 Hz (rapid vibration)
- Dots remain RED during shake

Phase 2: Error Text Display (400-1500ms)
- Shake stops
- "WRONG PASSWORD" text appears in red
- Text positioned below indicator dots
- Dots remain RED and filled

Phase 3: Reset (1500ms+)
- Error text cleared
- All 4 indicator dots reset to EMPTY state
- Screen ready for next PIN attempt
```

## Implementation Details

### Constants
```cpp
ERROR_ANIMATION_DURATION = 400ms   // Shake duration
ERROR_DISPLAY_DURATION = 1500ms    // Total animation time
SHAKE_AMPLITUDE = 12px             // Horizontal shake distance
```

### Shake Algorithm
```cpp
// High-frequency sine wave for rapid vibration
float phase = elapsed * 0.03;  // ~12 Hz frequency
shakeOffset = (int)(sin(phase) * SHAKE_AMPLITUDE);
```

### State Management
- `showingError` flag triggers animation
- `errorAnimationStart` timestamp tracks animation progress
- `update()` continuously redraws during animation
- `forceFullRedraw` triggered after animation completes

## Visual Design

### Error State Colors
- **Indicator Dots**: RED (`COLOR_ERROR = 0xF800`)
- **Error Text**: RED (`COLOR_ERROR`)
- **Background**: INK (`COLOR_INK = 0x0841`)

### Text Styling
- Font Size: 1 (small)
- Alignment: Top-Center
- Position: `DOTS_Y + 25` (25px below indicator dots)

## Testing Instructions

### Test Wrong PIN
1. Enter any 4-digit PIN except 1234
2. Observe shake animation (0-400ms)
3. Verify "WRONG PASSWORD" text appears (400-1500ms)
4. Confirm dots reset to empty after 1.5 seconds

### Expected Serial Output
```
[PIN_ENTRY] Validating PIN: 5555
[PIN_ENTRY] ✗ PIN INCORRECT - Starting shake animation + error text
[PIN_ERROR] Error state activated at t=12345 ms
[PIN_ERROR] Animation sequence:
[PIN_ERROR]   0-400ms: Horizontal shake (amplitude=12px)
[PIN_ERROR]   400-1500ms: Display 'WRONG PASSWORD' text
[PIN_ERROR]   1500ms+: Reset to empty state
[PIN_ERROR] Shake animation: elapsed=50ms, phase=1.50, offset=11px
[PIN_ERROR] Shake animation: elapsed=100ms, phase=3.00, offset=-11px
[PIN_ERROR] Shake animation: elapsed=150ms, phase=4.50, offset=9px
...
[PIN_ERROR] Displaying 'WRONG PASSWORD' text
...
[PIN_ERROR] Animation complete - resetting to empty state
[PIN_ENTRY] PIN cleared
```

## User Experience Flow

1. **User enters wrong PIN** → 4th digit auto-validates
2. **Immediate visual shock** → Dots turn red and shake violently
3. **Error confirmation** → "WRONG PASSWORD" text reinforces the mistake
4. **Graceful recovery** → Auto-reset to empty state, ready to retry

## Technical Notes

### Continuous Redraw During Animation
- `update()` sets `screenIsDirty = true` while `showingError` is active
- Main render loop continuously calls `draw()` → `drawDynamicData()`
- Creates smooth animation without blocking touch input

### Cleared Area Sizing
```cpp
// Clear 60px tall area to include dots + error text
tft->fillRect(0, DOTS_Y - 15, SCREEN_WIDTH, 60, COLOR_INK);
```

### Animation Independence
- Animation runs in `update()` without blocking touch processing
- User cannot add digits during error state (touch handler checks `unlocked`)
- Auto-reset ensures user doesn't need to manually clear

## Future Enhancements
- [ ] Haptic feedback (buzzer/vibration motor) during shake
- [ ] Attempt counter (e.g., "2/3 attempts remaining")
- [ ] Exponential backoff after multiple failed attempts
- [ ] Custom error messages based on context
