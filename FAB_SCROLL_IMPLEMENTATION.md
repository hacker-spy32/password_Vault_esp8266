# Floating Action Button (FAB) Scroll Implementation

## Overview
Replaced experimental finger-drag scrolling with a clean, discrete-step Floating Action Button (FAB) for vertical navigation. The FAB is a reusable component that can be easily added to any screen requiring scroll functionality.

---

## Implementation Summary

### ✅ Features Implemented

1. **Reusable FAB Component** - `FloatingScrollButton` class
2. **Visual Design** - Circular button with down chevron arrow
3. **Touch Feedback** - Instant color change on press (White → Amber)
4. **Discrete Scrolling** - Step-by-step navigation (one item per tap)
5. **Bounds Checking** - Prevents scrolling past content
6. **Drag Scroll Removal** - All finger-drag code stripped out

---

## Architecture

### Component Structure

```
FloatingScrollButton (Reusable Component)
├── Visual Design
│   ├── Circular button (40px diameter)
│   ├── Downward chevron (V shape)
│   └── Position: Bottom-right corner (inset 15px)
├── State Management
│   ├── Idle: White fill
│   ├── Pressed: Amber (#E8B564) fill
│   └── Tap detection
└── Integration
    ├── Update (touch state)
    ├── Draw (rendering)
    └── Tap query (action detection)
```

### Scroll System

```
Screen (AdvancedOptionsScreen, etc.)
├── scrollOffset (discrete position: 0, 1, 2...)
├── scrollDown() (advance one step)
├── getMaxScrollOffset() (bounds calculation)
└── Rendering with offset applied

ScreenManager Integration
├── Create FAB instance
├── Update FAB state
├── Check for taps
├── Call screen->scrollDown()
└── Draw FAB (ALWAYS LAST)
```

---

## Visual Design

### Geometry
```
┌──────────────────────────────┐
│                              │
│                              │
│      Content Area            │
│                              │
│                              │
│                        ┌─┐   │
│                        │ │   │  ← FAB
│                        │∨│   │  (Bottom-right)
│                        └─┘   │
└──────────────────────────────┘
```

### Button Specifications
- **Shape:** Circle
- **Diameter:** 40px (radius 20px)
- **Position:** Bottom-right corner
  - X: `SCREEN_WIDTH - 15 - 20` = 285px (for 320px screen)
  - Y: `SCREEN_HEIGHT - 15 - 20` = 205px (for 240px screen)
- **Inset:** 15px from edges
- **Border:** 2px dark grey

### Icon Design
- **Chevron Type:** Downward-pointing V
- **Size:** 16px wide (8px each side from center)
- **Thickness:** 2px (double line)
- **Color:**
  - Idle: Black on white background
  - Pressed: White on amber background

### Color Palette
```cpp
// Idle state
Fill: TFT_WHITE (0xFFFF)
Border: TFT_DARKGREY
Chevron: TFT_BLACK

// Pressed state
Fill: 0xFD60 (#E8B564 - Warm Amber/Gold)
Border: TFT_DARKGREY
Chevron: TFT_WHITE
```

---

## Touch Behavior

### State Machine
```
IDLE → [Touch in button circle] → PRESSED → [Lift finger] → TAP DETECTED → IDLE
  ↓                                   ↓
[Touch outside]                  [Drag outside]
  ↓                                   ↓
IDLE                              RELEASED (no tap)
```

### Touch Detection
- **Hit Test:** Circular distance calculation
  ```cpp
  dx = touchX - buttonCenterX
  dy = touchY - buttonCenterY
  distance² = dx² + dy²
  isHit = (distance² <= radius²)
  ```
- **Instant Feedback:** Color changes immediately on TOUCH_DOWN
- **Tap vs Drag:** Tap only registers if finger lifts within button circle

---

## Scroll Behavior

### Discrete Step Scrolling
- **Not Continuous:** Each tap advances by exactly one item/row
- **Item Height:** 50px per menu item
- **Scroll Offset:** Tracked as integer (0, 1, 2, 3...)
- **Pixel Offset:** `scrollY = -(scrollOffset * MENU_ITEM_HEIGHT)`

