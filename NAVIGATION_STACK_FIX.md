# Navigation Stack Fix - Advanced Options Sub-Screens

## Problem Description

### Issue
When navigating from Advanced Options to any sub-screen (Touch Test, Calibration, Orientation, Reset Calibration, or Factory Reset), pressing the Back button incorrectly returned to the main Settings screen instead of returning to the Advanced Options menu.

### Incorrect Flow (Before Fix)
```
Settings → Advanced Options → Touch Test → [Back] → Settings ❌
Settings → Advanced Options → Calibration → [Back] → Settings ❌
Settings → Advanced Options → Orientation → [Back] → Settings ❌
Settings → Advanced Options → Reset Cal → [Back] → Settings ❌
Settings → Advanced Options → Factory Reset → [Back] → Settings ❌
```

### Correct Flow (After Fix)
```
Settings → Advanced Options → Touch Test → [Back] → Advanced Options ✓
Settings → Advanced Options → Calibration → [Back] → Advanced Options ✓
Settings → Advanced Options → Orientation → [Back] → Advanced Options ✓
Settings → Advanced Options → Reset Cal → [Back] → Advanced Options ✓
Settings → Advanced Options → Factory Reset → [Back] → Advanced Options ✓
```

---

## Root Cause

All sub-screens opened from Advanced Options were hardcoded to return to `SCREEN_SETTINGS` using `popScreen(SCREEN_SETTINGS)` instead of properly returning to the screen that launched them (`SCREEN_ADVANCED_OPTIONS`).

### Navigation Stack Concept

The screen stack works like this:
```
[SCREEN_HOME]                           ← Initial state
[SCREEN_HOME, SCREEN_SETTINGS]          ← Push Settings
[SCREEN_HOME, SCREEN_SETTINGS, SCREEN_ADVANCED_OPTIONS]  ← Push Advanced Options
[SCREEN_HOME, SCREEN_SETTINGS, SCREEN_ADVANCED_OPTIONS, SCREEN_TOUCH_TEST]  ← Push Touch Test

Pop should go to: SCREEN_ADVANCED_OPTIONS
Was going to: SCREEN_SETTINGS (incorrect target)
```

---

## Fixes Applied

### 1. Touch Test Screen
**Location:** `src/ScreenManager.cpp`, `handleTouchTestTouch()`

**Before:**
```cpp
DEBUG_LOG("Back button pressed - returning to Settings");
popScreen(SCREEN_SETTINGS);
```

**After:**
```cpp
DEBUG_LOG("Back button pressed - returning to Advanced Options");
popScreen(SCREEN_ADVANCED_OPTIONS);
```

---

### 2. Calibration Wizard Screen (2 locations)
**Location:** `src/ScreenManager.cpp`, `updateCalibrationData()`

#### Exit Handler
**Before:**
```cpp
// Return to Settings
popScreen(SCREEN_SETTINGS);
```

**After:**
```cpp
// Return to Advanced Options
popScreen(SCREEN_ADVANCED_OPTIONS);
```

#### Completion Handler
**Before:**
```cpp
// Return to Settings
popScreen(SCREEN_SETTINGS);
```

**After:**
```cpp
// Return to Advanced Options
popScreen(SCREEN_ADVANCED_OPTIONS);
```

---

### 3. Calibration Reset Screen
**Location:** `src/ScreenManager.cpp`, `updateCalibrationResetData()`

**Before:**
```cpp
if (calibrationResetScreen->needsToExit()) {
    popScreen(SCREEN_SETTINGS);  // Return to settings
    calibrationResetScreen->reset();
}
```

**After:**
```cpp
if (calibrationResetScreen->needsToExit()) {
    popScreen(SCREEN_ADVANCED_OPTIONS);  // Return to Advanced Options
    calibrationResetScreen->reset();
}
```

---

### 4. Factory Reset Screen
**Location:** `src/ScreenManager.cpp`, `updateFactoryResetData()`

**Before:**
```cpp
if (factoryResetScreen->needsToExit()) {
    popScreen(SCREEN_SETTINGS);  // Return to settings
    factoryResetScreen->reset();
}
```

**After:**
```cpp
if (factoryResetScreen->needsToExit()) {
    popScreen(SCREEN_ADVANCED_OPTIONS);  // Return to Advanced Options
    factoryResetScreen->reset();
}
```

---

### 5. Orientation Screen
**Location:** `src/ScreenManager.cpp`, `updateOrientationData()`

**Before:**
```cpp
if (orientationScreen->needsToExit()) {
    DEBUG_LOG("Exiting Orientation screen");
    popScreen(SCREEN_SETTINGS);
}
```

**After:**
```cpp
if (orientationScreen->needsToExit()) {
    DEBUG_LOG("Exiting Orientation screen");
    popScreen(SCREEN_ADVANCED_OPTIONS);
}
```

---

## Navigation Flow Verification

### Complete Navigation Hierarchy

```
Home Screen
└── Settings Screen
    └── Advanced Options Screen
        ├── Touch Test Screen         → Back → Advanced Options ✓
        ├── Calibration Wizard Screen → Back → Advanced Options ✓
        ├── Orientation Screen        → Back → Advanced Options ✓
        ├── Reset Calibration Screen  → Back → Advanced Options ✓
        └── Factory Reset Screen      → Back → Advanced Options ✓
```

### Stack States During Navigation

**Starting from Home:**
```
Stack: [HOME]
```

**Navigate to Settings:**
```
Action: Push SETTINGS
Stack: [HOME, SETTINGS]
```

