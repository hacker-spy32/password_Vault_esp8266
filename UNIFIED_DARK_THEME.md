# Unified Dark Theme - Application-Wide Visual Consistency

## Overview
Complete visual theme unification across all screens to match the modern dark aesthetic of the PIN entry screen. Eliminates mismatched legacy blue/cyan colors in favor of a sophisticated dark design system.

## Design System Colors

### Core Palette
```cpp
#define COLOR_INK 0x0841          // #0A0C10 - Pitch black background
#define COLOR_SURFACE 0x18C3      // #141821 - Card/container surfaces
#define COLOR_LINE 0x2945         // #262C3A - Slate/dark-grey borders
#define COLOR_BRASS 0xFD60        // #E8B564 - Primary accent/highlights
#define COLOR_IVORY 0xFFDE        // #F4EFE4 - Clean white typography
#define COLOR_MUTED 0x8C51        // #8A8FA3 - Labels and metadata
```

### Semantic Colors
```cpp
#define COLOR_ERROR 0xF800        // Red for errors
#define COLOR_WARNING 0xFC00      // Orange/yellow for warnings
#define COLOR_SUCCESS 0x07E0      // Green for success states
```

## Before vs After

### Legacy Colors (Removed)
❌ `TFT_CYAN` - Bright cyan headers  
❌ `TFT_BLUE` - Blue button borders  
❌ `TFT_NAVY` - Navy button backgrounds  
❌ `TFT_DARKGREEN` - Dark green buttons  
❌ `TFT_GREEN` - Green borders  
❌ `TFT_MAROON` - Maroon danger backgrounds  

