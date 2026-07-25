# ScreenManager Architecture - Event-Driven Rendering

## System Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                          MAIN LOOP                              │
│                         (50ms cycle)                            │
└─────────────────────────────────────────────────────────────────┘
                                 │
                                 ├─→ touchManager->update()
                                 │   └─→ Sample touch sensor
                                 │
                                 ├─→ screenManager->update()  ─────┐
                                 │                                 │
                                 └─→ screenManager->render()  ─────┼──┐
                                                                   │  │
                                                                   │  │
┌──────────────────────────────────────────────────────────────────┼──┼────┐
│                         UPDATE PHASE (NO DRAWING)                │  │    │
└──────────────────────────────────────────────────────────────────┼──┼────┘
                                                                   │  │
    ┌──────────────────────────────────────────────────────────────┘  │
    │                                                                  │
    ├─→ updateXXXData()  ────────────────────────────────────────────┐│
    │   ├─ Read sensors                                              ││
    │   ├─ Process state machine                                     ││
    │   ├─ Update shared variables                                   ││
    │   └─ Set screenIsDirty = true (if changed)                     ││
    │                                                                 ││
    └─→ handleXXXTouch()  ───────────────────────────────────────────┼┘
        ├─ Check button hits                                         │
        └─ Call pushScreen() or popScreen()                          │
            └─→ Set forceFullRedraw = true  ─────────────────────────┼─┐
                                                                      │ │
                                                                      │ │
┌─────────────────────────────────────────────────────────────────────┼─┼──┐
│                      RENDER PHASE (ONLY DRAWING)                    │ │  │
└─────────────────────────────────────────────────────────────────────┼─┼──┘
                                                                      │ │
    ┌─────────────────────────────────────────────────────────────────┘ │
    │                                                                    │
    ├─→ if (forceFullRedraw) {  ◄───────────────────────────────────────┘
    │       tft->fillScreen(TFT_BLACK)
    │       drawXXXStaticUI()
    │       ├─ Draw title
    │       ├─ Draw buttons
    │       ├─ Draw labels
    │       └─ Draw static elements
    │       forceFullRedraw = false
    │       screenIsDirty = true
    │   }
    │
    └─→ if (screenIsDirty) {
            drawXXXDynamicData()
            ├─ Read shared variables
            ├─ Update coordinates
            ├─ Update values
            └─ Draw changing elements
            screenIsDirty = false
        }
```

## State Machine

```
┌─────────────────────────────────────────────────────────────────┐
│                      SCREEN LIFECYCLE                           │
└─────────────────────────────────────────────────────────────────┘

                    pushScreen(NEW_SCREEN)
                            │
                            ▼
                  ┌──────────────────────┐
                  │ forceFullRedraw=true │
                  │   screenIsDirty=true │
                  └──────────────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │   render() - First Frame      │
            ├───────────────────────────────┤
            │ 1. fillScreen(TFT_BLACK)      │
            │ 2. drawStaticUI()             │
            │    └─→ Title, buttons, labels │
            │ 3. forceFullRedraw = false    │
            │ 4. screenIsDirty = true       │
            └───────────────────────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │   render() - Second Frame     │
            ├───────────────────────────────┤
            │ 1. drawDynamicData()          │
            │    └─→ Initial values         │
            │ 2. screenIsDirty = false      │
            └───────────────────────────────┘
                            │
                            ▼
            ┌───────────────────────────────┐
            │   Steady State Loop           │
            ├───────────────────────────────┤
            │ update() → updateXXXData()    │
            │   └─→ Sets screenIsDirty      │
            │                               │
            │ render() → if screenIsDirty   │
            │   └─→ drawDynamicData()       │
            └───────────────────────────────┘
                            │
                            │ Touch detected
                            ▼
                  ┌──────────────────────┐
                  │   handleXXXTouch()   │
                  │ pushScreen/popScreen │
                  └──────────────────────┘
                            │
                            └──→ [Loop back to top]
```

## SPI Bus Management (TouchTest Pattern)

```
┌─────────────────────────────────────────────────────────────────┐
│                    SPI BUS ARBITRATION                          │
└─────────────────────────────────────────────────────────────────┘

    UPDATE PHASE                    RENDER PHASE
    (Touch Sensor)                  (Display)
         │                               │
         │                               │
    ┌────▼─────────┐              ┌─────▼──────┐
    │ TFT_CS=HIGH  │              │ TFT_CS=LOW │
    │ TOUCH_CS=LOW │              │ TOUCH_CS=? │
    └────┬─────────┘              └─────┬──────┘
         │                               │
         ├─→ ts.touched()                ├─→ tft->fillRect()
         ├─→ ts.getPoint()               ├─→ tft->drawString()
         ├─→ map(x,y)                    ├─→ tft->fillCircle()
         ├─→ Store in shared vars        └─→ Read shared vars
         └─→ Set screenIsDirty
                    
    NO DRAWING HERE!               NO SENSOR READING HERE!
```

## Screen Transition Example

```
HOME SCREEN                    SETTINGS SCREEN
    │                               │
    │  User touches "Settings"      │
    │         button                │
    ├────────────────────────────┐  │
    │ handleHomeTouch()          │  │
    │   └─→ pushScreen(SETTINGS) │  │
    │        ├─ currentScreen = SETTINGS
    │        └─ forceFullRedraw = true
    └────────────────────────────┘  │
                                    │
                                    ▼
                            Next render() call
                                    │
                            ┌───────▼────────┐
                            │ fillScreen()   │
                            │ drawSettingsUI │
                            └───────┬────────┘
                                    │
                                    ▼
                            Settings screen visible
