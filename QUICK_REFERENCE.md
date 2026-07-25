# ScreenManager Quick Reference Guide

## The Global Standard - Core Concept

```cpp
┌─────────────────────────────────────────┐
│         THE RENDERING STANDARD          │
├─────────────────────────────────────────┤
│ if (forceFullRedraw) {                  │
│     tft->fillScreen(TFT_BLACK);         │
│     drawStaticUI();                     │
│     forceFullRedraw = false;            │
│     screenIsDirty = true;               │
│ }                                       │
│                                         │
│ if (screenIsDirty) {                    │
│     drawDynamicData();                  │
│     screenIsDirty = false;              │
│ }                                       │
└─────────────────────────────────────────┘
```

## Golden Rules

### Rule #1: Update = No Drawing
```cpp
✅ void updateHomeData() {
    sensorValue = readSensor();
    if (sensorValue != lastValue) {
        screenIsDirty = true;  // Just set the flag
    }
}

❌ void updateHomeData() {
    sensorValue = readSensor();
    tft->print(sensorValue);  // NEVER draw in update!
}
```

### Rule #2: Render = No State Updates
```cpp
✅ void drawHomeDynamicData() {
    int value = sharedSensorValue;  // Read cached value
    tft->print(value);
}

❌ void drawHomeDynamicData() {
    int value = readSensor();  // NEVER read sensors in render!
    tft->print(value);
}
```

### Rule #3: Use pushScreen/popScreen
```cpp
✅ void handleHomeTouch(const TouchPoint& point) {
    if (buttonPressed) {
        pushScreen(SCREEN_SETTINGS);  // Automatic flags
    }
}

❌ void handleHomeTouch(const TouchPoint& point) {
    if (buttonPressed) {
        currentScreen = SCREEN_SETTINGS;  // Manual is error-prone
        forceFullRedraw = true;
    }
}
```

## Quick Patterns

### Pattern: Navigate Forward
```cpp
pushScreen(SCREEN_NEW_SCREEN);
```

### Pattern: Navigate Back
```cpp
popScreen(SCREEN_PREVIOUS_SCREEN);
```

### Pattern: Trigger Data Refresh
```cpp
screenIsDirty = true;
```

### Pattern: Button Hit Detection
```cpp
if (point.x >= BTN_X && point.x <= (BTN_X + BTN_W) && 
    point.y >= BTN_Y && point.y <= (BTN_Y + BTN_H)) {
    pushScreen(SCREEN_TARGET);
}
```

## Common Snippets

### Draw Button with Centered Text
```cpp
tft->fillRect(x, y, w, h, TFT_NAVY);
tft->drawRect(x, y, w, h, TFT_BLUE);
tft->setTextColor(TFT_WHITE, TFT_NAVY);
tft->setTextDatum(MC_DATUM);
tft->drawString("Label", x + w/2, y + h/2);
tft->setTextDatum(TL_DATUM);  // Reset
```

### Update Dynamic Value (Difference Only)
```cpp
static int lastValue = -1;

if (currentValue != lastValue) {
    tft->fillRect(x, y, w, h, TFT_BLACK);  // Clear old
    tft->setCursor(x, y);
    tft->print(currentValue);  // Draw new
    lastValue = currentValue;
}
```

### Clear and Draw Coordinate
```cpp
char buf[16];
snprintf(buf, sizeof(buf), "%d", value);
tft->fillRect(x, y, 80, 8, TFT_BLACK);
tft->setCursor(x, y);
tft->print(buf);
```

## Screen Template

```cpp
// 1. STATIC UI
void drawNewScreenStaticUI() {
    // Draw title
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print("SCREEN TITLE");
    
    // Draw buttons
    tft->fillRect(BTN_X, BTN_Y, BTN_W, BTN_H, TFT_NAVY);
    tft->drawRect(BTN_X, BTN_Y, BTN_W, BTN_H, TFT_BLUE);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Button", BTN_X + BTN_W/2, BTN_Y + BTN_H/2);
    tft->setTextDatum(TL_DATUM);
    
    // Draw static labels
    tft->setTextSize(1);
    tft->setCursor(10, 50);
    tft->print("Label:");
    
    DEBUG_LOG("NewScreen - Static UI drawn");
}

// 2. DYNAMIC DATA
void drawNewScreenDynamicData() {
    static int lastValue = -1;
    
    if (sharedValue != lastValue) {
        tft->fillRect(100, 50, 80, 10, TFT_BLACK);
        tft->setCursor(100, 50);
        tft->print(sharedValue);
        lastValue = sharedValue;
    }
    
    DEBUG_LOG("NewScreen - Dynamic data drawn");
}

// 3. UPDATE DATA
void updateNewScreenData() {
    // Read sensors, process state
    int newValue = readSensor();
    
    if (newValue != sharedValue) {
        sharedValue = newValue;
        screenIsDirty = true;  // Trigger render
    }
}

// 4. HANDLE TOUCH
void handleNewScreenTouch(const TouchPoint& point) {
    // Button detection
    if (point.x >= BTN_X && point.x <= (BTN_X + BTN_W) && 
        point.y >= BTN_Y && point.y <= (BTN_Y + BTN_H)) {
        pushScreen(SCREEN_NEXT);
    }
    
    // Back button
    if (point.x >= BACK_X && point.x <= (BACK_X + BACK_W) && 
        point.y >= BACK_Y && point.y <= (BACK_Y + BACK_H)) {
        popScreen(SCREEN_PREVIOUS);
    }
}
```

