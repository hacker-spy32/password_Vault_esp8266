# Two Tap Sleep Debug - Comprehensive Logging

## Diagnostic Build Uploaded

I've uploaded a version with **extensive debug logging** to diagnose why single taps might be triggering sleep.

---

## What to Test

### Test 1: Verify Feature State
1. Open Serial Monitor (115200 baud)
2. Navigate to Settings
3. Look at the "Manual Sleep" toggle
4. **Take a photo** of the toggle state (ON or OFF)

### Test 2: Single Tap Test
1. With Serial Monitor open
2. Ensure "Manual Sleep" is **ENABLED** (toggle ON/Brass)
3. Go to HOME screen
4. **Tap once** on an empty area
5. **Copy the entire serial output** from that tap

### Test 3: Double Tap Test  
1. Still on HOME screen
2. **Tap twice quickly** (< 400ms between taps)
3. **Copy the entire serial output**

---

## Expected Serial Output

### When Feature is DISABLED
```
[TOUCH] Two Tap Sleep is DISABLED - skipping manual sleep check
[DEBUG_INPUT] X: 160, Y: 120, Type: TOUCH_DOWN, Screen: HOME
```

### When Feature is ENABLED - Single Tap
```
[TOUCH] Two Tap Sleep is ENABLED - checking for manual sleep gesture
========================================
[MANUAL_SLEEP] ENTRY: manualTapCount=0, lastManualTapTime=0, currentTime=5234, elapsed=5234ms
[MANUAL_SLEEP] START NEW SEQUENCE: Tap 1/2 (window: 400ms)
[MANUAL_SLEEP] EXIT: manualTapCount=1, lastManualTapTime=5234
========================================
[TOUCH] Continuing to normal touch processing...
[DEBUG_INPUT] X: 160, Y: 120, Type: TOUCH_DOWN, Screen: HOME
```

### When Feature is ENABLED - Double Tap (Within 400ms)
**First Tap:**
```
[TOUCH] Two Tap Sleep is ENABLED - checking for manual sleep gesture
========================================
[MANUAL_SLEEP] ENTRY: manualTapCount=0, lastManualTapTime=0, currentTime=10500, elapsed=10500ms
[MANUAL_SLEEP] START NEW SEQUENCE: Tap 1/2 (window: 400ms)
[MANUAL_SLEEP] EXIT: manualTapCount=1, lastManualTapTime=10500
========================================
[TOUCH] Continuing to normal touch processing...
```

**Second Tap (200ms later):**
```
[TOUCH] Two Tap Sleep is ENABLED - checking for manual sleep gesture
========================================
[MANUAL_SLEEP] ENTRY: manualTapCount=1, lastManualTapTime=10500, currentTime=10700, elapsed=200ms
[MANUAL_SLEEP] WITHIN WINDOW: Tap 2/2 (elapsed: 200ms, window: 400ms)
[MANUAL_SLEEP] *** DOUBLE-TAP CONFIRMED - ENTERING SLEEP MODE ***
[SCREEN_SLEEP] Entering sleep mode
[SCREEN_SLEEP] Screen is now asleep (black)
========================================
```

### When Feature is ENABLED - Two Taps Too Slow (> 400ms)
**First Tap:**
```
[MANUAL_SLEEP] START NEW SEQUENCE: Tap 1/2 (window: 400ms)
[MANUAL_SLEEP] EXIT: manualTapCount=1, lastManualTapTime=15000
```

**Second Tap (500ms later):**
```
[MANUAL_SLEEP] ENTRY: manualTapCount=1, lastManualTapTime=15000, currentTime=15500, elapsed=500ms
[MANUAL_SLEEP] TIMEOUT: Elapsed 500ms > window 400ms - resetting counter
[MANUAL_SLEEP] START NEW SEQUENCE: Tap 1/2 (window: 400ms)
[MANUAL_SLEEP] EXIT: manualTapCount=1, lastManualTapTime=15500
```

---

## What We're Looking For

### Scenario A: Feature Actually Disabled
If you see:
```
[TOUCH] Two Tap Sleep is DISABLED
```
But the toggle **appears ON** in settings → **Settings not persisting correctly**

### Scenario B: Counter Not Resetting
If single tap shows:
```
[MANUAL_SLEEP] ENTRY: manualTapCount=1, lastManualTapTime=...
[MANUAL_SLEEP] WITHIN WINDOW: Tap 2/2
[MANUAL_SLEEP] *** DOUBLE-TAP CONFIRMED ***
```
→ **Counter not being reset properly**

### Scenario C: Immediate Sleep (No Debug)
If tap causes immediate sleep without seeing:
```
[MANUAL_SLEEP] *** DOUBLE-TAP CONFIRMED ***
```
→ **Different code path triggering sleep**

### Scenario D: Wrong Elapsed Time
If you see elapsed time like:
```
elapsed=50ms (but you tapped 2 seconds apart)
```
→ **Timer not updating correctly**

---

## Additional Info to Collect

1. **Toggle State**: Is "Manual Sleep" toggle ON or OFF?
2. **Visual Feedback**: Does toggle show Brass (ON) or Gray (OFF)?
3. **Tap Timing**: How fast are you tapping (try slow vs fast)?
4. **Screen Location**: Where on screen are you tapping?
5. **Serial Output**: Complete output between the `===` lines

---

## Quick Diagnostics

### If Screen Sleeps on Single Tap:

**Check 1:** Is the serial output showing?
- YES → Feature is triggering incorrectly
- NO → Something else is causing sleep (auto-lock?)

**Check 2:** What does manualTapCount show?
- manualTapCount=2 → Counter not being reset
- manualTapCount=1 → Shouldn't trigger sleep

**Check 3:** What does elapsed time show?
- < 400ms → Within window (might be second tap)
- > 400ms → Outside window (should reset)

---

## Recovery Steps

If device keeps sleeping unexpectedly:

1. **Disable Feature:**
   - Wake device (double-tap)
   - Navigate to Settings
   - Toggle "Manual Sleep" OFF

2. **Check Auto-Lock:**
   - Settings → Auto-Lock timeout
   - Set to "Never" temporarily
   - Test if sleep still occurs

3. **Reboot Device:**
   - Power cycle the device
   - Check if settings persist

---

## Send Me This Info

Please provide:

1. **Photo** of Settings screen showing toggle state
2. **Complete serial output** from:
   - Single tap test
   - Double tap test
3. **Your observation**: What exactly happens when you tap once?
4. **Timing**: Are you tapping intentionally fast or normal speed?

This debug output will show us exactly what's happening at the code level!

---

## Build Info

**Version:** Debug build v2 with comprehensive manual sleep logging
**Flash:** 34.9% (364,315 bytes)
**RAM:** 48.4% (39,656 bytes)
**Status:** ✅ Uploaded successfully

**Diagnostic Features:**
- Entry/exit state logging
- Elapsed time calculation
- Window comparison
- Counter tracking
- Feature enable/disable detection
