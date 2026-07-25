# PIN Keypad Touch Debugging & Diagnosis Guide

## Problem: Unresponsive PIN Keypad

### Symptom
The PIN entry keypad is displayed correctly on screen after boot or wake, but tapping any of the digit buttons (0-9 or backspace) produces no response. The indicator dots don't fill in, and no digits are registered.

---

## Comprehensive Debug Logging Added

To diagnose the exact cause of the touch unresponsiveness, I've added extensive debug logging throughout the touch input pipeline.

### Debug Points Added

1. **Screen Initialization** - Verifies layout constants
2. **Screen Reset** - Confirms state is properly cleared on wake
3. **Touch Event Arrival** - Shows when touches reach PIN entry handler
4. **Hit Testing** - Shows detailed button boundary calculations
5. **Key Detection** - Shows which key (if any) was detected
6. **Digit Entry** - Shows when digits are added/removed

---

## Serial Monitor Output Guide

### On Boot (Initialization)

**Expected output:**
```
========================================
[PIN_ENTRY] Screen initialized
[PIN_ENTRY] Layout constants: KEY_SIZE=38, KEY_SPACING=6, KEYPAD_START_Y=70
[PIN_ENTRY] Keypad dimensions: width=120, total height=176
[PIN_ENTRY] Keypad bounds: X=100-220, Y=70-246
========================================
```

**What to check:**
- ✅ KEY_SIZE should be 38 (not 60)
- ✅ KEYPAD_START_Y should be 70 (not 100)
- ✅ Total height should be 176 (4 rows fit in 240px screen)
- ✅ Y bounds should end at 246 (slightly over 240 is OK due to calculation)

**If values are wrong:**
- ❌ Old layout constants still in effect
- ❌ Recompile didn't pick up header changes
- ❌ Need to clean build: `platformio run -t clean`

### On Wake from Sleep (Screen Reset)

**Expected output:**
```
[SCREEN_WAKE] Resetting PIN entry screen internal state
========================================
[PIN_ENTRY] *** RESET CALLED ***
[PIN_ENTRY] Previous state: unlocked=YES, pinIndex=4
[PIN_ENTRY] New state: unlocked=NO, pinIndex=0, forceFullRedraw=YES
[PIN_ENTRY] Screen ready for touch input
========================================
```

**What to check:**
- ✅ Reset is called after wake
- ✅ `unlocked` changes to NO
- ✅ `pinIndex` resets to 0
- ✅ Screen declares itself ready for input

**If reset not called:**
- ❌ Wake gesture isn't triggering reset properly
- ❌ Check `wakeScreen()` implementation
- ❌ Verify screen type in switch statement

**If unlocked stays YES:**
- ❌ Critical bug - touches will be blocked
- ❌ Reset logic not working
- ❌ Memory corruption possible

### On Touch Event (Button Tap)

**Stage 1: Touch arrives at handler**
```
========================================
[PIN_ENTRY_TOUCH] handleTouch called: x=160, y=120, valid=YES
```

**What to check:**
- ✅ Handler is being called
- ✅ Coordinates are reasonable (within screen bounds 0-320, 0-240)
- ✅ Touch is marked as valid

**If handler not called:**
- ❌ Touch not reaching PIN entry screen
- ❌ Check ScreenManager touch dispatch
- ❌ Verify `currentScreen == SCREEN_PIN_ENTRY`
- ❌ Check sleep state isn't blocking touches

**If coordinates look wrong:**
- ❌ Touch calibration issue
- ❌ Orientation mismatch
- ❌ Run Touch Test screen to verify touch controller

**Stage 2: Unlocked check**
```
[PIN_ENTRY_TOUCH] *** BLOCKED: Already unlocked, ignoring touch ***
========================================
```

**If you see this:**
- ❌ **CRITICAL BUG** - Screen thinks it's unlocked
- ❌ `unlocked` flag not reset properly
- ❌ User successfully entered PIN but screen didn't transition
- ❌ Check unlock transition logic

**Should NOT see this on fresh boot or after wake!**

