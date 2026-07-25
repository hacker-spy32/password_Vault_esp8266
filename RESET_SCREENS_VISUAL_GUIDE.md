# Reset Screens - Visual Design Guide

## Side-by-Side Comparison

```
CalibrationResetScreen          |  FactoryResetScreen
────────────────────────────────|────────────────────────────────
                                |
  Reset Calibration (RED)       |    Factory Reset (RED)
     to Factory                 |
                                |
        ⚠️ (YELLOW)              |        ⚠️ (YELLOW)
       /!\                      |       /!\
                                |
  This will reset touch         |  This will reset
  calibration to defaults       |  all settings/passwords
                                |
                                |  This cannot be undone! (YELLOW)
                                |
┌──────────┐  ┌──────────┐     | ┌──────────┐  ┌──────────┐
│  Cancel  │  │  Reset   │     | │  Cancel  │  │ Factory  │
│  (BLUE)  │  │Calibration│    | │  (BLUE)  │  │  Reset   │
│          │  │  (RED)   │     | │          │  │  (RED)   │
└──────────┘  └──────────┘     | └──────────┘  └──────────┘
```

## Detailed Layouts

### CalibrationResetScreen
```
┌─────────────────────────────────────────┐
│ 0,0                              320,0  │
│                                         │
│          (Y=20, Centered)               │
│     Reset Calibration (SIZE=2, RED)     │
│          (Y=40, Centered)               │
│        to Factory (SIZE=2, RED)         │
│                                         │
│               (Y=75-115)                │
│            ⚠️ Warning Triangle           │
│           (YELLOW outline)              │
│          ! (SIZE=3, YELLOW)             │
│                                         │
│          (Y=130, Centered)              │
│    This will reset touch (SIZE=1)       │
│          (Y=145, Centered)              │
│   calibration to defaults (SIZE=1)      │
│                                         │
│                                         │
│ (20,190)         (170,190)              │
│ ┌────130px──┐   ┌────130px──┐          │
│ │           │   │            │          │
│ │  Cancel   │   │   Reset    │  Y+40    │
│ │  (NAVY)   │   │ Calibration│          │
│ │  Border:  │   │  (MAROON)  │          │
│ │  (BLUE)   │   │  Border:   │          │
│ │           │   │   (RED)    │          │
│ └───────────┘   └────────────┘          │
│                                         │
└─────────────────────────────────────────┘
  0,240                           320,240
```

### FactoryResetScreen
```
┌─────────────────────────────────────────┐
│ 0,0                              320,0  │
│                                         │
│          (Y=30, Centered)               │
│     Factory Reset (SIZE=2, RED)         │
│                                         │
│               (Y=65-105)                │
│            ⚠️ Warning Triangle           │
│           (YELLOW outline)              │
│          ! (SIZE=3, YELLOW)             │
│                                         │
│          (Y=120, Centered)              │
│      This will reset (SIZE=1)           │
│          (Y=135, Centered)              │
│   all settings/passwords (SIZE=1)       │
│                                         │
│          (Y=160, Centered)              │
│  This cannot be undone! (YELLOW)        │
│                                         │
│ (20,190)         (170,190)              │
│ ┌────130px──┐   ┌────130px──┐          │
│ │           │   │            │          │
│ │  Cancel   │   │  Factory   │  Y+40    │
│ │  (NAVY)   │   │   Reset    │          │
│ │  Border:  │   │  (MAROON)  │          │
│ │  (BLUE)   │   │  Border:   │          │
│ │           │   │   (RED)    │          │
│ └───────────┘   └────────────┘          │
│                                         │
└─────────────────────────────────────────┘
  0,240                           320,240
```

## Color Reference

### TFT_eSPI Color Constants Used

```cpp
// Background
TFT_BLACK         // #000000 - Screen background

// Text
TFT_WHITE         // #FFFFFF - Description text
TFT_DARKGREY      // #7BEF - Dim text (if needed)

// Warnings & Errors
TFT_RED           // #F800 - Headings, borders, danger
TFT_YELLOW        // #FFE0 - Warning triangle and text
TFT_MAROON        // #7800 - Reset button fill

// Cancel Button
TFT_NAVY          // #000F - Cancel button fill
TFT_BLUE          // #001F - Cancel button border
```

### Visual Color Legend

```
🔴 RED (TFT_RED)
   - Headings: "Reset Calibration to Factory", "Factory Reset"
   - Reset button borders
   - Indicates destructive action

🟡 YELLOW (TFT_YELLOW)
   - Warning triangle
   - Warning text: "This cannot be undone!"
   - Draws attention to caution messages

🔵 BLUE (TFT_NAVY/TFT_BLUE)
   - Cancel button (safe action)
   - Easy to distinguish from danger buttons

🟤 MAROON (TFT_MAROON)
   - Reset button backgrounds
   - Darker red to show "serious but not emergency"

⚪ WHITE (TFT_WHITE)
   - Description text
   - Button labels
   - Standard readable text

⚫ BLACK (TFT_BLACK)
   - Screen background
   - Creates high contrast
```

