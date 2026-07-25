# Screen Sleep Implementation - True Display Off

## Overview
Implemented proper screen sleep functionality where the display turns completely black (sleep mode) instead of returning to HOME screen, with double-tap wake capability.

---

## 1. Sleep State Machine

### States
```
AWAKE → [Auto-Lock Timeout] → ASLEEP
ASLEEP → [Double-Tap Wake] → AWAKE
AWAKE → [Manual Sleep (optional)] → ASLEEP
```

### State Variables
```cpp
bool screenAsleep;              // Current sleep state
ScreenState screenBeforeSleep;  // Screen to restore on wake
unsigned long lastWakeTapTime;  // For double-tap detection
uint8_t wakeTapCount;           // Tap counter
```

---

## 2. Auto-Lock Sleep Behavior

### Previous Behavior (❌ Incorrect)
```cpp
// OLD: Returned to HOME screen
if (timeout) {
    currentScreen = SCREEN_HOME;
    forceFullRedraw = true;
}
```

### New Behavior (✅ Correct)
```cpp
// NEW: Enters true sleep mode
if (timeout) {
    sleepScreen();  // Black screen, backlight off
}
```

### Implementation
```cpp
void ScreenManager::sleepScreen() {
    // Store current screen for restoration
    screenBeforeSleep = currentScreen;
    
    // Turn display black
    tft->fillScreen(TFT_BLACK);
    
    // Set sleep flag
    screenAsleep = true;
    
    Serial.println("[SCREEN_SLEEP] Entered sleep mode");
}
```

---

## 3. Double-Tap Wake Gesture

### Specifications
- **Tap Window**: 400ms (0.4 seconds)
- **Tap Count**: 2 taps required
- **Active When**: Screen is asleep
- **Action**: Wake screen and restore previous view

### Implementation Flow
```
Screen Asleep
    ↓
User Taps → [WAKE_GESTURE] Tap 1/2
    ↓
User Taps Again (within 400ms) → [WAKE_GESTURE] Tap 2/2
    ↓
Double-Tap Detected → wakeScreen()
    ↓
Screen Awake (restored to previous screen)
```

### Code Implementation
```cpp
void ScreenManager::handleWakeGesture() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastWakeTapTime <= WAKE_TAP_WINDOW_MS) {
        wakeTapCount++;
        
        if (wakeTapCount >= 2) {
            wakeScreen();  // Wake up!
            wakeTapCount = 0;
            return;
        }
    } else {
        wakeTapCount = 1;  // Reset if outside window
    }
    
    lastWakeTapTime = currentTime;
}
```

### Wake Screen Process
```cpp
void ScreenManager::wakeScreen() {
    // Restore previous screen
    currentScreen = screenBeforeSleep;
    forceFullRedraw = true;
    screenIsDirty = true;
    screenAsleep = false;
    
    // Reset auto-lock timer
    resetAutoLockTimer();
    
    Serial.println("[SCREEN_WAKE] Screen awake");
}
```

---

## 4. Manual Sleep Feature

### Description
**Renamed from "Two Tap Sleep" to "Manual Sleep"**

Optional feature that allows users to manually put the device to sleep by double-tapping anywhere on the screen (when awake).

### Settings UI
- **Row 2**: "Manual Sleep"
- **Control**: Toggle switch (OFF by default)
- **Status**: Brass when ON, Gray when OFF

### Behavior When Enabled
```
User on Any Screen
    ↓
Double-Tap (within 500ms)
    ↓
[MANUAL_SLEEP] Detected → sleepScreen()
    ↓
Screen Immediately Asleep
```

### Implementation
```cpp
// In ScreenManager::update()
if (settingsManager->getTwoTapSleepEnabled()) {
    // Track double-tap
    if (currentTime - lastManualTapTime <= 500) {
        manualTapCount++;
        if (manualTapCount >= 2) {
            sleepScreen();  // Manual sleep
            return;  // Don't process touch further
        }
    } else {
        manualTapCount = 1;
    }
    lastManualTapTime = currentTime;
}
```

---

