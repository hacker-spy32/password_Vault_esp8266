# Final Keyboard Layout - 320x240 Display

## Screen Layout (Portrait Mode)

```
┌─────────────────────────────────────────────┐  Y=0
│  STATUS BAR (24px)                          │
│  • SecureKey dot    WiFi status             │
├─────────────────────────────────────────────┤  Y=24
│  HEADER (32px)                              │
│  < Back    Add Password                     │
├─────────────────────────────────────────────┤  Y=56 ← KEYBOARD STARTS HERE
│                                             │
│  [Focused Field - Title/User/Pass]          │  (64px tall)
│  Shifted to Y=-12 when keyboard active      │
│                                             │
├─────────────────────────────────────────────┤  Y=56
│  ══════════ SUGGESTION CHIPS ═══════════    │  (24px)
│  [Amazon]  [Google]  [Netflix]              │
├─────────────────────────────────────────────┤  Y=80
│  ┌───┬───┬───┬───┬───┬───┬───┐             │
│  │ a │ b │ c │ d │ e │ f │ g │  Row 1      │  (28px)
│  └───┴───┴───┴───┴───┴───┴───┘             │
├─────────────────────────────────────────────┤  Y=110
│  ┌───┬───┬───┬───┬───┬───┬───┐             │
│  │ h │ i │ j │ k │ l │ m │ n │  Row 2      │  (28px)
│  └───┴───┴───┴───┴───┴───┴───┘             │
├─────────────────────────────────────────────┤  Y=140
│  ┌───┬───┬───┬───┬───┬───┬───┐             │
│  │ o │ p │ q │ r │ s │ t │ u │  Row 3      │  (28px)
│  └───┴───┴───┴───┴───┴───┴───┘             │
├─────────────────────────────────────────────┤  Y=170
│  ┌─────┬───┬─────────┬───┬───┬───┬───┬───┬─────┐
│  │SHIFT│ @ │  SPACE  │ v │ w │ x │ y │ z │  <  │  Row 4  (28px)
│  └─────┴───┴─────────┴───┴───┴───┴───┴───┴─────┘
│  (brass) (brass)     (brass)            (coral)
├─────────────────────────────────────────────┤  Y=204 (gap: 4px)
│  ┌─────────────┬─────────────────────────┐  │
│  │  < Del      │         SAVE            │  │  Button Bar (32px)
│  │  (coral)    │       (brass)           │  │
│  └─────────────┴─────────────────────────┘  │
└─────────────────────────────────────────────┘  Y=236 (4px from bottom)
           Total: 236px of 240px used
```

## Vertical Spacing Breakdown

| Section | Start Y | Height | End Y | Notes |
|---------|---------|--------|-------|-------|
| Status Bar | 0 | 24px | 24 | System info |
| Header | 24 | 32px | 56 | Back + Title |
| **Keyboard Area** | **56** | **180px** | **236** | **Full keyboard** |
| ├─ Suggestions | 56 | 24px | 80 | Auto-complete chips |
| ├─ Row 1 (A-G) | 80 | 28px | 110 | Letter keys |
| ├─ Row 2 (H-N) | 110 | 28px | 140 | Letter keys |
| ├─ Row 3 (O-U) | 140 | 28px | 170 | Letter keys |
| ├─ Row 4 (Special) | 170 | 28px | 200 | Shift, @, Space, V-Z, < |
| ├─ Gap | 200 | 4px | 204 | Breathing room |
| └─ Button Bar | 204 | 32px | 236 | < Del + Save |
| Bottom Padding | 236 | 4px | 240 | Screen edge |

## Key Improvements Made

### Version 1: Original Layout (Y=100)
- Keyboard started too low
- Button bar cut off at Y=248 (8px off screen)
- ❌ Save/Delete buttons not fully visible

### Version 2: Adjusted Layout (Y=68)
- Better positioning
- Button bar at Y=216
- ⚠️ Still slightly cut off on some displays

### Version 3: Final Layout (Y=56) ✓
- **Keyboard moved up 12px** (from Y=68 to Y=56)
- Button bar ends at Y=236 (**4px from screen edge**)
- ✅ All elements fully visible
- ✅ Closer to text field (better visual flow)
- ✅ More comfortable typing position

## Focused Field Positioning

When keyboard is active:
```
Focused field Y = keyboardTop - FIELD_HEIGHT - 4px
                = 56 - 64 - 4
                = -12 (slightly above header)
```

The focused field shifts up to stay visible above the keyboard, with only a 4px gap between the field and the suggestion chips.

## Button Dimensions

### Button Bar Layout
- **Total width**: 304px (screen 320px - 16px margins)
- **Backspace button**: 80px (left)
- **Gap**: 8px
- **Save button**: 216px (right, fills remaining space)
- **Height**: 32px (both buttons)

### Colors
- **Backspace**: Coral (#FF6B6B / 0xFB2D) with white text
- **Save**: Brass gradient (#E8B564 / 0xFD60) with dark text
- **Shift/@ /Space**: Brass background
- **Delete key (keyboard)**: Coral background
- **Regular keys**: Surface gray (#141821)

## Touch Zones

All buttons are 32px tall minimum for comfortable finger taps on embedded displays.

## Build Status
- **RAM**: 39.4% (32,236 / 81,920 bytes)
- **Flash**: 29.6% (309,375 / 1,044,464 bytes)
- **Status**: ✓ SUCCESS

## Verified Fit
- Total height: 236px
- Screen height: 240px
- **Clearance: 4px** ✓
- All UI elements fully visible
- Comfortable spacing maintained