**Navigate to Advanced Options:**
```
Action: Push ADVANCED_OPTIONS
Stack: [HOME, SETTINGS, ADVANCED_OPTIONS]
```

**Navigate to Touch Test:**
```
Action: Push TOUCH_TEST
Stack: [HOME, SETTINGS, ADVANCED_OPTIONS, TOUCH_TEST]
```

**Press Back Button:**
```
Action: Pop to ADVANCED_OPTIONS
Stack: [HOME, SETTINGS, ADVANCED_OPTIONS] ✓
```

**Press Back Again:**
```
Action: Pop to SETTINGS
Stack: [HOME, SETTINGS] ✓
```

**Press Back Again:**
```
Action: Pop to HOME
Stack: [HOME] ✓
```

---

## Implementation Details

### How popScreen() Works

The `popScreen(targetScreen)` function in ScreenManager:
1. Sets `currentScreen = targetScreen`
2. Sets `forceFullRedraw = true` (triggers full screen redraw)
3. Logs the navigation transition

**Key Point:** The target screen passed to `popScreen()` should be the screen you want to return to, which is the screen that called `pushScreen()` in the first place.

### Correct Pattern

```cpp
// In AdvancedOptionsScreen touch handler
if (userSelectedTouchTest) {
    manager->pushScreen(SCREEN_TOUCH_TEST);  // Stack: [..., ADV_OPTIONS, TOUCH_TEST]
}

// In TouchTestScreen back handler
if (userPressedBack) {
    manager->popScreen(SCREEN_ADVANCED_OPTIONS);  // Stack: [..., ADV_OPTIONS]
}
```

---

## Testing Checklist

### Manual Navigation Tests

**Test 1: Touch Test Navigation**
- [ ] Home → Settings → Advanced Options → Touch Test
- [ ] Press Back in Touch Test
- [ ] ✓ Should return to Advanced Options (not Settings)
- [ ] Press Back in Advanced Options
- [ ] ✓ Should return to Settings

**Test 2: Calibration Navigation**
- [ ] Home → Settings → Advanced Options → Touch Calibration
- [ ] Press Cancel/Exit in Calibration
- [ ] ✓ Should return to Advanced Options (not Settings)
- [ ] Complete calibration
- [ ] ✓ Should return to Advanced Options (not Settings)

**Test 3: Orientation Navigation**
- [ ] Home → Settings → Advanced Options → Screen Orientation
- [ ] Press Back in Orientation
- [ ] ✓ Should return to Advanced Options (not Settings)

**Test 4: Reset Calibration Navigation**
- [ ] Home → Settings → Advanced Options → Reset Calibration
- [ ] Press Back/Cancel
- [ ] ✓ Should return to Advanced Options (not Settings)

**Test 5: Factory Reset Navigation**
- [ ] Home → Settings → Advanced Options → Factory Reset
- [ ] Press Back/Cancel
- [ ] ✓ Should return to Advanced Options (not Settings)

### Complete Flow Test
- [ ] Start at Home
- [ ] Navigate to Settings
- [ ] Navigate to Advanced Options
- [ ] Open each sub-screen one at a time
- [ ] Press Back from each sub-screen
- [ ] ✓ All should return to Advanced Options
- [ ] From Advanced Options, press Back
- [ ] ✓ Should return to Settings
- [ ] From Settings, press Back
- [ ] ✓ Should return to Home

---

## Debug Output

### Expected Log Sequence

**Navigating to Touch Test:**
```
[NAV] Transition (PUSH): ADVANCED_OPTIONS -> TOUCH_TEST
```

**Pressing Back:**
```
[DEBUG_BUTTON] HIT: 'Back' on TOUCH_TEST screen
Back button pressed - returning to Advanced Options
[NAV] Transition (POP): TOUCH_TEST -> ADVANCED_OPTIONS
```

**Pressing Back Again:**
```
AdvancedOptions: Back button pressed
Exiting Advanced Options screen
[NAV] Transition (POP): ADVANCED_OPTIONS -> SETTINGS
```

---

## Files Modified

**File:** `src/ScreenManager.cpp`

**Functions Updated:**
1. `handleTouchTestTouch()` - Touch Test back button
2. `updateCalibrationData()` - Calibration exit and complete handlers
3. `updateCalibrationResetData()` - Reset Calibration exit handler
4. `updateFactoryResetData()` - Factory Reset exit handler
5. `updateOrientationData()` - Orientation exit handler

**Total Changes:** 6 popScreen() calls updated

---

## Impact Assessment

### Benefits
✅ **Correct Navigation:** Users can now properly navigate back to Advanced Options
✅ **Intuitive UX:** Back button behavior matches user expectations
✅ **Stack Integrity:** Screen stack remains consistent and predictable
✅ **No Side Effects:** Only affects navigation targets, no logic changes

### No Breaking Changes
- All screens still function identically
- Only navigation targets were changed
- No API or interface modifications
- Existing screen logic untouched

---

## Related Documentation

- `ADVANCED_OPTIONS_IMPLEMENTATION.md` - Advanced Options menu architecture
- `INSTANT_SCREEN_CHANGE_ARCHITECTURE.md` - Screen management patterns
- `IMPLEMENTATION_COMPLETE_SUMMARY.md` - Overall project status

---

**Fix Applied:** Current session
**Issue:** Navigation stack incorrect for Advanced Options sub-screens
**Solution:** Updated all popScreen() calls to return to ADVANCED_OPTIONS
**Status:** ✅ Complete - Ready for Testing
