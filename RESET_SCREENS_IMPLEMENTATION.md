# Reset Screens - Clean, Isolated Implementation

## Design Philosophy

Two **completely separate** classes with **zero shared dependencies**:
1. `CalibrationResetScreen` - Resets touch calibration only
2. `FactoryResetScreen` - Resets all settings and passwords

Each screen is self-contained and follows the same architectural pattern, making them easy to:
- Debug independently
- Modify without affecting the other
- Test in isolation
- Understand at a glance

---

## 1. CalibrationResetScreen

### Purpose
Allows users to reset touch calibration to factory defaults when the touchscreen becomes inaccurate or miscalibrated.

### Visual Design

```
┌──────────────────────────────────────┐
│     Reset Calibration                │  ← Red heading
│        to Factory                    │
│                                      │
│           ⚠                          │  ← Yellow warning triangle
│          /!\                         │
│                                      │
│   This will reset touch              │  ← White description
│   calibration to defaults            │
│                                      │
│                                      │
│  ┌──────────┐       ┌──────────┐    │
│  │  Cancel  │       │  Reset   │    │  ← Blue / Red buttons
│  └──────────┘       │Calibration│   │
│                     └──────────┘    │
└──────────────────────────────────────┘
```

### Button Layout

- **Cancel** (Bottom-Left): Navy blue background, blue border
  - Position: (20, 190), Size: 130×40
  - Action: Return to settings without changes

- **Reset Calibration** (Bottom-Right): Maroon background, red border
  - Position: (170, 190), Size: 130×40
  - Action: Call `settings->resetCalibration()` and save

### Code Structure

```cpp
class CalibrationResetScreen {
private:
    TFT_eSPI* display;
    TouchManager* touch;
    SettingsManager* settings;
    bool needsExit;
    
public:
    void draw();               // Renders UI
    void update();             // Logic updates
    void onTouchEvent();       // Handle touches
    bool needsToExit();        // Check exit flag
    
private:
    void drawStaticUI();       // Headers, buttons
    void drawDynamicData();    // (None for this screen)
    void performReset();       // Execute reset action
};
```

### Action Flow

```
User taps "Reset Calibration"
    ↓
performReset() called
    ↓
settings->resetCalibration()  // Reset to defaults
    ↓
settings->save()              // Persist to storage
    ↓
needsExit = true              // Signal to ScreenManager
    ↓
ScreenManager.update() detects needsExit
    ↓
popScreen() → Return to Settings
```

---

## 2. FactoryResetScreen

### Purpose
Allows users to completely wipe all settings, calibration, and passwords back to factory state. This is a nuclear option for troubleshooting or device handoff.

### Visual Design

```
┌──────────────────────────────────────┐
│       Factory Reset                  │  ← Red heading
│                                      │
│           ⚠                          │  ← Yellow warning triangle
│          /!\                         │
│                                      │
│   This will reset                    │  ← White description
│   all settings/passwords             │
│                                      │
│   This cannot be undone!             │  ← Yellow warning
│                                      │
│  ┌──────────┐       ┌──────────┐    │
│  │  Cancel  │       │ Factory  │    │  ← Blue / Red buttons
│  └──────────┘       │  Reset   │    │
│                     └──────────┘    │
└──────────────────────────────────────┘
```

### Button Layout

- **Cancel** (Bottom-Left): Navy blue background, blue border
  - Position: (20, 190), Size: 130×40
  - Action: Return to settings without changes

- **Factory Reset** (Bottom-Right): Maroon background, red border
  - Position: (170, 190), Size: 130×40
  - Action: Call `settings->factoryReset()` and save

### Code Structure

```cpp
class FactoryResetScreen {
private:
    TFT_eSPI* display;
    TouchManager* touch;
    SettingsManager* settings;
    bool needsExit;
    
public:
    void draw();               // Renders UI
    void update();             // Logic updates
    void onTouchEvent();       // Handle touches
    bool needsToExit();        // Check exit flag
    
private:
    void drawStaticUI();       // Headers, buttons
    void drawDynamicData();    // (None for this screen)
    void performFactoryReset(); // Execute reset action
};
```

### Action Flow

```
User taps "Factory Reset"
    ↓
performFactoryReset() called
    ↓
settings->factoryReset()      // Reset EVERYTHING
    ↓
settings->save()              // Persist to storage
    ↓
needsExit = true              // Signal to ScreenManager
    ↓
ScreenManager.update() detects needsExit
    ↓
popScreen() → Return to Settings
```

---

## Color Scheme (Consistent Across Both)

```cpp
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_ERROR TFT_RED              // Headings and reset button border
#define COLOR_WARNING TFT_YELLOW          // Warning triangle and text
#define COLOR_BUTTON_CANCEL TFT_NAVY      // Cancel button fill
#define COLOR_BUTTON_CANCEL_BORDER TFT_BLUE  // Cancel button outline
// Reset buttons use TFT_MAROON fill with COLOR_ERROR border
```

This consistent color scheme ensures users immediately recognize:
- **Red** = Destructive action
- **Yellow** = Warning/caution
- **Blue** = Safe/cancel action

---

## Integration with ScreenManager

### Adding to ScreenManager (Example)

```cpp
// In ScreenManager.h
enum ScreenState {
    SCREEN_HOME,
    SCREEN_SETTINGS,
    SCREEN_CALIBRATION,
    SCREEN_CALIBRATION_RESET,  // NEW
    SCREEN_FACTORY_RESET,      // NEW
    // ...
};

class ScreenManager {
private:
    CalibrationResetScreen* calibrationResetScreen;
    FactoryResetScreen* factoryResetScreen;
    
    // ...
};
```