## Integration Checklist

When adding a new screen:

- [ ] 1. Add `SCREEN_XXX` to enum in ScreenManager.h
- [ ] 2. Declare four methods in ScreenManager.h (private section):
  - `void drawXXXStaticUI();`
  - `void drawXXXDynamicData();`
  - `void updateXXXData();`
  - `void handleXXXTouch(const TouchPoint& point);`
- [ ] 3. Add cases to `render()` switch (both static and dynamic)
- [ ] 4. Add case to `update()` switch (for updateXXXData)
- [ ] 5. Add case to `update()` switch (for handleXXXTouch)
- [ ] 6. Implement all four methods in ScreenManager.cpp

## Debug Tips

### Enable Debug Logging
```cpp
// In Config.h
#define DEV_MODE 1  // Enable
#define DEV_MODE 0  // Disable
```

### Watch Serial Monitor For
```
ScreenManager initialized - Global Standard Active
pushScreen -> 1 (forceFullRedraw=true)
Home - Static UI drawn
Home - Dynamic data drawn
pushScreen -> 5 (forceFullRedraw=true)
TouchTest - Static UI drawn
TouchTest - Dynamic data drawn
```

### Common Issues

#### Flickering
```
Symptom: Screen flickers constantly
Cause: screenIsDirty being set every frame
Fix: Only set screenIsDirty when data actually changes
```

#### Artifacts
```
Symptom: Old pixels visible after screen change
Cause: fillScreen not being called
Fix: Ensure forceFullRedraw = true on pushScreen/popScreen
```

#### Frozen Display
```
Symptom: Screen doesn't update
Cause: screenIsDirty never being set
Fix: Set screenIsDirty = true in updateXXXData()
```

#### SPI Conflicts
```
Symptom: Touch or display glitches
Cause: Drawing in update() or reading sensors in render()
Fix: Follow separation of concerns rules
```

## Constants Reference

### Button Positions (Config.h)
```cpp
// Top row
ADDPASS_BTN_X      20
ADDPASS_BTN_Y      60
ADDPASS_BTN_W      120
ADDPASS_BTN_H      50

PASSWORDS_BTN_X    180
PASSWORDS_BTN_Y    60
PASSWORDS_BTN_W    120
PASSWORDS_BTN_H    50

// Bottom row
FAVORITES_BTN_X    20
FAVORITES_BTN_Y    130
FAVORITES_BTN_W    120
FAVORITES_BTN_H    50

SETTINGS_BTN_X     180
SETTINGS_BTN_Y     130
SETTINGS_BTN_W     120
SETTINGS_BTN_H     50
```

### Colors
```cpp
TFT_BLACK          0x0000
TFT_NAVY           0x000F
TFT_DARKGREEN      0x03E0
TFT_MAROON         0x7800
TFT_PURPLE         0x780F
TFT_OLIVE          0x7BE0
TFT_BLUE           0x001F
TFT_GREEN          0x07E0
TFT_RED            0xF800
TFT_WHITE          0xFFFF
TFT_YELLOW         0xFFE0
Gray (custom)      0x8410
```

### Text Datum Modes
```cpp
TL_DATUM  // Top Left (default)
TC_DATUM  // Top Center
TR_DATUM  // Top Right
ML_DATUM  // Middle Left
MC_DATUM  // Middle Center (for buttons)
MR_DATUM  // Middle Right
BL_DATUM  // Bottom Left
BC_DATUM  // Bottom Center
BR_DATUM  // Bottom Right
```

## Performance Targets

- **Screen Transition**: < 100ms
- **Dynamic Update**: < 20ms
- **Touch Response**: < 50ms
- **TouchTest Dot**: 20 updates/second (50ms cycle)
- **Main Loop**: 50ms cycle (20 Hz)

## Memory Budget

- **Static Variables**: < 1KB per screen
- **Stack Usage**: < 512 bytes per function
- **Free Heap**: > 20KB during operation
- **Total Flash**: < 32KB for ScreenManager

## Best Practices

### DO
✅ Use static variables for tracking last drawn values
✅ Clear old content before drawing new (fillRect)
✅ Reset text datum after changing it
✅ Use MC_DATUM for centered button text
✅ Constrain coordinates to screen bounds
✅ Add DEBUG_LOG for state changes

### DON'T
❌ Draw in update() methods
❌ Read sensors in render() methods
❌ Set currentScreen directly
❌ Forget to clear dirty flags
❌ Use floating point math for coordinates
❌ Draw outside screen bounds

## Testing Command
```
Build: platformio run
Upload: platformio run --target upload
Monitor: platformio device monitor
Clean: platformio run --target clean
```

---

**Remember:** When in doubt, follow the TouchTest pattern - it's the proven reference implementation!