## 5. Touch Event Routing

### Asleep State Handling
```cpp
if (touch->getState() == TOUCH_DOWN && point.valid) {
    if (screenAsleep) {
        handleWakeGesture();  // Only handle wake gestures
        return;  // Don't process normal UI touches
    }
    
    // Normal awake touch handling...
}
```

### Render Prevention While Asleep
```cpp
void ScreenManager::render() {
    if (screenAsleep) {
        return;  // Don't render anything while asleep
    }
    
    // Normal rendering...
}
```

---

## 6. Backlight Control

### Current Implementation
The current implementation fills the screen with black:
```cpp
tft->fillScreen(TFT_BLACK);
```

### Hardware Backlight Control (Optional Enhancement)
For true backlight off (hardware-dependent):

**ESP8266 with PWM Backlight:**
```cpp
// In sleepScreen()
analogWrite(TFT_BL_PIN, 0);  // Turn off backlight

// In wakeScreen()
analogWrite(TFT_BL_PIN, 255);  // Turn on backlight
```

**ESP32 with LEDC:**
```cpp
// In sleepScreen()
ledcWrite(0, 0);  // Channel 0, brightness 0

// In wakeScreen()
ledcWrite(0, 255);  // Channel 0, brightness 255
```

**Note**: This requires:
1. Defining `TFT_BL_PIN` in your hardware config
2. Setting up PWM/LEDC channel in `initHardware()`

---

## 7. Serial Debug Output

### Auto-Lock Sleep
```
[AUTO_LOCK] Timeout triggered after 15 seconds (limit: 15)
[SCREEN_SLEEP] Entering sleep mode
[SCREEN_SLEEP] Screen is now asleep (black)
[AUTO_LOCK] Device entered sleep mode
```

### Wake Gesture
```
[WAKE_GESTURE] Tap 1/2 (window: 400ms)
[WAKE_GESTURE] Tap 2/2 (window: 400ms)
[WAKE_GESTURE] Double-tap detected - waking device
[SCREEN_WAKE] Waking up screen
[SCREEN_WAKE] Screen awake, restored to PASSWORDS
```

### Manual Sleep
```
[MANUAL_SLEEP] Double-tap detected - entering sleep
[SCREEN_SLEEP] Entering sleep mode
[SCREEN_SLEEP] Screen is now asleep (black)
```

---

## 8. User Experience Flow

### Scenario 1: Auto-Lock Sleep
```
User on Password Detail Screen
    ↓
No touch for 15 seconds
    ↓
[AUTO_LOCK] Timeout
    ↓
Screen turns black (sleep)
    ↓
User double-taps
    ↓
Screen wakes to Password Detail (where they left off)
```

### Scenario 2: Manual Sleep
```
User on Settings Screen
    ↓
User enables "Manual Sleep"
    ↓
User goes to Home Screen
    ↓
User double-taps quickly
    ↓
Screen turns black (sleep)
    ↓
User double-taps to wake
    ↓
Screen wakes to Home Screen
```

### Scenario 3: Single Tap During Sleep
```
Screen is asleep (black)
    ↓
User taps once
    ↓
[WAKE_GESTURE] Tap 1/2
    ↓
Nothing happens (waiting for second tap)
    ↓
500ms passes
    ↓
Tap counter resets
```

---

## 9. Settings Integration

### Auto-Lock Setting
- **Location**: Settings → Auto-Lock row
- **Options**: 5s, 10s, 15s, 20s, 30s, Never
- **Default**: 15s
- **Behavior**: After timeout, screen enters sleep mode

### Manual Sleep Setting
- **Location**: Settings → Manual Sleep row
- **Control**: Toggle switch
- **Default**: OFF
- **Behavior**: When ON, double-tap anywhere to immediately sleep

### Persistence
Both settings are saved to LittleFS and persist across reboots:
```
[SETTINGS] Saved 64 bytes to /settings.dat
```

---

## 10. Memory Usage

### Build Results
```
✅ Compiled successfully
✅ RAM: 47.6% (38,972 / 81,920 bytes)
✅ Flash: 34.8% (363,383 / 1,044,464 bytes)
```