## Typography Scale

```
Heading (SIZE=2):
  Height: ~16px
  Use: "Reset Calibration to Factory", "Factory Reset"

Warning Icon (SIZE=3):
  Height: ~24px
  Use: "!" inside warning triangle

Body Text (SIZE=1):
  Height: ~8px
  Use: Description text, button labels
```

## Button Specifications

### Cancel Button
```
Position: (20, 190)
Size: 130 × 40 pixels
Fill: TFT_NAVY (#000F)
Border: TFT_BLUE (#001F)
Text: "Cancel" (SIZE=1, WHITE, Centered)
```

### Reset Calibration Button
```
Position: (170, 190)
Size: 130 × 40 pixels
Fill: TFT_MAROON (#7800)
Border: TFT_RED (#F800)
Text: "Reset" + "Calibration" (SIZE=1, WHITE, Centered)
  Line 1 (Y offset -5): "Reset"
  Line 2 (Y offset +5): "Calibration"
```

### Factory Reset Button
```
Position: (170, 190)
Size: 130 × 40 pixels
Fill: TFT_MAROON (#7800)
Border: TFT_RED (#F800)
Text: "Factory" + "Reset" (SIZE=1, WHITE, Centered)
  Line 1 (Y offset -5): "Factory"
  Line 2 (Y offset +5): "Reset"
```

## Warning Triangle Construction

```
Triangle vertices (Calibration):
  Top: (160, 75)
  Bottom-Left: (135, 115)
  Bottom-Right: (185, 115)

Triangle vertices (Factory):
  Top: (160, 65)
  Bottom-Left: (135, 105)
  Bottom-Right: (185, 105)

! Symbol:
  Position: Center of triangle (160, ~85 or ~75)
  Size: 3
  Color: YELLOW
```

## Touch Zones (Hitboxes)

```
Cancel Button:
  X: 20 to 150 (130px wide)
  Y: 190 to 230 (40px tall)

Reset Button:
  X: 170 to 300 (130px wide)
  Y: 190 to 230 (40px tall)

Gap between buttons: 20 pixels
Margins: 20 pixels from edges
```

## Text Centering

Both screens use `setTextDatum(TC_DATUM)` for top-center alignment:

```cpp
display->setTextDatum(TC_DATUM);
display->drawString("Text", SCREEN_WIDTH / 2, Y_position);
```

This centers text horizontally at X=160 (screen width / 2).

After drawing centered text, reset to default:
```cpp
display->setTextDatum(TL_DATUM);  // Top-Left default
```

## Screen Flow Diagram

```
        Settings Screen
               │
      ┌────────┴────────┐
      │                 │
      ▼                 ▼
CalibrationReset   FactoryReset
     Screen           Screen
      │                 │
  [Cancel]         [Cancel]
      │                 │
      └────────┬────────┘
               │
               ▼
        Settings Screen
               │
      [Reset Calibration]
               │
     settings->resetCalibration()
     settings->save()
               │
               ▼
        Settings Screen
               │
      [Factory Reset]
               │
     settings->factoryReset()
     settings->save()
               │
               ▼
        Settings Screen
```

## Accessibility Notes

1. **High Contrast**: Red buttons on black background highly visible
2. **Large Touch Targets**: 130×40px buttons easy to tap
3. **Clear Visual Hierarchy**: Size and color guide attention
4. **Redundant Warnings**: Triangle + text + button color all warn
5. **Safe Default**: Cancel button on left (standard position)

## Responsive Considerations

Works on 320×240 screen (ILI9341):
- ✅ All text fits without scrolling
- ✅ Buttons don't overlap
- ✅ Touch zones have adequate spacing
- ✅ Warning triangle visible and recognizable
- ✅ All elements centered or properly aligned

## Print Reference for Development

```
Quick Copy-Paste Coordinates:

CalibrationResetScreen:
  Heading Y: 20, 40
  Triangle: (160,75), (135,115), (185,115)
  ! Symbol: (160, 85)
  Text Y: 130, 145
  Cancel: (20,190,130,40)
  Reset: (170,190,130,40)

FactoryResetScreen:
  Heading Y: 30
  Triangle: (160,65), (135,105), (185,105)
  ! Symbol: (160, 75)
  Text Y: 120, 135, 160
  Cancel: (20,190,130,40)
  Reset: (170,190,130,40)
```