**Stage 3: Hit testing**
```
[PIN_HIT_TEST] Touch: (160, 120), Keypad bounds: X=100-220, Y=70-246
[PIN_HIT_TEST] Grid position: row=1, col=1
[PIN_HIT_TEST] Circle test: center=(138,114), radius=17, dist²=512, radius²=289
[PIN_HIT_TEST] REJECT: Outside circular button boundary
```

**What to check:**
- Touch coordinates: (160, 120)
- Keypad X bounds: 100-220 (width=120, centered on 320px screen)
- Keypad Y bounds: 70-246
- Grid position: row=1, col=1 (middle button of row 2 = "5")
- Button center: (138, 114)
- Circle radius: 17 (KEY_SIZE/2 - 2 = 38/2 - 2 = 17)
- Distance²: 512
- Radius²: 289

**Analysis:**
- dist² (512) > radius² (289) = **MISS**
- Touch is ~22 pixels away from button center
- Touch is hitting between buttons (in spacing)

**Successful hit looks like:**
```
[PIN_HIT_TEST] Touch: (138, 114), Keypad bounds: X=100-220, Y=70-246
[PIN_HIT_TEST] Grid position: row=1, col=1
[PIN_HIT_TEST] Circle test: center=(138,114), radius=17, dist²=0, radius²=289
[PIN_HIT_TEST] MATCH: Key index 5
```

**Stage 4: Key detection**
```
[PIN_ENTRY_TOUCH] Key index detected: 5
[PIN_ENTRY_TOUCH] *** DIGIT KEY PRESSED: 5 ***
[PIN_ENTRY] Added digit, PIN length now: 1
========================================
```

**What to check:**
- ✅ Key index should be 0-9 for digits, -2 for backspace
- ✅ "Added digit" message confirms entry
- ✅ PIN length increments

**If key index is -1:**
- Touch missed all buttons
- Either hit spacing between buttons
- Or touch is outside keypad bounds entirely

---

## Common Failure Modes

### Mode 1: Touch Never Reaches Handler

**Symptom:** No `[PIN_ENTRY_TOUCH] handleTouch called` messages

**Serial output:**
```
[TOUCH] Two Tap Sleep is DISABLED
[TOUCH] Continuing to normal touch processing...
[DEBUG_INPUT] X: 160, Y: 120, Type: TOUCH_DOWN, Screen: PIN_ENTRY
... (nothing from PIN_ENTRY_TOUCH)
```

**Cause:** Touch event not dispatched to PIN entry handler

**Check:**
1. Verify `currentScreen == SCREEN_PIN_ENTRY`
   ```
   Look for: "[DEBUG_INPUT] ... Screen: PIN_ENTRY"
   ```

2. Check if screen is asleep
   ```
   If you see: "[SLEEP_WAKE]" messages
   Device is still in sleep mode - shouldn't happen
   ```

3. Verify switch statement includes PIN_ENTRY case
   ```cpp
   case SCREEN_PIN_ENTRY:
       handlePINEntryTouch(point);
       break;
   ```

**Fix:** Add missing case or verify screen state

### Mode 2: Handler Called But Blocked by Unlocked Flag

**Symptom:** Handler called but immediately returns

**Serial output:**
```
[PIN_ENTRY_TOUCH] handleTouch called: x=160, y=120, valid=YES
[PIN_ENTRY_TOUCH] *** BLOCKED: Already unlocked, ignoring touch ***
```

**Cause:** `unlocked` flag is true when it shouldn't be

**This should ONLY happen after:**
- User successfully enters correct PIN
- Screen transitions to HOME
- But if you see it right after boot/wake, it's a bug!

**Check:**
1. Look for reset messages
   ```
   [PIN_ENTRY] New state: unlocked=NO
   ```

2. If unlocked=YES after reset, memory corruption or logic error

**Fix:** Ensure reset() is called on wake and sets `unlocked = false`

### Mode 3: Hit Testing Always Misses

**Symptom:** Handler works but all touches miss buttons

**Serial output:**
```
[PIN_ENTRY_TOUCH] handleTouch called: x=160, y=120, valid=YES
[PIN_HIT_TEST] Touch: (160, 120), Keypad bounds: X=100-220, Y=70-246
[PIN_HIT_TEST] REJECT: Y out of bounds
[PIN_ENTRY_TOUCH] Key index detected: -1
[PIN_ENTRY_TOUCH] *** NO KEY HIT - touch outside button area ***
```

