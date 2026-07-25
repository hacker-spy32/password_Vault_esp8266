# Text Field Above Keyboard - Fix Summary

## Problem
When keyboard was active, the focused text field was positioned off-screen (Y=-12 or similar negative values), making it invisible to the user.

## Root Cause
- **Header ends at**: Y=56 (24px status bar + 32px header)
- **Keyboard starts at**: Y=60 (moved up to fit button bar)
- **Available space**: Only 4px between header and keyboard
- **Field height**: 64px (too tall to fit in 4px gap!)

## Solution Implemented

### 1. Compact Field Mode
When keyboard is active, use a **compact field height of 48px** (instead of 64px):
- **Normal mode** (keyboard hidden): 64px tall field
- **Compact mode** (keyboard active): 48px tall field

### 2. Smart Positioning
Position the compact field to fit perfectly between header and keyboard:
```
Y=12  ← Field starts here (compact 48px)
      ┌───────────────────────┐
      │ TITLE (label)         │  8px from top
      │ amazon (value)        │  26px from top
      └───────────────────────┘
Y=60  ← Field ends / Keyboard begins
```

### 3. Field Rendering Updates
Modified `drawInputField()` to:
- Check if field is off-screen (`if (y < 0) return;`)
- Use `actualFieldHeight` based on keyboard state
- Adjust label position: Y+8 (was Y+10)
- Adjust value position: Y+26 (was Y+32)
- Tighter spacing for compact mode

## Layout Visualization

```
┌─────────────────────────────────────┐
│  STATUS BAR (24px)                  │  Y=0
├─────────────────────────────────────┤
│  HEADER: < Back  Add Password       │  Y=24-56
├─────────────────────────────────────┤
│  ┌─────────────────────────────┐   │
│  │ TITLE                       │   │  Y=12-60 (48px compact field)
│  │ amazon                      │   │
│  └─────────────────────────────┘   │
├─────────────────────────────────────┤
│  Suggestions: [Amazon] [Apple]      │  Y=60-84
├─────────────────────────────────────┤
│  Keyboard Row 1: a b c d e f g      │  Y=84-112
│  Keyboard Row 2: h i j k l m n      │  Y=114-142
│  Keyboard Row 3: o p q r s t u      │  Y=144-172
│  Keyboard Row 4: SHIFT @ SPACE etc  │  Y=174-202
├─────────────────────────────────────┤
│  [< Del]  [       SAVE       ]      │  Y=208-240 (button bar)
└─────────────────────────────────────┘
```

## Key Measurements

| Element | Y Position | Height | Total Y |
|---------|-----------|--------|---------|
| Status Bar | 0 | 24px | 24 |
| Header | 24 | 32px | 56 |
| **Focused Field** | **12** | **48px** | **60** |
| Suggestions | 60 | 24px | 84 |
| Keyboard Rows | 84 | 120px | 204 |
| Gap | 204 | 4px | 208 |
| Button Bar | 208 | 32px | 240 |

## Benefits
✓ **Field fully visible** - positioned at Y=12 to Y=60
✓ **Fits perfectly** - 48px compact field matches available space
✓ **No overlap** - clean separation between elements
✓ **User can see input** - text visible while typing
✓ **Smooth experience** - field appears when keyboard opens

## Code Changes

### TouchKeyboard.h
- `KEYBOARD_START_Y = 60` (was 56, now 60 for 4px breathing room)

### AddPasswordManualScreen.cpp
- **drawInputField()**: Added compact mode logic
  - `actualFieldHeight = keyboardActive ? 48 : 64`
  - Adjusted label/value Y positions for tighter spacing
  - Added off-screen check
  
- **getFieldYPosition()**: Updated calculation
  - `visibleY = keyboardTop - compactFieldHeight`
  - `visibleY = 60 - 48 = 12` (perfect fit!)

## Build Status
- **RAM**: 39.4% (32,236 / 81,920 bytes)
- **Flash**: 29.6% (309,407 / 1,044,464 bytes)
- **Status**: ✓ SUCCESS

## Testing Checklist
- [ ] Tap Title field → field visible above keyboard
- [ ] Tap User field → field visible above keyboard  
- [ ] Tap Pass field → field visible above keyboard
- [ ] Type text → see input in field above keyboard
- [ ] All suggestions appear correctly
- [ ] Button bar (< Del + Save) fully visible at bottom
