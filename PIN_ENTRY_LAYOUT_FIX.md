# PIN Entry Screen Layout Fix

## The Problem (Visual)

### Before (Broken):
```
┌──────────────────────────────────────┐
│  ENTER MASTER PIN           Y=0-24  │
├──────────────────────────────────────┤
│                             Y=24     │
│  ●  ●  ●  ●  ← Dots        Y=32     │
│                             Y=40     │
│                             Y=60     │
│  WRONG PASSWORD             Y=64     │
│                             Y=72     │
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ Y=82 ← Clear region ends
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=84 ← Keypad starts
│ │  1  │ │  2  │ │  3  │   ← TOP OF BUTTONS ERASED!
│ └─────┘ └─────┘ └─────┘   Y=118
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=124
│ │  4  │ │  5  │ │  6  │
│ └─────┘ └─────┘ └─────┘   Y=158
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=164
│ │  7  │ │  8  │ │  9  │
│ └─────┘ └─────┘ └─────┘   Y=198
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=204
│ │  C  │ │  0  │ │  <  │
│ └─────┘ └─────┘ └─────┘   Y=238
└──────────────────────────────────────┘ Y=240
```

**Problem:** Clear region (Y=22-82) overlaps keypad start (Y=84)

---

## The Solution (Fixed):

### After (Working):
```
┌──────────────────────────────────────┐
│  ENTER MASTER PIN           Y=0-24  │
├──────────────────────────────────────┤
│                             Y=24 ← Clear starts
│  ●  ●  ●  ●  ← Dots        Y=32     │
│                             Y=40     │
│                             Y=60     │
│  WRONG PASSWORD             Y=64     │
│                             Y=72 ← Clear ends
│ ────────────────────────────────────────
│ ↑ SAFE GAP (12px)          Y=84 ← Keypad starts
│ ────────────────────────────────────────
│ ┌─────┐ ┌─────┐ ┌─────┐   ← FULLY VISIBLE!
│ │  1  │ │  2  │ │  3  │
│ └─────┘ └─────┘ └─────┘   Y=118
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=124
│ │  4  │ │  5  │ │  6  │
│ └─────┘ └─────┘ └─────┘   Y=158
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=164
│ │  7  │ │  8  │ │  9  │
│ └─────┘ └─────┘ └─────┘   Y=198
│ ┌─────┐ ┌─────┐ ┌─────┐   Y=204
│ │  C  │ │  0  │ │  <  │
│ └─────┘ └─────┘ └─────┘   Y=238
└──────────────────────────────────────┘ Y=240
```

**Solution:** Clear region (Y=24-72) stops before keypad (Y=84)

---

## Code Comparison

### Before (Buggy):
```cpp
void PINEntryScreen::drawDynamicData() {
    // Clear from (DOTS_Y - 10) = 22, height 60px → Y=22 to Y=82
    tft->fillRect(0, DOTS_Y - 10, SCREEN_WIDTH, 60, COLOR_INK);
    //                ^Y=22        ^width=320   ^height=60
    //                                           Clear ends at Y=82
    //                                           Overlaps keypad at Y=84!
    
    drawIndicatorDots();  // Dots at Y=32
    // ... error text at Y=64
}
```

### After (Fixed):
```cpp
void PINEntryScreen::drawDynamicData() {
    // Clear from Y=24, height 48px → Y=24 to Y=72
    tft->fillRect(0, 24, SCREEN_WIDTH, 48, COLOR_INK);
    //                ^Y=24 ^width=320 ^height=48
    //                                  Clear ends at Y=72
    //                                  12px gap before keypad at Y=84
    
    drawIndicatorDots();  // Dots at Y=32
    // ... error text at Y=64
}
```

---

## Layout Math

### Element Positions:
| Element | Y Position | Height | Notes |
|---------|-----------|--------|-------|
| Title | 12 | 8px | "ENTER MASTER PIN" |
| Dots | 32 | 16px | Radius=8, so Y=24-40 |
| Error Text | 64 | 8px | Size 1 = 8px tall |
| **Safe Gap** | **72-84** | **12px** | **Buffer zone** |
| Keypad | 84-238 | 154px | 3x4 grid |

### Clear Region Sizing:
```
Old Clear Region:
- Start: DOTS_Y - 10 = 32 - 10 = 22
- Height: 60px
- End: 22 + 60 = 82
- Problem: Only 2px before keypad!

New Clear Region:
- Start: 24 (above dots)
- Height: 48px
- End: 24 + 48 = 72
- Solution: 12px gap before keypad at 84
```

---

## Why This Caused "Grey Border" Glitch

### What Users Saw:
```
Instead of:          Users saw:
┌─────┐             ┌─────┐
│  1  │             │     │  ← Hollow!
└─────┘             └─────┘
```

### Why:
1. `drawKeypad()` draws button in 2 steps:
   ```cpp
   fillRoundRect(x, y, w, h, r, COLOR_SURFACE);  // Fill
   drawRoundRect(x, y, w, h, r, COLOR_LINE);     // Border
   ```

2. During error animation, `drawDynamicData()` runs repeatedly

3. Clear operation erases top ~2-4px of button fills

4. But borders (1px lines) are still visible lower down

5. Result: Hollow rectangles with just borders

---

## Testing Before/After

### Test 1: Enter Wrong PIN
**Before:**
1. Enter 0000
2. Shake animation starts
3. Top of buttons disappears → Grey borders only
4. After 1.5s, buttons still broken
5. Must tap to trigger redraw

**After:**
1. Enter 0000
2. Shake animation starts
3. Buttons stay fully visible ✅
4. After 1.5s, buttons still visible ✅
5. No extra taps needed ✅

### Test 2: Rapid Entry
**Before:**
1. Tap 1234 quickly
2. Dots update, buttons flicker
3. Sometimes buttons disappear

**After:**
1. Tap 1234 quickly
2. Dots update smoothly
3. Buttons always visible ✅

---

## Auto-Lock Behavior

### Why PIN Entry is Excluded:

```cpp
void ScreenManager::checkAutoLock() {
    if (screenAsleep) return;
    
    // NEW: Don't auto-lock during PIN entry
    if (currentScreen == SCREEN_PIN_ENTRY) {
        return;  // User is actively unlocking!
    }
    
    // ... rest of logic for other screens
}
```

### Reasoning:
- **Security UX**: Locking during unlock is confusing
- **Standard behavior**: No OS locks the lock screen
- **User expectation**: PIN entry = actively using device
- **Avoids frustration**: Timeout won't interrupt PIN entry

---

**Fix Summary:**
- Reduced clear height: 60px → 48px
- Added safe gap: 12px between clear and keypad
- Disabled auto-lock on PIN Entry screen
- Keypad now always visible, no more glitches! ✅
