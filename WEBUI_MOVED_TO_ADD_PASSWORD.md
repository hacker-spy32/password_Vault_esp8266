# ✅ Web UI Control Moved to Add Password Menu

## Changes Made

The "Web UI" control has been **moved from Settings menu** to the **Add Password menu** as requested.

## New Navigation Flow

```
PIN Entry (1234)
    ↓
Home → Add Pass ★
    ↓
Add Password Menu
    ├─ Add Manually → Manual entry screen
    ├─ Add Using Web UI → Web import screen  
    └─ Web UI → [NEW LOCATION] Web UI Status Screen ★
```

## Build Results

```
========================= [SUCCESS] Took 8.38 seconds =========================
RAM:   58.3% (47,736 bytes)
Flash: 41.5% (433,043 bytes)
```

## Updated Menu Structure

### Add Password Menu (3 options):
1. **Add Manually** - Enter credentials directly
2. **Add Using Web UI** - Configure via local server
3. **Web UI** ★ - Control WiFi hotspot (NEW LOCATION)

### Settings Menu (5 options - Web UI removed):
1. Auto-Lock - Configure timeout
2. Manual Sleep - Toggle two-tap sleep
3. Change PIN - Change device PIN
4. Advanced - Advanced options
5. About - About screen

## User Flow to Access Web UI

### Step-by-Step:
1. Boot device → Enter PIN `1234`
2. Tap **"Add Pass"** button (top-left on Home screen)
3. Menu appears with 3 options
4. Tap **"Web UI"** (3rd option) ★
5. Web UI Status screen loads
6. Tap **"Start Web UI"** to launch hotspot
7. Connect phone to WiFi "SecureKey" (password: securekey)
8. Open browser → http://192.168.4.1
9. Enter pair code: 123456

## Screen Layout - Add Password Menu

```
┌─────────────────────────────────────┐
│ ●                           WiFi    │ ← Status bar
│ < Back      Add Password            │ ← Header
├─────────────────────────────────────┤
│                                     │
│  ┌─────────────────────────────┐   │
│  │ Add Manually                │   │ ← Option 1
│  │ Enter credentials directly  │   │
│  │                           > │   │
│  └─────────────────────────────┘   │
│                                     │
│  ┌─────────────────────────────┐   │
│  │ Add Using Web UI            │   │ ← Option 2
│  │ Configure via local server  │   │
│  │                           > │   │
│  └─────────────────────────────┘   │
│                                     │
│  ┌─────────────────────────────┐   │
│  │ Web UI                      │   │ ← Option 3 ★ NEW
│  │ Control WiFi hotspot        │   │
│  │                           > │   │
│  └─────────────────────────────┘   │
│                                     │
└─────────────────────────────────────┘
```

## Code Changes Summary

### Files Modified:

1. **include/AddPasswordMenuScreen.h**
   - Added `METHOD_WEBUI_CONTROL` enum value

2. **src/AddPasswordMenuScreen.cpp**
   - Added 3rd menu row "Web UI" with subtitle "Control WiFi hotspot"
   - Updated touch detection for 3 rows

3. **src/ScreenManager.cpp**
   - Added navigation case: `METHOD_WEBUI_CONTROL` → `SCREEN_WEBUI_STATUS`
   - Changed Web UI Status back navigation: Settings → Add Password Menu
   - Removed `SETTINGS_ROW_WEBUI` case from Settings handler

4. **include/SettingsListScreen.h**
   - Removed `SETTINGS_ROW_WEBUI` enum value

5. **src/SettingsListScreen.cpp**
   - Removed "Web UI" row from Settings menu (was row 4)
   - Updated row count: 6 → 5 rows
   - Updated row detection logic for 5 rows
   - Removed touch handling case for Web UI

## Why This Change Makes Sense

### Logical Grouping:
- **Add Password menu** is now the hub for all password-related actions:
  - Manual entry
  - Web-based entry
  - Web UI server control (enables web-based workflows)

### Settings Focus:
- **Settings menu** now focuses purely on device settings:
  - Security (Auto-Lock, Change PIN)
  - Display/UI (Manual Sleep)
  - System (Advanced, About)

### User Mental Model:
- Users thinking "I want to add passwords" → naturally go to "Add Pass"
- Users see Web UI as a tool for password management, not a system setting
- Web UI control is contextually closer to "Add Using Web UI" option

## Testing Checklist

### Navigation Test:
- [ ] Home → Add Pass → 3 options appear
- [ ] Verify "Web UI" is the 3rd option
- [ ] Tap "Web UI" → Web UI Status screen loads
- [ ] Tap back → Returns to Add Password menu
- [ ] Tap back again → Returns to Home

### Settings Verification:
- [ ] Home → Settings
- [ ] Verify only 5 rows (no Web UI row)
- [ ] Rows are: Auto-Lock, Manual Sleep, Change PIN, Advanced, About

### Web UI Functionality:
- [ ] Add Pass → Web UI → Start Web UI
- [ ] Status changes to ACTIVE
- [ ] WiFi "SecureKey" appears
- [ ] Connect and access http://192.168.4.1
- [ ] Stop Web UI → Status changes to INACTIVE

## Memory Impact

```
Before: 47,784 bytes RAM, 432,987 bytes Flash
After:  47,736 bytes RAM, 433,043 bytes Flash
Change: -48 bytes RAM, +56 bytes Flash (negligible)
```

## Quick Reference

### New Navigation Path:
```
Home
 └─ Add Pass
     └─ Web UI ★ (3rd option)
         └─ Web UI Status Screen
```

### Old Navigation Path (removed):
```
Home
 └─ Settings
     └─ Web UI ✗ (was 4th option - REMOVED)
```

## Flash Instructions

```bash
platformio run --target upload
```

---

**Status:** ✅ Build successful - Ready to flash  
**Change:** Web UI moved from Settings to Add Password menu  
**Date:** 2026-07-24