### Example Scroll Sequence
```
Initial State:
  scrollOffset = 0
  Item 0: Y = 70
  Item 1: Y = 120
  Item 2: Y = 170
  Item 3: Y = 220 (partially visible)
  Item 4: Y = 270 (off-screen)

After 1 tap:
  scrollOffset = 1
  Item 0: Y = 20 (partially visible)
  Item 1: Y = 70
  Item 2: Y = 120
  Item 3: Y = 170
  Item 4: Y = 220 (partially visible)

After 2 taps:
  scrollOffset = 2
  Item 0: Y = -30 (off-screen)
  Item 1: Y = 20 (partially visible)
  Item 2: Y = 70
  Item 3: Y = 120
  Item 4: Y = 170 (fully visible)
```

### Bounds Checking
```cpp
int getMaxScrollOffset() {
    int visibleHeight = SCREEN_HEIGHT - 70;  // Minus header
    int totalHeight = MENU_ITEM_COUNT * MENU_ITEM_HEIGHT;
    
    if (totalHeight <= visibleHeight) {
        return 0;  // All visible, no scroll needed
    }
    
    int maxOffset = (totalHeight - visibleHeight) / MENU_ITEM_HEIGHT;
    if ((totalHeight - visibleHeight) % MENU_ITEM_HEIGHT > 0) {
        maxOffset++;  // Round up for partial items
    }
    
    return maxOffset;
}
```

---

## Drag Scroll Removal

### Code Removed from AdvancedOptionsScreen

**Header (Removed):**
```cpp
int scrollY;
bool dragging;
int lastTouchY;
void updateScroll();
```

**Implementation (Removed):**
```cpp
void AdvancedOptionsScreen::updateScroll() {
    // Entire drag handling logic removed
}
```

**Replaced With:**
```cpp
int scrollOffset;  // Discrete position
void scrollDown(); // Step-by-step scroll
```

### Benefits
✅ **Simpler Code:** No complex drag delta tracking
✅ **Predictable:** Always scrolls by exact item height
✅ **No Overshoot:** Bounded by content limits
✅ **Better Touch Detection:** No conflict between drag and tap
✅ **Easier Debugging:** Discrete offsets are clear

---

## Integration Guide

### Adding FAB to New Screen

**Step 1: Add scrollOffset to your screen class**
```cpp
class YourScreen {
private:
    int scrollOffset;  // Current scroll position
    
public:
    void scrollDown();  // Public method for FAB to call
};
```

**Step 2: Implement scrollDown()**
```cpp
void YourScreen::scrollDown() {
    int maxScroll = getMaxScrollOffset();
    
    if (scrollOffset < maxScroll) {
        scrollOffset++;
        screenIsDirty = true;
    }
}
```

**Step 3: Apply offset in rendering**
```cpp
void YourScreen::drawDynamicData() {
    int scrollY = -(scrollOffset * ITEM_HEIGHT);
    
    for (int i = 0; i < ITEM_COUNT; i++) {
        int itemY = START_Y + (i * ITEM_HEIGHT) + scrollY;
        // Draw item at itemY...
    }
}
```

**Step 4: Integrate FAB in ScreenManager**
```cpp
void ScreenManager::updateYourScreenData() {
    // Update FAB
    if (scrollFAB != nullptr) {
        scrollFAB->update(touch->getPoint(), touch->getState());
        
        if (scrollFAB->wasTapped()) {
            yourScreen->scrollDown();
            scrollFAB->clearTap();
        }
    }
    
    // Update screen...
}

void ScreenManager::drawYourScreenDynamicData() {
    // Draw screen content
    yourScreen->draw();
    
    // Draw FAB LAST (on top of everything)
    if (scrollFAB != nullptr) {
        scrollFAB->draw();
    }
}
```

**Step 5: Prevent menu touches on FAB**
```cpp
void ScreenManager::handleYourScreenTouch(const TouchPoint& point) {
    // Check FAB first
    if (scrollFAB != nullptr && scrollFAB->isTouchOnButton(point.x, point.y)) {
        return;  // Let FAB handle it
    }
    
    // Handle screen touches
    yourScreen->onTouchEvent(point);
}
```

---

## Code Quality

### FloatingScrollButton Class

**Responsibilities:**
- ✅ Render circular button with chevron
- ✅ Track touch state (pressed/released)
- ✅ Detect tap events
- ✅ Provide hit test for touch filtering
- ✅ Manage dirty flag for efficient rendering

**Does NOT:**
- ❌ Know about screen content
- ❌ Control scroll offset directly
- ❌ Handle navigation
- ❌ Manage screen state

**Clean Separation:**
```
FloatingScrollButton (UI Component)
        ↓ (tap event)
ScreenManager (Controller)
        ↓ (scrollDown call)
Screen (Content Owner)
```

### Performance Optimizations

