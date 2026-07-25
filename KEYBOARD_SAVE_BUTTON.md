# Keyboard Save Button Implementation

## Overview
Added a button bar below the custom ABCD touch keyboard with **Backspace** and **Save** buttons for quick actions without closing the keyboard.

## Visual Design
- **Position**: Below the keyboard (Y = keyboard bottom + 4px gap)
- **Layout**: Two buttons side by side
  - **Backspace Button (Left)**: 80px wide, coral (#FF6B6B), "< Del" text
  - **Save Button (Right)**: Remaining width (~224px), brass gradient (#E8B564), "Save" text
- **Gap**: 8px between buttons
- **Height**: 32px for both buttons
- **Margins**: 8px on each side of screen
- **Rounded corners**: 6px border-radius

## Layout Structure
```
┌─────────────────────────────────────┐
│  Suggestion Chips (24px)            │ Y=68
├─────────────────────────────────────┤
│  Keyboard Row 1: A-G (28px)         │ Y=92
│  Keyboard Row 2: H-N (28px)         │ Y=122
│  Keyboard Row 3: O-U (28px)         │ Y=152
│  Keyboard Row 4: Shift @ Space V-Z  │ Y=182
│                  [Backspace <]       │
├─────────────────────────────────────┤
│  [ < Del ]  [    SAVE BUTTON    ]   │ Y=216
│    (80px)       (~224px, brass)     │
└─────────────────────────────────────┘
Total height: 180px (was 144px)
```

## Button Bar Features

### Backspace Button (Left)
- **Width**: 80px (fixed)
- **Color**: Coral (#FF6B6B / 0xFB2D)
- **Text**: "< Del" (white ivory text)
- **Function**: Delete last character (same as keyboard backspace key)
- **Convenience**: Easier to reach than top-right keyboard backspace

### Save Button (Right)
- **Width**: Dynamic (~224px, fills remaining space)
- **Color**: Brass gradient (#E8B564 to #c99a4e)
- **Text**: "Save" (dark text #1a1305)
- **Function**: Saves password and exits immediately
- **Style**: Matches main form save button design

## Technical Implementation

### Files Modified
1. **include/TouchKeyboard.h**
   - Added `SAVE_BUTTON_HEIGHT` constant (32px)
   - Added `SAVE_BUTTON_MARGIN` constant (4px gap)
   - Updated `handleTouch()` documentation to include -5 return code

2. **src/TouchKeyboard.cpp**
   - Updated `draw()` method to render save button below keyboard
   - Modified `getKeyboardHeight()` to include save button height
   - Updated `handleTouch()` to detect save button taps (returns -5)

3. **src/AddPasswordManualScreen.cpp**
   - Added handling for keyCode == -5 in `onTouchEvent()`
   - Sets `passwordSaved = true` and `needsExit = true` when save button pressed

## Return Codes from Keyboard
- **-1**: Backspace (from keyboard key OR button bar)
- **-2**: Enter/Done (closes keyboard)
- **-3**: Space
- **-4**: Mode change (Shift toggle)
- **-5**: Save button pressed
- **0**: No input/invalid touch
- **1-255**: ASCII character input

## User Flow
1. User taps a field (Title, User, or Pass)
2. Keyboard appears with the focused field visible above it
3. User types using the ABCD keyboard layout
4. User can:
   - Tap **"< Del"** button → deletes last character (convenient alternative to keyboard backspace)
   - Tap **"Save"** button → saves and exits immediately
   - Use keyboard **backspace key** (top-right coral key)
   - Close keyboard (back button) → returns to form with Cancel/Save buttons
   - Tap "Cancel" button → exits without saving

## Build Results
- **Status**: ✓ SUCCESS
- **RAM Usage**: 39.4% (32,236 / 81,920 bytes)
- **Flash Usage**: 29.6% (309,287 / 1,044,464 bytes)
- **Warnings**: Minor initialization order warning (non-critical)

## Benefits
- **Faster workflow**: No need to close keyboard before saving
- **Touch-friendly**: Large 32px button is easy to tap
- **Consistent design**: Matches the brass primary button style from main form
- **Intuitive**: "Save" action is immediately visible while typing

## Color Tokens Used
- `COLOR_BRASS`: #E8B564 (0xFD60) - Button background
- `0xE4C0`: #c99a4e - Darker brass for gradient
- `0x10A2`: #1a1305 - Dark text on brass button

## Next Steps (Optional Enhancements)
- Add visual feedback (color change) when buttons are pressed
- Add validation before saving (check for empty required fields)
- Add success animation/feedback after save
- Consider haptic feedback for button presses (if hardware supports it)
- Add "Done" button option to close keyboard without saving

## Build Results
- **Status**: ✓ SUCCESS
- **RAM Usage**: 39.4% (32,236 / 81,920 bytes)
- **Flash Usage**: 29.6% (309,391 / 1,044,464 bytes)
- **Warnings**: Minor initialization order warning (non-critical)

## Benefits
- **Faster workflow**: No need to close keyboard before saving
- **Convenient backspace**: Large button bar backspace is easier to reach than keyboard key
- **Touch-friendly**: 32px tall buttons are easy to tap
- **Dual backspace options**: Keyboard key (top-right) + button bar (bottom-left)
- **Consistent design**: Matches the design system (brass for primary, coral for delete)
- **Intuitive**: Action buttons immediately visible while typing

## Color Tokens Used
- `COLOR_BRASS`: #E8B564 (0xFD60) - Save button background
- `COLOR_CORAL`: #FF6B6B (0xFB2D) - Backspace button background
- `COLOR_IVORY`: #F4EFE4 (0xFFDE) - White text on buttons
- `0xE4C0`: #c99a4e - Darker brass for gradient
- `0x10A2`: #1a1305 - Dark text on brass button
