# Settings Screen Complete Redesign

## Overview
Completely redesigned the Settings screen from separate boxes to a clean, unified list-row layout with modern design system compliance and new features.

## New Features Implemented

### 1. Auto-Lock Setting
**Description:** Screen timeout with dropdown selector  
**Default:** 15 seconds  
**Options:** 5s, 10s, 15s, 20s, 30s, Never  
**UI:** Value displayed on right with dropdown chevron (v)  
**Interaction:** Tap right side to cycle through options

### 2. Two Tap Sleep
**Description:** Toggle feature for double-tap sleep functionality  
**Default:** Disabled  
**UI:** Toggle switch on right side of row  
**Interaction:** Tap switch to enable/disable

### 3. Advanced Options
**Description:** Navigate to Advanced Options menu  
**UI:** Right chevron (>) indicator  
**Navigation:** Settings → Advanced Options → (back) → Settings

### 4. About Screen
**Description:** Project information and credits  
**Content:**
- Project name: SecureKey
- Version: 1.0.0
- Description: Secure password manager for embedded devices
- Author credit: "Master"
- Copyright notice

## Design System Compliance

### Colors Used
```cpp
COLOR_INK     0x0841  // #0A0C10 - Background
COLOR_SURFACE 0x18C3  // #141821 - Card surfaces
COLOR_LINE    0x2945  // #262C3A - Borders/separators
COLOR_BRASS   0xFD60  // #E8B564 - Accents/values
COLOR_STEEL   0x7BCF  // #778899 - Steel accent (for switches)
COLOR_IVORY   0xFFDE  // #F4EFE4 - Primary text
COLOR_MUTED   0x8C51  // #8A8FA3 - Secondary text/icons
```

### Layout Specifications
```
┌─────────────────────────────────┐
│ ● Settings                      │  Header (56px)
├─────────────────────────────────┤
│ ┌─────────────────────────────┐ │
│ │ Auto-Lock          15s  v   │ │  Row 1 (60px)
│ └─────────────────────────────┘ │
│                                 │  Spacing (2px)
│ ┌─────────────────────────────┐ │
│ │ Two Tap Sleep      [◯────]  │ │  Row 2 (60px)
│ └─────────────────────────────┘ │
│                                 │  Spacing (2px)
│ ┌─────────────────────────────┐ │
│ │ Advanced                 >  │ │  Row 3 (60px)
│ └─────────────────────────────┘ │
│                                 │  Spacing (2px)
│ ┌─────────────────────────────┐ │
│ │ About                    >  │ │  Row 4 (60px)
│ └─────────────────────────────┘ │
└─────────────────────────────────┘
```

## Files Created

### 1. include/SettingsListScreen.h
**Purpose:** Header for new list-based Settings screen  
**Key Components:**
- `SettingsRow` enum (AUTO_LOCK, TWO_TAP_SLEEP, ADVANCED, ABOUT)
- `AutoLockTimeout` enum (5s, 10s, 15s, 20s, 30s, Never)
- Toggle switch drawing method
- Dropdown indicator drawing method

### 2. src/SettingsListScreen.cpp
**Purpose:** Implementation of Settings list screen  
**Size:** ~350 lines  
**Features:**
- List-row rendering with rounded rectangles
- Toggle switch (48×24px with animated knob)
- Dropdown chevron indicator
- Touch detection for rows, switches, and dropdowns
- Auto-lock timeout cycling logic
- Two-tap sleep toggle logic

### 3. include/AboutScreen.h
**Purpose:** Header for About screen  
**Components:**
- Simple screen with text content
- Back button navigation

### 4. src/AboutScreen.cpp
**Purpose:** About screen implementation  
**Size:** ~150 lines  
**Content Layout:**
```
SecureKey (brass, large)
Version 1.0.0 (muted)
─────────────────────
A secure password manager
for embedded devices.

Featuring offline-first
encrypted storage, intuitive
touch interface, and modern
design system aesthetics.
─────────────────────
Created by
Master (brass)
© 2024 All Rights Reserved
```

