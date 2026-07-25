# Ghost Click Debug - Diagnostic Version

## What Was Done

I've added **extensive debug logging** to trace exactly what happens when you tap the bottom-right corner of the Settings screen. This will help us identify where the ghost click is coming from.

---

## Debug Output Added

### 1. Touch Event Entry
Every touch on Settings screen now logs:
```
========================================
[SETTINGS_LIST] RAW TOUCH EVENT: X=300, Y=230
========================================
```

### 2. Back Button Check
```
[SETTINGS_LIST] Back button check: MISS
```
or
```
[SETTINGS_LIST] >>> BACK BUTTON HIT <<<
```

### 3. Row Detection Details
Verbose logging of the calculation process:
```
[ROW_DETECT] Input: X=300, Y=230
[ROW_DETECT] X within bounds (16-304)
[ROW_DETECT] Relative Y: 172 (Y=230 - START=58)
[ROW_DETECT] Calculated rowIndex: 3 (relY=172 / (H=42 + S=4))
[ROW_DETECT] Y position in row: 34 (must be < 42)
[ROW_DETECT] *** RESULT: ABOUT (enum=4) ***
```

### 4. Row Action Handling
```
[SETTINGS_LIST] Row: ABOUT
[SETTINGS_LIST] >>> About selected <<<
========================================
```

### 5. Rejection Cases
When touch is invalid:
```
[ROW_DETECT] *** REJECT: rowIndex=4 exceeds max (3) ***
```
or
```
[ROW_DETECT] *** REJECT: Touch in row spacing (yInRow=42 >= 42) ***
```

---

## Testing Instructions

### Step 1: Open Serial Monitor
1. Connect your device via USB
2. Open PlatformIO Serial Monitor (or Arduino Serial Monitor)
3. Set baud rate to **115200**

### Step 2: Navigate to Settings
1. Tap "Settings" button on HOME screen
2. Wait for Settings screen to appear

### Step 3: Tap Bottom-Right Corner
1. **Tap the exact spot** where the ghost click occurs (around X=300, Y=230)
2. **Immediately check the serial output**
3. Copy/paste the complete debug output (everything between the `===` lines)

### Step 4: Share the Output
Send me the serial monitor output that looks like this:

```
========================================
[SETTINGS_LIST] RAW TOUCH EVENT: X=???, Y=???
========================================
[SETTINGS_LIST] Back button check: ???
[ROW_DETECT] Input: X=???, Y=???
[ROW_DETECT] ...
[ROW_DETECT] *** RESULT: ??? ***
[SETTINGS_LIST] Row: ???
[SETTINGS_LIST] >>> ??? <<<
========================================
```

---

## What We're Looking For

### Scenario A: Row Detection Issue
If you see:
```
[ROW_DETECT] *** RESULT: ABOUT (enum=4) ***
[SETTINGS_LIST] >>> About selected <<<
```
**Then:** The row detection is incorrectly matching the corner to the About row.

### Scenario B: Back Button False Positive
If you see:
```
[SETTINGS_LIST] >>> BACK BUTTON HIT <<<
```
**Then:** The back button hit-test has wrong coordinates.

### Scenario C: Two-Tap Sleep Interference
If you see the device lock immediately after one tap:
```
[TWO_TAP_SLEEP] Double-tap detected - locking device
```
**Then:** The two-tap sleep gesture is triggering incorrectly.

### Scenario D: Touch Outside All Areas
If you see:
```
[ROW_DETECT] *** REJECT: ... ***
[SETTINGS_LIST] No row hit - touch ignored
```
**But the back action still happens**, then the issue is at a **higher level** (ScreenManager or TouchManager).

---

## Additional Tests

### Test 1: Different Corner Positions
Try tapping at different spots in that corner:
- Bottom-right: (300, 230)
- Bottom-middle: (160, 230)
- Right-middle: (300, 180)

Record the coordinates and results for each.

### Test 2: Test All Rows
Tap the center of each row and verify the debug output matches:
- Row 1 (Auto-Lock): Should show `RESULT: AUTO_LOCK`
- Row 2 (Two Tap Sleep): Should show `RESULT: TWO_TAP_SLEEP`
- Row 3 (Advanced): Should show `RESULT: ADVANCED`
- Row 4 (About): Should show `RESULT: ABOUT`

### Test 3: Test Row Spacing
Tap between rows (in the 4px spacing) and verify:
```
[ROW_DETECT] *** REJECT: Touch in row spacing ***
```

---

## What Happens Next

Once you provide the serial output, I can determine:

1. **Where the touch is being misinterpreted** (coordinates vs expected region)
2. **Which detection logic is failing** (back button, row detection, or higher level)
3. **The exact fix needed** (adjust bounds, fix calculation, or remove global handler)

---

## Current Suspicions

Based on the code review, the most likely culprits are:

1. **Two-Tap Sleep Gesture** - May be counting single taps as double-taps
2. **Row 4 (About) Hit-Box** - May extend too far into the corner
3. **Touch Calibration Issue** - Corner touches may be reporting wrong coordinates
4. **Hidden Global Handler** - Some code we haven't found yet

The debug output will reveal which one is the true cause!

---

## Build Info

**Version:** Debug build with extensive logging
**Flash:** 34.7% (362,763 bytes)
**RAM:** 47.3% (38,760 bytes)
**Status:** ✅ Uploaded to device

**Ready for diagnostic testing!**