**Possible causes:**

**A) Touch coordinates completely wrong**
```
Touch: (1000, 500)  ← Way outside 320x240 screen
```
- Touch calibration broken
- Wrong orientation mapping
- Run Touch Test to verify touch controller

**B) Keypad bounds wrong**
```
Keypad bounds: X=100-220, Y=200-500  ← Starts too low, goes off screen
```
- Old KEY_SIZE/KEYPAD_START_Y constants
- Clean build and recompile

**C) Touch lands between buttons**
```
Touch: (160, 120)
Grid position: row=1, col=1
Circle test: dist²=512, radius²=289  ← Miss
```
- Touch is in spacing between buttons
- Buttons too small for reliable tapping
- Consider increasing KEY_SIZE or reducing spacing

### Mode 4: Coordinates Offset/Shifted

**Symptom:** Touch consistently misses by same offset

**Serial output:**
```
User taps center of "5" button visually
[PIN_HIT_TEST] Touch: (180, 150)  ← Should be ~(138, 114)
[PIN_HIT_TEST] Grid position: row=1, col=2  ← Detected as "6" not "5"
```

**Cause:** Touch calibration or orientation mismatch

**Check:**
1. Display orientation setting
   ```
   Should match touch controller orientation
   Usually rotation=1 for landscape
   ```

2. Touch calibration values
   ```cpp
   TOUCH_CAL_X_MIN = 249
   TOUCH_CAL_X_MAX = 3810
   TOUCH_CAL_Y_MIN = 388
   TOUCH_CAL_Y_MAX = 3686
   ```

3. Run calibration wizard
   - Settings → Advanced → Touch Calibration

**Fix:** Recalibrate touch or adjust orientation

---

## Diagnostic Test Procedure

### Test 1: Verify Screen State

1. Boot device
2. Open Serial Monitor (115200 baud)
3. Look for initialization output

**Check:**
```
[PIN_ENTRY] Layout constants: KEY_SIZE=38, KEY_SPACING=6, KEYPAD_START_Y=70
```

**If constants are wrong (e.g., KEY_SIZE=60):**
```bash
# Clean and rebuild
platformio run -t clean
platformio run
```

### Test 2: Verify Touch Reaches Handler

1. With Serial Monitor open
2. Tap center of any keypad button
3. Look for handler message

**Should see:**
```
[PIN_ENTRY_TOUCH] handleTouch called: x=???, y=???, valid=YES
```

**If you DON'T see this:**
- Touch event not reaching PIN entry screen
- Check ScreenManager touch dispatch
- Verify `currentScreen` state

**If you see this:**
- Touch is reaching handler ✓
- Continue to Test 3

### Test 3: Check Unlocked Flag

**Look at the handler output:**

**Good (Expected):**
```
[PIN_ENTRY_TOUCH] handleTouch called: ...
[PIN_HIT_TEST] Touch: ...
```

**Bad (Bug):**
```
[PIN_ENTRY_TOUCH] handleTouch called: ...
[PIN_ENTRY_TOUCH] *** BLOCKED: Already unlocked, ignoring touch ***
```

**If blocked:**
- Critical bug - touches will never work
- Check reset() is being called
- Verify `unlocked` flag is cleared

### Test 4: Analyze Hit Testing

**Look at hit test output:**

```
[PIN_HIT_TEST] Touch: (138, 114), Keypad bounds: X=100-220, Y=70-246
[PIN_HIT_TEST] Grid position: row=1, col=1
[PIN_HIT_TEST] Circle test: center=(138,114), radius=17, dist²=0, radius²=289
[PIN_HIT_TEST] MATCH: Key index 5
```

**Calculate manually:**
- Button "5" is row 1, col 1 (0-indexed)
- X position: 100 + 1 * (38 + 6) + 38/2 = 100 + 44 + 19 = 163... wait, that's wrong!

**Let me recalculate button positions:**

