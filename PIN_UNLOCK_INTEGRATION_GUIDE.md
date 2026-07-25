# PIN Unlock System Integration Guide

## Overview
This document describes the PIN unlock security system implementation with Apple Watch-style keypad.

## Components Created

### 1. PINManager (include/PINManager.h, src/PINManager.cpp)
- **Purpose**: Handles PIN storage, validation, and security
- **Storage**: `/pin.dat` on LittleFS with magic number validation
- **Default PIN**: "1234" (set on first boot)
- **Features**:
  - `validatePIN()` - Check if entered PIN is correct
  - `setPIN()` - Set new PIN (4 digits required)
  - `changePIN()` - Change PIN with current PIN validation
  - `resetPIN()` - Reset to default (1234)

### 2. PINEntryScreen (include/PINEntryScreen.h, src/PINEntryScreen.cpp)
- **Purpose**: Lock screen with Apple Watch-style circular keypad
- **Features**:
  - 4 indicator dots at top (fill as digits entered)
  - Circular number pad (0-9 + backspace)
  - Auto-validates on 4th digit
  - Shake animation on wrong PIN (500ms)
  - Unlocks vault on correct PIN

### 3. ChangePINScreen (include/ChangePINScreen.h, src/ChangePINScreen.cpp)
- **Purpose**: Settings screen for changing PIN
- **Workflow**:
  1. Enter current PIN (validates)
  2. Enter new PIN (4 digits)
  3. Confirm new PIN (must match)
  4. Success message → saves to storage
- **Error handling**: Shows error messages for 2 seconds

## Integration Steps

### Step 1: Update ScreenManager

Already completed:
- ✅ Added SCREEN_PIN_ENTRY and SCREEN_CHANGE_PIN to enum
- ✅ Added forward declarations
- ✅ Added private members (pinManager, pinEntryScreen, changePINScreen)
- ✅ Added method declarations
- ✅ Updated constructor to start with SCREEN_PIN_ENTRY
- ✅ Updated begin() to initialize PIN manager

### Step 2: Implement Screen Handlers (REQUIRED)

Add to ScreenManager.cpp update() method:
```cpp
case SCREEN_PIN_ENTRY:
    if (pinEntryScreen != nullptr) {
        pinEntryScreen->update();
        
        // Check if unlocked
        if (pinEntryScreen->isUnlocked()) {
            Serial.println("[SCREEN_MGR] PIN correct - transitioning to HOME");
            pushScreen(SCREEN_HOME);
        }
    }
    updatePINEntryData();
    break;

case SCREEN_CHANGE_PIN:
    if (changePINScreen == nullptr) {
        changePINScreen = new ChangePINScreen(tft, touch, pinManager);
        changePINScreen->begin();
    }
    changePINScreen->update();
    
    // Check if should exit
    if (changePINScreen->shouldExit()) {
        Serial.println("[SCREEN_MGR] ChangePIN complete - returning to settings");
        popScreen(SCREEN_SETTINGS);
    }
    
    updateChangePINData();
    break;
```

Add to ScreenManager.cpp renderCurrentScreen() method:
```cpp
case SCREEN_PIN_ENTRY:
    if (pinEntryScreen != nullptr) {
        pinEntryScreen->draw();
    } else {
        drawPINEntryStaticUI();
        drawPINEntryDynamicData();
    }
    break;

case SCREEN_CHANGE_PIN:
    if (changePINScreen != nullptr) {
        changePINScreen->draw();
    } else {
        drawChangePINStaticUI();
        drawChangePINDynamicData();
    }
    break;
```

Add touch handlers (find handleXXXTouch methods section):
```cpp
void ScreenManager::handlePINEntryTouch(const TouchPoint& point) {
    if (pinEntryScreen != nullptr) {
        pinEntryScreen->handleTouch(point);
    }
}

void ScreenManager::handleChangePINTouch(const TouchPoint& point) {
    if (changePINScreen != nullptr) {
        changePINScreen->handleTouch(point);
    }
}
```

Add placeholder draw/update methods (end of file):
```cpp
// PIN Entry Screen
void ScreenManager::drawPINEntryStaticUI() {
    // Handled by PINEntryScreen class
}

void ScreenManager::drawPINEntryDynamicData() {
    // Handled by PINEntryScreen class
}

void ScreenManager::updatePINEntryData() {
    // Handled by PINEntryScreen class
}

// Change PIN Screen
void ScreenManager::drawChangePINStaticUI() {
    // Handled by ChangePINScreen class
}

void ScreenManager::drawChangePINDynamicData() {
    // Handled by ChangePINScreen class
}

void ScreenManager::updateChangePINData() {
    // Handled by ChangePINScreen class
}
```

### Step 3: Add Change PIN to Settings Menu

Update SettingsListScreen (enum in .h, draw and touch in .cpp):

In `include/SettingsListScreen.h`:
```cpp
enum SettingsRow {
    SETTINGS_ROW_AUTO_LOCK = 0,
    SETTINGS_ROW_TWO_TAP_SLEEP = 1,
    SETTINGS_ROW_CHANGE_PIN = 2,     // NEW
    SETTINGS_ROW_ADVANCED = 3,       // Shifted
    SETTINGS_ROW_ABOUT = 4,          // Shifted
    SETTINGS_ROW_NONE = -1
};
```

In `src/SettingsListScreen.cpp` drawDynamicData():
```cpp
// Row 3: Change PIN
drawSettingsRow(y, "Change PIN", nullptr, ">", false, false);
y += ROW_HEIGHT + ROW_SPACING;
```

In `src/SettingsListScreen.cpp` handleTouch():
```cpp
case SETTINGS_ROW_CHANGE_PIN:
    selectedAction = SETTINGS_ROW_CHANGE_PIN;
    Serial.println("[SETTINGS_LIST] Change PIN selected");
    break;
```