### Memory Impact
- RAM increase: +212 bytes (from previous build)
- Flash increase: +620 bytes
- Well within ESP8266 constraints

---

## 11. Testing Checklist

### Auto-Lock Sleep Testing
- [ ] Set auto-lock to 5s
- [ ] Navigate to any screen
- [ ] Don't touch for 5 seconds
- [ ] Verify screen turns black (not HOME screen)
- [ ] Double-tap to wake
- [ ] Verify returns to same screen (not HOME)

### Wake Gesture Testing
- [ ] Put device to sleep (auto-lock or manual)
- [ ] Single tap → Verify nothing happens
- [ ] Wait 500ms → Single tap → Verify nothing happens
- [ ] Double-tap quickly (< 400ms) → Verify wakes up
- [ ] Double-tap slowly (> 400ms) → Verify doesn't wake

### Manual Sleep Testing
- [ ] Go to Settings → Enable "Manual Sleep"
- [ ] Go to any screen
- [ ] Single tap → Verify nothing happens
- [ ] Double-tap quickly → Verify screen goes black
- [ ] Double-tap to wake → Verify restores screen

### Edge Cases
- [ ] Sleep from HOME screen → Wake → Verify restores HOME
- [ ] Sleep from Password Detail → Wake → Verify restores detail
- [ ] Set auto-lock to "Never" → Wait 60s → Verify never sleeps
- [ ] Disable Manual Sleep → Double-tap → Verify no manual sleep

---

## 12. Known Limitations

### 1. Backlight Control
Current implementation only fills screen with black. True hardware backlight off requires:
- Defining backlight pin in hardware config
- Implementing PWM/LEDC control
- Hardware-specific implementation (varies by display)

### 2. Power Consumption
Black screen reduces power but doesn't enter deep sleep mode. For true low-power:
- Would need ESP8266 light sleep mode
- Wake on touch interrupt
- More complex implementation

### 3. Wake Window
400ms double-tap window is fixed. Could be made configurable:
- Settings option: "Wake Sensitivity" (300ms - 600ms)
- Stored in SettingsData struct

---

## 13. Future Enhancements

### Potential Features
1. **Backlight Dimming**: Gradual fade to black before sleep
2. **Sleep Animation**: Visual feedback before entering sleep
3. **Wake Animation**: Smooth fade-in on wake
4. **Configurable Wake Window**: User-adjustable sensitivity
5. **Deep Sleep Mode**: True power-off with GPIO wake interrupt
6. **Sleep Statistics**: Track sleep/wake events and duration
7. **Sleep Sounds**: Audio feedback on sleep/wake
8. **Gradual Timeout Warning**: Show countdown before sleep

---

## 14. Architecture Notes

### Design Principles
1. **State Preservation**: Always restore exact screen user was on
2. **Non-Blocking**: Sleep/wake doesn't delay main loop
3. **Touch Isolation**: Wake gestures don't trigger UI interactions
4. **Fail-Safe**: If wake fails, user can still reset device
5. **Clear Feedback**: Serial logging for all state transitions

### Code Organization
```
Sleep State Management (ScreenManager)
    ├─ sleepScreen()          → Enter sleep mode
    ├─ wakeScreen()           → Exit sleep mode
    ├─ checkAutoLock()        → Auto-sleep trigger
    ├─ handleWakeGesture()    → Double-tap wake
    └─ Manual Sleep Detection → Optional quick sleep
```

---

## Implementation Status: ✅ COMPLETE

All requested features successfully implemented:
- ✅ Screen turns completely black on auto-lock (not HOME screen)
- ✅ Double-tap wake gesture (400ms window)
- ✅ Previous screen restored on wake
- ✅ Manual sleep feature (optional double-tap to sleep)
- ✅ Touch input remains active during sleep
- ✅ Settings persist across reboots

**Build**: Compiled successfully, uploaded to device
**Memory**: Within constraints (47.6% RAM, 34.8% Flash)
**Status**: Ready for device testing