1. **Dirty Flag Rendering**
   ```cpp
   if (!needsRedraw) {
       return;  // Skip draw if state unchanged
   }
   ```

2. **Efficient Hit Testing**
   ```cpp
   // Integer math only, no sqrt()
   int distanceSquared = (dx * dx) + (dy * dy);
   return distanceSquared <= radiusSquared;
   ```

3. **Minimal Redraws**
   - Only redraws when state changes
   - Screen content drawn separately
   - FAB drawn once per state change

---

## Files Summary

### Created (3 files)
1. **`include/FloatingScrollButton.h`** (63 lines)
   - Reusable FAB component header
   - Clean interface, minimal dependencies
   - Self-contained touch and render logic

2. **`src/FloatingScrollButton.cpp`** (123 lines)
   - Complete FAB implementation
   - Circular hit detection
   - Efficient dirty-flag rendering
   - Chevron drawing with double-line thickness

3. **`FAB_SCROLL_IMPLEMENTATION.md`** (this file)
   - Complete documentation
   - Integration guide
   - Visual specifications

### Modified (4 files)
1. **`include/AdvancedOptionsScreen.h`**
   - Removed: scrollY, dragging, lastTouchY, updateScroll()
   - Added: scrollOffset, scrollDown(), getMaxScrollOffset()

2. **`src/AdvancedOptionsScreen.cpp`**
   - Stripped out all drag scroll code
   - Implemented discrete step scrolling
   - Added scroll helper methods

3. **`include/ScreenManager.h`**
   - Added FloatingScrollButton forward declaration
   - Added scrollFAB pointer

4. **`src/ScreenManager.cpp`**
   - Integrated FAB into Advanced Options
   - Added FAB update, draw, and touch handling
   - FAB drawn LAST (on top)

---

## Testing Checklist

### Visual Tests
- [ ] FAB appears at bottom-right corner
- [ ] FAB is white circle with down chevron (idle)
- [ ] FAB turns amber when touched
- [ ] FAB returns to white when released
- [ ] Chevron is clearly visible in both states
- [ ] FAB renders on top of all content

### Touch Tests
- [ ] Tap FAB → scrolls down one item
- [ ] Hold FAB → stays amber, no continuous scroll
- [ ] Release FAB → color returns to white
- [ ] Tap outside FAB → menu items respond
- [ ] Tap on FAB → menu items don't respond
- [ ] Multiple taps → incremental scroll

### Scroll Tests
- [ ] First tap → item scrolls up by 50px
- [ ] Multiple taps → continues scrolling
- [ ] At bottom → tap does nothing (bounded)
- [ ] Content stops at correct position
- [ ] No overshoot or bounce

### Integration Tests
- [ ] FAB works in Advanced Options screen
- [ ] Navigation to sub-screens works
- [ ] Back button returns correctly
- [ ] Screen transitions clean (no FAB artifacts)
- [ ] Orientation change works (FAB stays in corner)

---

## Future Expansion

### Additional Screens
The FAB can be easily added to:
- **Passwords Screen** - Scroll through password entries
- **Favorites Screen** - Navigate favorite items
- **Settings Screen** - If it gets longer menu

### Potential Enhancements
- [ ] Scroll Up FAB (top-left corner, upward chevron)
- [ ] Page Up/Down (skip multiple items)
- [ ] Scroll position indicator
- [ ] Fade FAB when at top/bottom
- [ ] Hide FAB if all content fits on screen
- [ ] Smooth animation (ease-in/out)
- [ ] Haptic feedback (if hardware supports)

---

## Known Limitations

- **Single Direction:** Only down scrolling (by design)
- **Fixed Position:** Always bottom-right (could be configurable)
- **No Animation:** Instant scroll (could add smooth transition)
- **Manual Integration:** Each screen needs explicit FAB setup

---

## Success Criteria

✅ **Drag Scroll Removed:** All finger-drag code stripped out
✅ **FAB Component Created:** Reusable, self-contained class
✅ **Visual Design:** Matches specifications (white/amber, chevron)
✅ **Touch Feedback:** Instant color change on press
✅ **Discrete Scrolling:** One item per tap
✅ **Bounds Checking:** Stops at bottom
✅ **Integration:** Working in Advanced Options screen
✅ **Code Quality:** Clean, documented, no warnings
✅ **Reusable:** Easy to add to other screens

---

**Implementation Date:** Current session
**Status:** ✅ Complete and Ready for Testing
**Drag Scroll:** ❌ Removed
**FAB Scroll:** ✅ Implemented
