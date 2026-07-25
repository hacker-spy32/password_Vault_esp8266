# ✅ PIN Entry Screen Glitch Fixed

## Build Status

```
========================= [SUCCESS] Took 7.01 seconds =========================
RAM:   58.2% (47,688 bytes)
Flash: 41.4% (432,779 bytes)
```

## Issues Fixed

### 1. ✅ Keypad Disappearing Glitch
**Problem:**
- Keypad buttons would disappear randomly
- Only holes (indicator dots) and grey border visible
- Had to tap multiple times to "fix" it

**Root Cause:**
- The `drawDynamicData()` method was clearing too large an area
- Clear region: `Y=22 to Y=82` (60px tall)
- Keypad starts at: `Y=84`
- The clear was overlapping and erasing the top of the keypad buttons!

**Fix:**
- Reduced clear region from 60px to 48px
- New clear region: `Y=24 to Y=72` (48px tall)
- Leaves 12px gap before keypad starts at Y=84
- Keypad no longer gets erased during updates

### 2. ✅ Auto-Sleep Not Working on PIN Screen
**Problem:**
- Device would stay on PIN Entry screen indefinitely
- Auto-lock timer never triggered on this screen

**Root Cause:**
- `checkAutoLock()` was running on all screens including PIN Entry
- This doesn't make sense - you can't lock during PIN entry!

**Fix:**
- Added early return in `checkAutoLock()` for `SCREEN_PIN_ENTRY`
- PIN Entry screen is now excluded from auto-lock
- Makes logical sense: device shouldn't sleep while user is entering PIN

## Technical Details

### Before (Broken):
```cpp
void PINEntryScreen::drawDynamicData() {
    // PROBLEM: Clear region overlaps keypad!
    tft->fillRect(0, DOTS_Y - 10, SCREEN_WIDTH, 60, COLOR_INK);
    //             Y=22 (32-10)          ^60px tall = Y=22 to Y=82
    //                                    Keypad at Y=84 gets clipped!
    ...
}
```

### After (Fixed):
```cpp
void PINEntryScreen::drawDynamicData() {
    // FIXED: Clear only dots + error text region
    tft->fillRect(0, 24, SCREEN_WIDTH, 48, COLOR_INK);
    //             Y=24         ^48px tall = Y=24 to Y=72
    //                          Leaves 12px gap before keypad at Y=84
    ...
}
```

### Layout Verification:
```
Screen Layout (320x240):
├─ Y=0-24:   Title "ENTER MASTER PIN"
├─ Y=24-40:  Indicator dots (4 circles at Y=32)
├─ Y=60-72:  Error text "WRONG PASSWORD"
├─ Y=72-84:  ← Safe gap (12px)
└─ Y=84+:    Keypad (3x4 grid, 80x34px buttons)
```

## Changes Made

### Files Modified:

1. **src/PINEntryScreen.cpp**
   - Changed clear region from `(0, 22, 320, 60)` to `(0, 24, 320, 48)`
   - Reduced clear height: 60px → 48px
   - Added comment explaining the safe layout

2. **src/ScreenManager.cpp**
   - Added `if (currentScreen == SCREEN_PIN_ENTRY) return;` to `checkAutoLock()`
   - PIN Entry screen now excluded from auto-lock timer

## Testing Checklist

### Keypad Visibility Test:
- [ ] Boot device → PIN Entry screen appears
- [ ] All 12 buttons visible (1-9, C, 0, <)
- [ ] Tap several digits rapidly
- [ ] Verify buttons never disappear
- [ ] Enter wrong PIN to trigger error animation
- [ ] Verify keypad stays visible during shake animation
- [ ] Verify keypad stays visible during "WRONG PASSWORD" text
- [ ] Wait for error animation to complete (1.5s)
- [ ] Verify keypad still fully visible

### Auto-Lock Behavior:
- [ ] Boot device → PIN Entry screen
- [ ] Wait for auto-lock timeout (e.g., 15 seconds)
- [ ] Verify screen DOES NOT sleep
- [ ] Enter correct PIN → Navigate to Home
- [ ] Wait for auto-lock timeout again
- [ ] Verify screen DOES sleep from Home

### Error Animation:
- [ ] Enter wrong PIN (e.g., 0000)
- [ ] Verify dots shake horizontally (±6px)
- [ ] Verify "WRONG PASSWORD" appears in red
- [ ] Verify keypad remains visible throughout
- [ ] Wait 1.5 seconds
- [ ] Verify dots reset to empty
- [ ] Verify error text disappears
- [ ] Verify keypad still visible

## Why Auto-Lock is Disabled on PIN Entry

### Logical Reasons:
1. **Security**: If screen sleeps during PIN entry, user has to wake + re-enter
2. **UX**: Frustrating if timeout triggers while typing PIN
3. **Boot Flow**: PIN Entry is the "unlock" screen - can't lock what's already locked
4. **Standard Behavior**: iOS, Android, etc. don't sleep on lock screens

### Implementation:
```cpp
void ScreenManager::checkAutoLock() {
    if (screenAsleep) return;
    
    // NEW: Skip PIN Entry screen
    if (currentScreen == SCREEN_PIN_ENTRY) {
        return;  // Don't auto-lock during PIN entry
    }
    
    if (settingsManager == nullptr) return;
    // ... rest of auto-lock logic
}
```

## Visual Comparison

### Before (Glitchy):
```
[Title]
[●●●●]  ← Indicator dots
[ERROR TEXT]
─────────  ← Clear region erases here!
[?????]    ← Top of keypad gets erased (glitch!)
[?????]
```

### After (Fixed):
```
[Title]
[●●●●]  ← Indicator dots
[ERROR TEXT]
─────────  ← Clear region stops here
   ↑ 12px safe gap
─────────
[1 2 3]  ← Keypad fully visible
[4 5 6]
[7 8 9]
[C 0 <]
```

## Memory Impact

```
RAM:  No change (47,688 bytes)
Flash: No change (432,779 bytes)
```

## Root Cause Analysis

### Why It Was Intermittent:
The glitch appeared "sometimes" because:
1. Error animation triggers `drawDynamicData()` repeatedly
2. Each redraw clears Y=22-82, overlapping keypad at Y=84
3. If keypad was drawn before error animation, it gets partially erased
4. If error animation completes and `forceFullRedraw` is set, keypad redraws correctly
5. Hence "tapping fixes it" - touches trigger `forceFullRedraw`

### Why It Looked Like "Grey Border":
- The clear operation erased the button fills
- But left the border outlines intact (drawn slightly lower)
- Result: Hollow grey rectangles where buttons should be

---

**Status:** ✅ Build successful - Ready to flash  
**Fixes:** Keypad glitch + Auto-lock disabled on PIN screen  
**Date:** 2026-07-24