## Files Modified

### include/ScreenManager.h
**Changes:**
- Added `SCREEN_ABOUT` enum
- Added forward declarations: `SettingsListScreen`, `AboutScreen`
- Added member variables: `settingsListScreen`, `aboutScreen`
- Added method declarations for About screen

### src/ScreenManager.cpp
**Changes:**
- Added includes for new screen classes
- Updated constructor initialization
- Added `SCREEN_ABOUT` to `getScreenName()`
- Replaced old Settings implementation with new list-based version
- Added Settings to self-managing screens list
- Implemented About screen methods (~40 lines)
- Updated navigation: About returns to Settings

## UI Component Specifications

### Toggle Switch
**Size:** 48×24 pixels  
**States:**
- **OFF:** Gray track, knob on left
- **ON:** Brass track, knob on right

**Visual:**
```
OFF:  [◯────────]  (gray/muted)
ON:   [────────◯]  (brass/golden)
```

**Drawing:**
```cpp
void drawToggleSwitch(int x, int y, bool enabled) {
    // Track (rounded rectangle)
    uint16_t trackColor = enabled ? COLOR_BRASS : COLOR_MUTED;
    tft->fillRoundRect(x, y, 48, 24, 12, trackColor);
    
    // Knob (circle, 10px radius)
    int knobX = enabled ? (x + 38) : (x + 10);
    tft->fillCircle(knobX, y + 12, 10, COLOR_IVORY);
}
```

### Dropdown Indicator
**Size:** 12×8 pixels (V shape)  
**Color:** COLOR_MUTED  
**Position:** Right side of row

**Visual:**
```
  ╲ ╱
   V
```

**Drawing:**
```cpp
void drawDropdownIndicator(int x, int y) {
    // Two lines forming V shape
    tft->drawLine(x - 6, y - 3, x, y + 3, COLOR_MUTED);
    tft->drawLine(x, y + 3, x + 6, y - 3, COLOR_MUTED);
}
```

### Settings Row
**Height:** 60 pixels  
**Spacing:** 2 pixels between rows  
**Margin:** 16 pixels on sides  
**Border Radius:** 8 pixels  

**Structure:**
```
┌─────────────────────────────────┐
│ Label (left)        Value/Icon  │
│ (Ivory, size 2)     (Brass/Icon)│
└─────────────────────────────────┘
```

## Navigation Flow

### Settings Screen Navigation
```
HOME → Settings
  ↓ Tap Auto-Lock dropdown
  Cycles: 5s → 10s → 15s → 20s → 30s → Never → 5s...
  
  ↓ Tap Two Tap Sleep toggle
  Toggles: OFF ⟷ ON
  
  ↓ Tap Advanced
  Settings → Advanced Options → (back) → Settings
  
  ↓ Tap About
  Settings → About → (back) → Settings
  
  ↓ Tap Back
  Settings → HOME
```

### About Screen Navigation
```
Settings → About
  ↓ Tap Back
About → Settings
```

## Touch Detection Logic

### Row Detection
```cpp
SettingsRow getRowAtPosition(int16_t x, int16_t y) {
    // Calculate row index from Y position
    int relativeY = y - ROWS_START_Y;
    int rowIndex = relativeY / (ROW_HEIGHT + ROW_SPACING);
    
    // Map to enum
    switch (rowIndex) {
        case 0: return SETTINGS_ROW_AUTO_LOCK;
        case 1: return SETTINGS_ROW_TWO_TAP_SLEEP;
        case 2: return SETTINGS_ROW_ADVANCED;
        case 3: return SETTINGS_ROW_ABOUT;
    }
}
```