### New Unified Colors
✓ `COLOR_INK` (#0A0C10) - Consistent pitch-black background  
✓ `COLOR_SURFACE` (#141821) - Dark surfaces for all buttons/cards  
✓ `COLOR_LINE` (#262C3A) - Slate outlines for all borders  
✓ `COLOR_BRASS` (#E8B564) - High-contrast brass accents  
✓ `COLOR_IVORY` (#F4EFE4) - Clean white text throughout  

## Updated Screens

### 1. SettingsScreen
**Changed:**
- Header: Cyan → Dark surface (#141821)
- Header text: Black → White
- Back button: White → Brass accent
- Menu items: Dark grey → Dark surface
- Borders: Light grey → Slate
- Selection: Blue → Brass

**Visual Impact:** Clean, modern menu with consistent dark aesthetic

### 2. OrientationScreen
**Changed:**
- Header: Cyan → Dark surface
- Header text: Black → White
- Back button: White → Brass accent
- Buttons: Dark grey → Dark surface
- Button borders: Light grey → Slate
- Selected state: Green → Brass
- Help text: White → Muted grey

**Visual Impact:** Sophisticated orientation selector with brass highlighting

### 3. AdvancedOptionsScreen
**Changed:**
- Header: Cyan → Dark surface
- Header text: Black → White
- Back button: White → Brass accent
- Diagnostic items: Dark green → Dark surface (same as others)
- Config items: Navy → Dark surface
- Danger items: Maroon → Dark red (#780000)
- All borders: → Slate

**Visual Impact:** Unified menu with subtle danger item differentiation

### 4. FactoryResetScreen
**Changed:**
- Cancel button background: Navy → Dark surface
- Cancel button border: Blue → Slate
- Confirm button background: → Dark red
- Confirm button border: → Bright red
- Added brass accent for highlights

**Visual Impact:** Clear danger indication with consistent dark backdrop

### 5. CalibrationResetScreen
**Changed:**
- Identical updates to FactoryResetScreen
- Cancel button: Navy → Dark surface
- Cancel button border: Blue → Slate
- Confirm button: Dark red with bright red border

**Visual Impact:** Matches factory reset aesthetic

### 6. AddPasswordWebScreen
**Changed:**
- Header: Cyan → Dark surface
- Info text: Yellow → Brass accent
- Back button: → Brass accent

**Visual Impact:** Clean, consistent with rest of app

### 7. ScreenManager (Home/Vault)
**Changed:**
- All button backgrounds: Navy → Dark surface
- All button borders: Blue → Slate
- Favorites border: Green → Brass (special highlight)
- All button text: White on dark surface
- All buttons: Sharp rectangles → Rounded (6px radius)
- Touch feedback dots: Blue → Brass

**Visual Impact:** Modern rounded buttons with cohesive dark theme

### 8. AboutScreen
**Status:** Already using correct dark theme ✓  
**No changes needed**

### 9. ChangePINScreen
**Status:** Already using correct dark theme ✓  
**No changes needed**

### 10. PINEntryScreen
**Status:** Reference implementation ✓  
**Theme originated here**

### 11. CalibrationWizardScreen
**Status:** Preserved as-is per requirements ✓  
**Calibration logic must remain untouched**

## Button Style Improvements

### Old Style (Sharp Rectangles)
```cpp
tft->fillRect(x, y, w, h, TFT_NAVY);
tft->drawRect(x, y, w, h, TFT_BLUE);
```

### New Style (Rounded Rectangles)
```cpp
tft->fillRoundRect(x, y, w, h, 6, COLOR_SURFACE);  // 6px border radius
tft->drawRoundRect(x, y, w, h, 6, COLOR_LINE);
```

**Benefits:**
- Modern, polished appearance
- Softer, more approachable UI
- Matches PIN entry keypad aesthetic
- Reduces visual harshness

## Touch Feedback Consistency

### Password List Touch Dots
**Before:** Blue fill with white outline  
**After:** Brass fill with ivory outline

```cpp
// Old
tft->fillCircle(x, y, radius, TFT_BLUE);
tft->drawCircle(x, y, radius, TFT_WHITE);

// New
tft->fillCircle(x, y, radius, 0xFD60);  // COLOR_BRASS
tft->drawCircle(x, y, radius, 0xFFDE);  // COLOR_IVORY
```

## Typography Hierarchy

### Primary Text
- **Color:** COLOR_IVORY (#F4EFE4)
- **Usage:** Main content, button labels, titles
- **Size:** 1-3 (context dependent)

### Secondary Text
- **Color:** COLOR_MUTED (#8A8FA3)
- **Usage:** Help text, metadata, timestamps
- **Size:** 1

### Accent Text
- **Color:** COLOR_BRASS (#E8B564)
- **Usage:** Back buttons, highlights, interactive elements
- **Size:** 1-2

### Header Text
- **Color:** COLOR_IVORY on COLOR_SURFACE
- **Usage:** Screen titles
- **Size:** 2

## Color Psychology

### Pitch Black Background (#0A0C10)
- **Effect:** Reduces eye strain in low light
- **Feel:** Premium, sophisticated, modern
- **Benefit:** OLED-friendly (power savings)

### Brass Accent (#E8B564)
- **Effect:** Warm, inviting, luxurious
- **Feel:** High-end, quality, attention-grabbing
- **Benefit:** High contrast against dark background

### Slate Borders (#262C3A)
- **Effect:** Subtle definition without harshness
- **Feel:** Clean, organized, structured
- **Benefit:** Maintains hierarchy without visual noise

### Ivory Text (#F4EFE4)
- **Effect:** Softer than pure white, reduces glare
- **Feel:** Elegant, readable, refined
- **Benefit:** Easier on eyes during extended use

## Build Metrics

### Memory Usage
- **RAM:** 54.8% (44,880 bytes / 81,920 bytes)
- **Flash:** 36.0% (376,315 bytes / 1,044,464 bytes)
- **Status:** ✓ SUCCESS

### Changed Files
- SettingsScreen.cpp ✓
- OrientationScreen.cpp ✓
- AdvancedOptionsScreen.cpp ✓
- FactoryResetScreen.cpp ✓
- CalibrationResetScreen.cpp ✓
- AddPasswordWebScreen.cpp ✓
- ScreenManager.cpp ✓

### Preserved Files
- CalibrationWizardScreen.cpp (per requirements)
- AboutScreen.cpp (already correct)
- ChangePINScreen.cpp (already correct)
- PINEntryScreen.cpp (reference implementation)

## Testing Checklist

### Visual Consistency
- [ ] All menus use dark surface headers
- [ ] All buttons have rounded corners (6px)
- [ ] All borders use slate color
- [ ] All text uses ivory color
- [ ] All accents use brass color
- [ ] No cyan/blue legacy colors visible

### Screen-by-Screen
- [ ] Settings menu - unified dark theme
- [ ] Orientation screen - brass selection
- [ ] Advanced options - consistent list style
- [ ] Factory reset - proper danger colors
- [ ] Calibration reset - matches factory reset
- [ ] Add password - clean dark interface
- [ ] Home/Vault - rounded buttons
- [ ] About - already correct ✓
- [ ] Change PIN - already correct ✓
- [ ] PIN Entry - reference ✓

### Interaction Testing
- [ ] Touch feedback dots are brass
- [ ] Back buttons highlighted in brass
- [ ] Selected items show brass accent
- [ ] Danger actions show red tones
- [ ] All text readable on dark backgrounds

## Benefits Summary

✓ **Visual Cohesion** - Entire app feels unified and intentional  
✓ **Modern Aesthetic** - Contemporary dark theme design  
✓ **Reduced Eye Strain** - Dark backgrounds easier on eyes  
✓ **Professional Feel** - Premium appearance throughout  
✓ **Improved Contrast** - Brass accents pop against dark  
✓ **OLED Friendly** - Pitch black saves power  
✓ **Accessibility** - Clear visual hierarchy  
✓ **Brand Identity** - Distinctive brass/slate signature look

## Design Principles Applied

1. **Consistency First** - Same colors, same patterns everywhere
2. **High Contrast** - Brass on black for critical elements
3. **Subtle Hierarchy** - Three text weights (primary, accent, muted)
4. **Modern Patterns** - Rounded corners, dark surfaces
5. **Functional Color** - Red for danger, brass for interaction
6. **Restrained Palette** - Only essential colors used
7. **Accessible** - Clear, readable, high contrast

## Future Enhancements

- [ ] Add subtle gradients to buttons
- [ ] Implement smooth color transitions
- [ ] Add shadow effects for depth
- [ ] Create themed icon set
- [ ] Design custom font rendering
- [ ] Add animation easing curves
- [ ] Implement theme switching (light/dark)