In `src/SettingsListScreen.cpp` getSelectedAction() usage (ScreenManager):
```cpp
case SETTINGS_ROW_CHANGE_PIN:
    pushScreen(SCREEN_CHANGE_PIN);
    break;
```

### Step 4: Auto-Lock Returns to PIN Screen

Update `checkAutoLock()` in ScreenManager.cpp:
```cpp
void ScreenManager::checkAutoLock() {
    if (!settingsManager->isAutoLockEnabled()) {
        return;  // Auto-lock disabled
    }
    
    unsigned long timeoutMs = settingsManager->getAutoLockTimeout() * 1000;
    unsigned long elapsed = millis() - lastInteractionTime;
    
    if (elapsed >= timeoutMs && !autoLockTriggered && !screenAsleep) {
        Serial.println("[AUTO_LOCK] Timeout reached - LOCKING DEVICE");
        
        // Lock device - return to PIN entry screen
        pinEntryScreen->lock();
        pushScreen(SCREEN_PIN_ENTRY);
        
        autoLockTriggered = true;
    }
}
```

### Step 5: Update Main Loop

In `src/main.cpp`, ensure auto-lock is checked:
```cpp
void loop() {
    screenManager.update();
    screenManager.checkAutoLock();  // Check for auto-lock timeout
    screenManager.render();
}
```

## Design System Colors

All PIN screens use the standard design tokens:
```cpp
#define COLOR_INK 0x0841          // Background
#define COLOR_SURFACE 0x18C3      // Button/card surfaces
#define COLOR_LINE 0x2945         // Borders
#define COLOR_BRASS 0xFD60        // Primary accent (filled dots, pressed keys)
#define COLOR_IVORY 0xFFDE        // Text
#define COLOR_MUTED 0x8C51        // Secondary text
#define COLOR_ERROR 0xF800        // Error red (shake animation)
#define COLOR_SUCCESS 0x07E0      // Success green
```

## Layout Specifications

### PIN Entry Screen
- **Title**: "Enter PIN" at Y=20
- **Subtitle**: "Tap to Unlock" at Y=42
- **Indicator Dots**: 4 dots, 8px radius, 24px spacing, centered at Y=60
- **Keypad**: Starts at Y=100, 60px key size, 10px spacing
- **Keys Layout**: 3x4 grid (1-9, backspace-0-empty)

### Change PIN Screen
- **Header**: Back button + "Change PIN" title at Y=16
- **Prompt**: Dynamic text at Y=48
- **Indicator Dots**: Same as PIN entry (Y=80)
- **Keypad**: Starts at Y=120, 50px key size, 8px spacing

## User Flow

### Initial Boot
1. Device boots → PIN Entry Screen
2. Default PIN is "1234" (shown in first-time setup docs)
3. User enters PIN → Unlocks to HOME screen

### Auto-Lock
1. User inactive for configured timeout (5s, 10s, 15s, etc.)
2. Device locks → PIN Entry Screen
3. User enters PIN → Returns to previous screen (preserved)

### Change PIN
1. User navigates: HOME → Settings → Change PIN
2. Enter current PIN (validates)
3. Enter new PIN
4. Confirm new PIN
5. Success message → Saved to storage
6. Returns to Settings

### Wake from Sleep
1. Double-tap to wake (PIN entry screen shown if locked)
2. Enter PIN to unlock
3. Resume to previous screen

## Security Features

1. **PIN Persistence**: Stored in `/pin.dat` with magic number validation
2. **Auto-Lock**: Configurable timeout locks device automatically
3. **PIN Validation**: Must be exactly 4 digits
4. **Error Feedback**: Shake animation on wrong PIN (no retry limit by default)
5. **Default PIN**: "1234" (user should change immediately)

## Testing Checklist

- [ ] Initial boot shows PIN entry screen
- [ ] Default PIN "1234" unlocks device
- [ ] Wrong PIN shows shake animation and clears dots
- [ ] Correct PIN transitions to HOME screen
- [ ] Change PIN workflow (current → new → confirm)
- [ ] Change PIN validates current PIN
- [ ] Change PIN requires matching confirmation
- [ ] New PIN persists across reboot
- [ ] Auto-lock returns to PIN screen after timeout
- [ ] PIN screen shows after wake from sleep
- [ ] Back button exits Change PIN screen

## Files Modified/Created

### Created
- `include/PINManager.h`
- `src/PINManager.cpp`
- `include/PINEntryScreen.h`
- `src/PINEntryScreen.cpp`
- `include/ChangePINScreen.h`
- `src/ChangePINScreen.cpp`

### Modified (Partial - needs completion)
- `include/ScreenManager.h` - Added enums, forward declarations, members
- `src/ScreenManager.cpp` - Added includes, constructor, begin() method

### Need Modification (Step 2-4)
- `src/ScreenManager.cpp` - Add update/render/touch handlers
- `include/SettingsListScreen.h` - Add CHANGE_PIN row enum
- `src/ScreenManager.cpp` - Add placeholder methods at end of file
- `src/SettingsListScreen.cpp` - Add Change PIN row to UI and touch handler

## Next Steps

1. Complete ScreenManager integration (Step 2)
2. Add Change PIN to Settings menu (Step 3)
3. Update auto-lock to return to PIN screen (Step 4)
4. Build and test on device
5. Document default PIN ("1234") for users

## Default PIN Notice

**IMPORTANT**: The default PIN is "1234". Users should be instructed to change this immediately via Settings → Change PIN for security.

Consider adding a first-time setup wizard that forces PIN change on first boot (future enhancement).