### Navigation from Settings

```cpp
// In ScreenManager::handleSettingsTouch()
if (resetCalibrationButtonPressed) {
    pushScreen(SCREEN_CALIBRATION_RESET);
}

if (factoryResetButtonPressed) {
    pushScreen(SCREEN_FACTORY_RESET);
}
```

### Update Loop Handling

```cpp
// In ScreenManager::update()
case SCREEN_CALIBRATION_RESET:
    updateCalibrationResetData();
    break;
    
case SCREEN_FACTORY_RESET:
    updateFactoryResetData();
    break;

void ScreenManager::updateCalibrationResetData() {
    if (calibrationResetScreen->needsToExit()) {
        popScreen(SCREEN_SETTINGS);  // Return to settings
        calibrationResetScreen->reset();  // Clear exit flag
    }
}

void ScreenManager::updateFactoryResetData() {
    if (factoryResetScreen->needsToExit()) {
        popScreen(SCREEN_SETTINGS);  // Return to settings
        factoryResetScreen->reset();  // Clear exit flag
    }
}
```

---

## File Structure

```
include/
  ├── CalibrationResetScreen.h
  └── FactoryResetScreen.h

src/
  ├── CalibrationResetScreen.cpp
  └── FactoryResetScreen.cpp
```

Each pair of files is **completely independent** - no shared base class, no common utility functions, no dependencies between them.

---

## Benefits of Separation

### 1. **Zero Coupling**
- Change one screen without affecting the other
- No risk of breaking factory reset when modifying calibration reset

### 2. **Easy to Understand**
- Each class is small and focused
- No inheritance hierarchy to navigate
- All code for one feature in one place

### 3. **Simple Testing**
- Test calibration reset without factory reset code loaded
- Mock SettingsManager independently for each

### 4. **Clear Responsibilities**
- CalibrationResetScreen → Touch calibration only
- FactoryResetScreen → Everything

### 5. **Maintainability**
- Future developer sees two distinct screens
- No need to understand shared abstractions
- Easy to add new reset types later

---

## Usage Example

### From Settings Screen

```cpp
// Settings button handling
if (point.x >= RESET_CAL_BTN_X && point.y >= RESET_CAL_BTN_Y) {
    // Navigate to calibration reset screen
    screenManager->pushScreen(SCREEN_CALIBRATION_RESET);
}

if (point.x >= FACTORY_RESET_BTN_X && point.y >= FACTORY_RESET_BTN_Y) {
    // Navigate to factory reset screen
    screenManager->pushScreen(SCREEN_FACTORY_RESET);
}
```

### In Main Loop

```cpp
void loop() {
    touchManager->update();
    screenManager->update();  // Handles needsToExit() checks
    screenManager->render();  // Calls draw() on active screen
    delay(50);
}
```

---

## SettingsManager Interface Required

Both screens depend on a `SettingsManager` class with these methods:

```cpp
class SettingsManager {
public:
    void resetCalibration();  // Reset touch cal to defaults
    void factoryReset();       // Reset everything
    void save();               // Persist to EEPROM/Flash
};
```

---

## Debug Output

### CalibrationResetScreen
```
CalibrationResetScreen initialized
CalibrationReset: Touch at (220,210)
CalibrationReset: Reset Calibration pressed
CalibrationReset: Performing calibration reset...
CalibrationReset: Reset complete
```

### FactoryResetScreen
```
FactoryResetScreen initialized
FactoryReset: Touch at (220,210)
FactoryReset: Factory Reset pressed
FactoryReset: Performing factory reset...
FactoryReset: Factory reset complete
```

---

## Testing Checklist

### CalibrationResetScreen
- [ ] Screen renders correctly
- [ ] Cancel button returns to settings without changes
- [ ] Reset Calibration button calls `settings->resetCalibration()`
- [ ] Reset Calibration button calls `settings->save()`
- [ ] needsToExit flag is set after action
- [ ] Red button styling matches design

### FactoryResetScreen
- [ ] Screen renders correctly
- [ ] Cancel button returns to settings without changes
- [ ] Factory Reset button calls `settings->factoryReset()`
- [ ] Factory Reset button calls `settings->save()`
- [ ] needsToExit flag is set after action
- [ ] Red button styling matches design
- [ ] Warning text is visible and clear

---

## Future Enhancements (Optional)

### Confirmation Dialog
Add a second confirmation step:
```cpp
if (isResetButtonPressed()) {
    if (!confirmationShown) {
        showConfirmationPrompt();
    } else {
        performReset();
    }
}
```

### Progress Indicator
Show visual feedback during reset:
```cpp
void performReset() {
    drawProcessingMessage();
    settings->resetCalibration();
    drawSuccessMessage();
    delay(1000);
    needsExit = true;
}
```

### Sound Feedback
Add audio confirmation:
```cpp
void performReset() {
    playWarningTone();
    settings->resetCalibration();
    playSuccessTone();
}
```

---

## Summary

Two clean, isolated classes that:
- ✅ Have no shared dependencies
- ✅ Follow the same architectural pattern
- ✅ Use consistent COLOR_ERROR styling
- ✅ Handle their own drawing and touch events
- ✅ Properly signal exit via needsToExit()
- ✅ Call appropriate SettingsManager methods
- ✅ Are easy to debug, test, and maintain

This is the clean, modular approach you requested!
