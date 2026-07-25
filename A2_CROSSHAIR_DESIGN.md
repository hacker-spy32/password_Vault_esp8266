# A2 Calibration Crosshair Design - Implementation Complete

## Visual Design

```
                    |
                    |
                 ───|───
              ────  |  ────
           ────     |     ────
                    |
        ───── ┌─────────┐ ─────
              │  ╔═══╗  │
              │  ║ ● ║  │  ← Red filled center (8px radius)
              │  ╚═══╝  │
        ───── └─────────┘ ─────
                    |
           ────     |     ────
              ────  |  ────
                 ───|───
                    |
                    |
```

## Component Breakdown

### 1. Outer White Ring (Target)
- Double circle for visibility
- Radius: 19px and 20px
- Color: `COLOR_TEXT` (White)
- Purpose: Provides contrast against any background

### 2. Inner Colored Circle
- Outer ring: 15px radius (unfilled)
- Inner fill: 8px radius (solid)
- Color: `COLOR_CROSSHAIR_RED` (Red normally, Green on flash)
- Purpose: Main targeting area

### 3. Crosshair Lines (Extending)
- Length: 8px each (from 22px to 30px from center)
- Positioned outside the white ring
- Color: Matches inner circle color
- Purpose: Precision alignment guides

## Exact Implementation

```cpp
void CalibrationWizardScreen::drawCalibrationPoint() {
    int16_t x, y;
    getPointCoordinates(currentPoint, x, y);
    
    uint16_t crosshairColor = COLOR_CROSSHAIR_RED;  // Red (or Green for flash)
    
    // Outer circle (Target ring) - White
    display->drawCircle(x, y, 20, COLOR_TEXT);
    display->drawCircle(x, y, 19, COLOR_TEXT);
    
    // Inner circle (Solid center) - Colored
    display->drawCircle(x, y, 15, crosshairColor);
    display->fillCircle(x, y, 8, crosshairColor);
    
    // Crosshair lines (Extending outwards from the circle)
    display->drawLine(x - 30, y, x - 22, y, crosshairColor);  // Left
    display->drawLine(x + 22, y, x + 30, y, crosshairColor);  // Right
    display->drawLine(x, y - 30, x, y - 22, crosshairColor);  // Top
    display->drawLine(x, y + 22, x, y + 30, crosshairColor);  // Bottom
}
```

## Dimensions Reference

```
Total width/height: 60px (30px from center each direction)

     ← 30px →
     ┌──────────────┐
  ↑  │   ← 8px →    │
30px │   ────────   │  ← Crosshair line (outside white ring)
  ↓  │     ╔══╗     │
     │     ║  ║     │  ← 8px filled circle (red/green)
     │     ╚══╝     │
     │   ────────   │
     └──────────────┘

Gap between rings and lines: 2px (lines start at 22px, ring ends at 20px)
```

## Color States

### Normal Calibration (Red)
- User sees: Red center with white ring
- Meaning: "Tap here to calibrate"

### Point Flash (Green) - Future Implementation
- User sees: Green center with white ring
- Meaning: "Point accepted, moving to next"
- Duration: ~200ms flash

## Screen Layout During Calibration

```
┌─────────────────────────────────────┐
│ (30,30)                             │
│    ◎                                │  Point 0 - Top Left
│                                     │
│                                     │
│                                     │
│                                     │
│              (160,120)              │
│                 ◎                   │  Point 4 - Center
│                                     │
│                                     │
│                                     │
│                                     │
│                              ◎      │  Point 2 - Bottom Right
│                         (290,210)   │
└─────────────────────────────────────┘
```

## Why This Design Works

1. **High Contrast**: White ring visible on any background
2. **Precise Center**: 8px filled circle is large enough to see but small for accuracy
3. **Clear Targeting**: Extending lines help align finger/stylus
4. **Professional Look**: Circular design is more polished than simple crosshair
5. **Color Feedback**: Red/Green states provide instant visual confirmation
6. **Proven UX**: Successfully used in A2 project

## Comparison to Previous Design

### OLD (Simple Crosshair)
```
    |
    |
────┼────
    |
    |
```
- Just two intersecting lines
- Hard to see in corners
- No depth or targeting guidance

### NEW (A2 Circular Target)
```
      |
   ───┼───
  ╔═══╪═══╗
  ║   |   ║
──╫───●───╫──
  ║   |   ║
  ╚═══╪═══╝
   ───┼───
      |
```
- Multi-layer targeting reticle
- High visibility
- Professional appearance
- Better user guidance

## Implementation Status

✅ **Crosshair Drawing** - A2 circular target implemented
✅ **Clean Calibrating State** - No text, just crosshair
✅ **Color System** - Red/Green colors defined and ready
✅ **Exact Positioning** - MARGIN-based corner placement
✅ **Intro Screen** - Unchanged and perfect
⏳ **Flash State** - Color logic ready, timing to be added

## Next Steps (Optional Enhancement)

To add the green flash feedback:

```cpp
// In collectSample(), after point completion:
if (samplesAtCurrentPoint >= SAMPLES_PER_POINT) {
    // ... store calibration data ...
    
    // Flash green briefly
    currentState = CAL_POINT_FLASH;
    screenIsDirty = true;
    draw();  // Force immediate redraw with green
    delay(200);  // Hold green for 200ms
    
    // Advance to next point
    advanceToNextPoint();
}
```