### Toggle Switch Hit Detection
```cpp
bool isToggleSwitchPressed(int x, int y, SettingsRow row) {
    // Switch is 48×24px on right side
    int toggleX = SIDE_MARGIN + rowWidth - 60;
    int toggleY = rowY + (ROW_HEIGHT - 24) / 2;
    
    return (x >= toggleX && x <= toggleX + 48 &&
            y >= toggleY && y <= toggleY + 24);
}
```

### Dropdown Hit Detection
```cpp
bool isDropdownPressed(int x, int y, SettingsRow row) {
    // Right 80px of row is dropdown area
    int dropdownX = SIDE_MARGIN + rowWidth - 80;
    
    return (x >= dropdownX && x <= SIDE_MARGIN + rowWidth &&
            y >= rowY && y <= rowY + ROW_HEIGHT);
}
```

## State Management

### Auto-Lock Timeout
**Storage:** `uint8_t autoLockTimeout` (in seconds)  
**Default:** 15  
**Values:** 5, 10, 15, 20, 30, 0 (0 = Never)  
**Persistence:** TODO - Save to SettingsManager

### Two Tap Sleep
**Storage:** `bool twoTapSleepEnabled`  
**Default:** false  
**Persistence:** TODO - Save to SettingsManager

## Testing Checklist

### Settings List Screen
- [ ] Navigate to Settings from HOME
- [ ] All 4 rows display correctly
- [ ] Auto-Lock shows "15s" by default
- [ ] Tap Auto-Lock right side cycles through values
- [ ] Values cycle: 5s → 10s → 15s → 20s → 30s → Never → 5s
- [ ] Two Tap Sleep shows toggle switch
- [ ] Toggle switch is OFF by default
- [ ] Tap switch toggles between OFF/ON states
- [ ] Toggle animates (knob moves left/right)
- [ ] Advanced row shows chevron (>)
- [ ] About row shows chevron (>)
- [ ] Back button returns to HOME

### Advanced Navigation
- [ ] Tap Advanced → Opens Advanced Options
- [ ] Back from Advanced → Returns to Settings
- [ ] Settings state preserved (timeout, toggle values)

### About Screen
- [ ] Tap About → Opens About screen
- [ ] Shows "SecureKey" title in brass
- [ ] Shows version "1.0.0"
- [ ] Shows project description (multi-line)
- [ ] Shows "Created by" label
- [ ] Shows "Master" in brass/prominent
- [ ] Shows copyright notice
- [ ] Back button returns to Settings

### Visual Design
- [ ] All colors match design system
- [ ] Rows have proper spacing (2px)
- [ ] Toggle switch colors: gray (off), brass (on)
- [ ] Dropdown chevron is visible and muted
- [ ] Text is ivory on surface background
- [ ] Values are brass colored
- [ ] Border radius on rows is smooth (8px)

## Future Enhancements

### Planned Features
- [ ] Save settings to SettingsManager persistent storage
- [ ] Load settings on boot
- [ ] Auto-lock timer implementation
- [ ] Two-tap sleep gesture detection
- [ ] Settings sync across reboots
- [ ] More timeout options if needed
- [ ] Confirmation dialog for "Never" timeout

### Possible Additions
- [ ] Screen brightness control
- [ ] Sound/haptic feedback toggle
- [ ] Language selection
- [ ] Theme selector (dark/light)
- [ ] Factory reset shortcut
- [ ] Export/import settings

## Summary

✅ **Settings Screen Redesigned** - Clean list-row layout  
✅ **Auto-Lock Feature** - Timeout selector with dropdown  
✅ **Two Tap Sleep** - Toggle switch implementation  
✅ **Advanced Navigation** - Proper back button behavior  
✅ **About Screen Created** - Project info & Master credit  
✅ **Design System Compliant** - All color tokens used correctly  
✅ **Touch Detection** - Row, toggle, and dropdown areas  

The Settings screen now provides a modern, intuitive interface for managing device settings with proper visual feedback and seamless navigation.