```

## Data Flow - Touch Test Screen

```
┌─────────────────────────────────────────────────────────────────┐
│                  TOUCH TEST DATA FLOW                           │
└─────────────────────────────────────────────────────────────────┘

    INPUT                UPDATE                  RENDER
      │                    │                       │
      │                    │                       │
    ┌─▼──────────┐   ┌─────▼────────┐   ┌────────▼────────┐
    │ Touch XY   │──→│ updateTouch  │──→│ drawDynamic     │
    │ Sensor     │   │ TestData()   │   │ Data()          │
    └────────────┘   └──────────────┘   └─────────────────┘
                            │                     │
                            ├─ Map coords        ├─ Read sharedX/Y/P
                            ├─ Deadzone check    ├─ Update labels
                            ├─ Store in:         ├─ Clear old dot
                            │  • sharedTouchX    └─ Draw new dot
                            │  • sharedTouchY
                            │  • sharedTouchP
                            │  • sharedTouchActive
                            └─ screenIsDirty=true

    NO DRAWING!                                   NO SENSOR READ!
```

## Memory Layout

```
┌─────────────────────────────────────────────────────────────────┐
│                      SCREENMAMAGER CLASS                        │
├─────────────────────────────────────────────────────────────────┤
│ TFT_eSPI* tft              ← Display driver pointer            │
│ TouchManager* touch         ← Touch manager pointer            │
│ ScreenState currentScreen   ← Current screen enum              │
│                                                                 │
│ bool forceFullRedraw        ← THE STANDARD: Screen transition  │
│ bool screenIsDirty          ← THE STANDARD: Data changed       │
│                                                                 │
│ volatile int16_t sharedTouchX    ← Touch Test: X coordinate   │
│ volatile int16_t sharedTouchY    ← Touch Test: Y coordinate   │
│ volatile int16_t sharedTouchP    ← Touch Test: Pressure       │
│ volatile bool sharedTouchActive  ← Touch Test: Active flag    │
└─────────────────────────────────────────────────────────────────┘

Static Variables (Inside Functions)
┌─────────────────────────────────────────────────────────────────┐
│ drawTouchTestDynamicData()                                      │
│   ├─ lastDotX, lastDotY      ← Previous dot position          │
│   └─ lastDisplayX/Y/Z         ← Previous displayed values      │
│                                                                 │
│ updateTouchTestData()                                           │
│   ├─ lastSampleX/Y/P          ← Previous sensor reading       │
│   └─ lastTouchState           ← Previous touch state          │
└─────────────────────────────────────────────────────────────────┘
```

## Timing Diagram

```
Time →
0ms     50ms    100ms   150ms   200ms   250ms   300ms
│       │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┼───────┼───────
│       │       │       │       │       │       │
│ Touch │ Touch │ Touch │ Touch │ Touch │ Touch │ Touch
│ Update│ Update│ Update│ Update│ Update│ Update│ Update
│       │       │       │       │       │       │
│ Screen│ Screen│ Screen│ Screen│ Screen│ Screen│ Screen
│ Update│ Update│ Update│ Update│ Update│ Update│ Update
│       │       │       │       │       │       │
│ Render│ Render│ Render│ Render│ Render│ Render│ Render
│ (skip)│ (skip)│ (draw)│ (skip)│ (skip)│ (draw)│ (skip)
│       │       │  ▲    │       │       │  ▲    │
│       │       │  │    │       │       │  │    │
│       │       │  └────┼───────┼───────┼──┘    │
│       │       │       │       │       │       │
│       │       │   screenIsDirty=true  │       │
│       │       │   (touch moved)       │       │
```

## Performance Metrics

```
┌─────────────────────────────────────────────────────────────────┐
│                     RENDER EFFICIENCY                           │
├─────────────────────────────────────────────────────────────────┤
│ Static UI Draw:      ~50-100ms  (Once per screen)             │
│ Dynamic Update:      ~5-20ms    (Only when dirty)             │
│ Skipped Render:      ~0ms       (Most frames)                 │
│                                                                 │
│ Touch Test Screen:                                             │
│   └─ Continuous updates at 20Hz with zero flicker             │
│                                                                 │
│ Screen Transitions:                                            │
│   └─ Clean switch with full redraw in 2 frames               │
└─────────────────────────────────────────────────────────────────┘
```

## Anti-Patterns to Avoid

```
❌ WRONG: Drawing in update()
    void updateHomeData() {
        int count = getCount();
        tft->print(count);  // NO! This causes SPI conflicts
    }

✅ RIGHT: Set dirty flag in update()
    void updateHomeData() {
        int count = getCount();
        if (count != lastCount) {
            screenIsDirty = true;  // YES! Trigger render
            lastCount = count;
        }
    }

❌ WRONG: Reading sensors in render()
    void drawHomeDynamicData() {
        int value = analogRead(A0);  // NO! Keep render pure
        tft->print(value);
    }

✅ RIGHT: Read from shared state in render()
    void drawHomeDynamicData() {
        int value = sharedSensorValue;  // YES! Read cached
        tft->print(value);
    }

❌ WRONG: Manual screen state management
    currentScreen = SCREEN_SETTINGS;
    needsRedraw = true;  // NO! Use the standard

✅ RIGHT: Use transition methods
    pushScreen(SCREEN_SETTINGS);  // YES! Automatic flags
```