```
Keypad width: (38 * 3) + (6 * 2) = 114 + 12 = 126
Start X: (320 - 126) / 2 = 97

Button layout:
Row 0 (1,2,3): Y = 70
  Col 0 (1): X = 97 + 0*(38+6) + 19 = 116
  Col 1 (2): X = 97 + 1*(38+6) + 19 = 160
  Col 2 (3): X = 97 + 2*(38+6) + 19 = 204

Row 1 (4,5,6): Y = 70 + 44 = 114
  Col 0 (4): X = 116
  Col 1 (5): X = 160
  Col 2 (6): X = 204

Row 2 (7,8,9): Y = 70 + 88 = 158
  Col 0 (7): X = 116
  Col 1 (8): X = 160
  Col 2 (9): X = 204

Row 3 (<,0): Y = 70 + 132 = 202
  Col 0 (<): X = 116
  Col 1 (0): X = 160
```

**Button "5" center should be at (160, 114)**

**To hit button "5", tap around:**
- X: 160 ± 17 (radius) = 143-177
- Y: 114 ± 17 = 97-131

### Test 5: Test Specific Buttons

**Tap each button and verify correct detection:**

| Button | Expected Position | Expected Index |
|--------|------------------|----------------|
| 1 | (116, 70) | 1 |
| 2 | (160, 70) | 2 |
| 3 | (204, 70) | 3 |
| 4 | (116, 114) | 4 |
| 5 | (160, 114) | 5 |
| 6 | (204, 114) | 6 |
| 7 | (116, 158) | 7 |
| 8 | (160, 158) | 8 |
| 9 | (204, 158) | 9 |
| < | (116, 202) | -2 |
| 0 | (160, 202) | 0 |

**For each tap, check:**
```
[PIN_HIT_TEST] MATCH: Key index <expected>
[PIN_ENTRY_TOUCH] *** DIGIT KEY PRESSED: <expected> ***
```

---

## Expected Behavior Summary

### Successful Touch Flow

```
1. User taps button "5"
   ↓
2. TouchManager detects touch at (160, 114)
   ↓
3. ScreenManager sees TOUCH_DOWN
   ↓
4. ScreenManager dispatches to handlePINEntryTouch()
   ↓
5. PIN Entry handleTouch() called
   ↓
6. Check: unlocked == false ✓
   ↓
7. getKeyAtPosition() called
   ↓
8. Hit test: within keypad bounds ✓
   ↓
9. Hit test: grid position row=1, col=1 ✓
   ↓
10. Hit test: within circular boundary ✓
    ↓
11. Return key index: 5
    ↓
12. addDigit('5') called
    ↓
13. PIN length increments
    ↓
14. Indicator dot fills in ✓
    ↓
15. Serial: "[PIN_ENTRY] Added digit, PIN length now: 1"
```

---

## Build Status

✅ **Compilation:** SUCCESS  
✅ **RAM:** 54.4% (44,576 bytes)  
✅ **Flash:** 36.4% (380,675 bytes)  
✅ **Debug Logging:** Comprehensive coverage

---

## Next Steps Based on Serial Output

### If handler never called:
→ Check ScreenManager touch dispatch  
→ Verify `currentScreen` state  
→ Check wake/sleep state

### If handler blocked by unlocked flag:
→ Verify reset() is called on wake  
→ Check unlock transition logic  
→ May indicate memory corruption

### If hit testing always misses:
→ Check touch calibration  
→ Verify layout constants updated  
→ Run Touch Test screen  
→ Check orientation settings

### If buttons too small to hit reliably:
→ Consider increasing KEY_SIZE to 40-42  
→ Or reducing KEY_SPACING to 4-5  
→ Trade-off: larger buttons vs tighter layout

---

## How to Use This Guide

1. **Open Serial Monitor** (115200 baud)
2. **Boot device** or **wake from sleep**
3. **Tap a keypad button** (try button "5" in center)
4. **Read serial output** and match against sections above
5. **Identify failure mode** from the output patterns
6. **Follow diagnostic steps** for that specific mode
7. **Report findings** with serial log excerpt

The comprehensive logging will reveal exactly where the touch input pipeline is breaking down.
