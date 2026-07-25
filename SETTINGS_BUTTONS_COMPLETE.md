# Settings Screen - Reset Buttons Added ✅

## Summary

The Settings screen now has **4 buttons** to access all touch and reset functions:

```
┌──────────────────────────────────────┐
│  SETTINGS                   < Back   │
│                                      │
│  ┌────────────┐                     │
│  │Touch Test  │  ← Green (Test)     │
│  └────────────┘                     │
│                                      │
│  ┌────────────┐                     │
│  │Calibrate   │  ← Blue (Action)    │
│  │Touch       │                      │
│  └────────────┘                     │
│                                      │
│  ┌────────────┐                     │
│  │Reset Cal   │  ← Red (Warning)    │
│  └────────────┘                     │
│                                      │
│  ┌────────────┐                     │
│  │Factory     │  ← Red (Danger)     │
│  │Reset       │                      │
│  └────────────┘                     │
└──────────────────────────────────────┘
```

## Button Details

### 1. Touch Test (Green)
- **Position:** (20, 50, 130×35)
- **Function:** Opens live touch testing screen
- **Navigation:** Settings → Touch Test

### 2. Calibrate Touch (Blue)
- **Position:** (20, 95, 130×35)
- **Function:** Opens calibration wizard
- **Navigation:** Settings → Calibration Wizard

### 3. Reset Cal (Red)
- **Position:** (20, 140, 130×35)
- **Function:** Resets touch calibration to defaults
- **Navigation:** Settings → Calibration Reset Screen

### 4. Factory Reset (Red)
- **Position:** (20, 185, 130×35)
- **Function:** Resets ALL settings and passwords
- **Navigation:** Settings → Factory Reset Screen

### 5. Back Button (Gray)
- **Position:** (220, 10, 90×30)
- **Function:** Returns to Home screen
- **Navigation:** Settings → Home

## Implementation Complete

### Files Modified/Created

#### Modified:
- `include/ScreenManager.h` - Added new screen states and method declarations
- `src/ScreenManager.cpp` - Added reset screen handlers and Settings UI

#### Created:
- `include/CalibrationResetScreen.h` - Reset calibration screen class
- `src/CalibrationResetScreen.cpp` - Implementation
- `include/FactoryResetScreen.h` - Factory reset screen class
- `src/FactoryResetScreen.cpp` - Implementation
- `include/SettingsManager.h` - Settings persistence (stub)
- `src/SettingsManager.cpp` - Implementation (stub)

### New Screen States Added

```cpp
enum ScreenState {
    SCREEN_HOME,
    SCREEN_SETTINGS,
    SCREEN_INFO,
    SCREEN_FAVORITES,
    SCREEN_ADDPASS,
    SCREEN_TOUCH_TEST,
    SCREEN_CALIBRATION,
    SCREEN_CALIBRATION_RESET,  // NEW
    SCREEN_FACTORY_RESET       // NEW
};
```

### Navigation Flow

```
Home
  ↓
Settings
  ├→ Touch Test (testing screen)
  ├→ Calibrate Touch (wizard)
  ├→ Reset Cal (confirmation screen)
  │   ├→ Cancel → Settings
  │   └→ Reset Calibration → Settings
  ├→ Factory Reset (confirmation screen)
  │   ├→ Cancel → Settings
  │   └→ Factory Reset → Settings
  └→ Back → Home
```

## Touch Hit Zones

All buttons on Settings screen:

```cpp
Touch Test:         [20-150, 50-85]
Calibrate Touch:    [20-150, 95-130]
Reset Calibration:  [20-150, 140-175]
Factory Reset:      [20-150, 185-220]
Back:               [220-310, 10-40]
```

## Color Coding

- **Green** (TFT_DARKGREEN/TFT_GREEN) - Safe testing/viewing
- **Blue** (TFT_NAVY/TFT_BLUE) - Normal actions
- **Red** (TFT_MAROON/TFT_RED) - Destructive/warning actions
- **Gray** (TFT_DARKGREY/TFT_LIGHTGREY) - Navigation

## Reset Screen Behavior

### Calibration Reset
1. User taps "Reset Cal" in Settings
2. Confirmation screen shows with warning
3. User taps "Cancel" → Returns to Settings
4. User taps "Reset Calibration" → Calls `settings->resetCalibration()` + `settings->save()` → Returns to Settings

### Factory Reset
1. User taps "Factory Reset" in Settings
2. Confirmation screen shows with strong warning
3. User taps "Cancel" → Returns to Settings
4. User taps "Factory Reset" → Calls `settings->factoryReset()` + `settings->save()` → Returns to Settings

## Testing Checklist

- [x] Settings screen shows all 4 buttons
- [x] Touch Test button opens touch test screen
- [x] Calibrate Touch button opens calibration wizard
- [x] Reset Cal button opens calibration reset screen
- [x] Factory Reset button opens factory reset screen
- [x] Back button returns to Home
- [x] All buttons have correct colors
- [x] Hit zones are accurate
- [x] Navigation works bidirectionally
- [x] Reset screens call correct SettingsManager methods

## Debug Output Example

```
[DEBUG_INPUT] X: 85, Y: 157, Type: TOUCH_DOWN, Screen: SETTINGS
[DEBUG_BUTTON] Settings screen - checking button hits
[DEBUG_BUTTON] HIT: 'Reset Calibration' on SETTINGS screen
[NAV] Transition (PUSH): SETTINGS -> CALIBRATION_RESET
[DEBUG_RENDER] Full Clear Triggered on CALIBRATION_RESET screen
CalibrationResetScreen created
CalibrationResetScreen initialized
[DEBUG_RENDER] Static UI complete on CALIBRATION_RESET screen
[DEBUG_RENDER] Dynamic data update on CALIBRATION_RESET screen
```

## Current Status

✅ **Settings UI Updated** - 4 buttons displayed correctly
✅ **New Screen States** - CALIBRATION_RESET and FACTORY_RESET added
✅ **Reset Screen Classes** - Fully implemented and integrated
✅ **SettingsManager** - Stub created (ready for implementation)
✅ **Navigation** - All transitions working
✅ **Touch Handling** - Delegated to reset screen classes
✅ **Rendering** - Delegated to reset screen classes

## Next Steps (Optional)

1. Implement actual EEPROM storage in SettingsManager
2. Add confirmation prompts (double-confirm for factory reset)
3. Add progress indicators during reset operations
4. Add success/failure toast messages
5. Implement password storage encryption
6. Add settings persistence across reboots

## Visual Reference

Settings Button Layout (320×240 screen):

```
┌─────────────────────────────────────────┐
│ 0,0          SETTINGS        < Back 320,0│
│                              (220,10)    │
│                                          │
│ (20,50)                                  │
│ ┌──────────────────┐  130×35            │
│ │   Touch Test     │  GREEN             │
│ └──────────────────┘                    │
│                                          │
│ (20,95)                                  │
│ ┌──────────────────┐  130×35            │
│ │ Calibrate Touch  │  BLUE              │
│ └──────────────────┘                    │
│                                          │
│ (20,140)                                 │
│ ┌──────────────────┐  130×35            │
│ │   Reset Cal      │  RED               │
│ └──────────────────┘                    │
│                                          │
│ (20,185)                                 │
│ ┌──────────────────┐  130×35            │
│ │  Factory Reset   │  RED               │
│ └──────────────────┘                    │
│                                          │
└─────────────────────────────────────────┘
0,240                               320,240
```

All buttons are left-aligned at X=20, stacked vertically with proper spacing.
Back button is in top-right corner for easy access without interfering with main buttons.
