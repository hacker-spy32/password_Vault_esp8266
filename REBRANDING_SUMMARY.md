# 🎨 PassGuard Rebranding Summary

## Overview
The project has been successfully rebranded from **SecureKey** to **PassGuard**.

## Changes Made

### 1. Source Code Files
- ✅ `src/WebUISettingsManager.cpp` - Updated default SSID to "PassGuard" and password to "passguard"
- ✅ `src/main.cpp` - Updated CSV export filename to "passguard_export.csv"
- ✅ `src/AddPasswordMenuScreen.cpp` - Updated brand comment from "SecureKey" to "PassGuard"
- ✅ `src/AddPasswordManualScreen.cpp` - Updated brand comment from "SecureKey" to "PassGuard"
- ✅ `src/AboutScreen.cpp` - Updated on-screen display text from "SecureKey" to "PassGuard"

### 2. Web UI Portal
- ✅ `include/portal_html.h` - Updated all web interface references:
  - Page title: "SecureKey Portal" → "PassGuard Portal"
  - Header text: "SecureKey" → "PassGuard"
  - WiFi SSID placeholder: "SecureKey" → "PassGuard"
  - Default SSID fallback: "SecureKey" → "PassGuard"

### 3. Documentation
- ✅ `README.md` - Updated all references:
  - Project title: "SecureKey" → "PassGuard"
  - Project description references
  - WiFi access point name: "SecureKey-Vault" → "PassGuard-Vault"
  - Binary filename reference: "securekey_nodemcuv2.bin" → "passguard_nodemcuv2.bin"

### 4. Binary Files
- ✅ `bin/securekey_nodemcuv2.bin` → `bin/passguard_nodemcuv2.bin` (renamed)

### 5. Random Credentials Feature
Added in `src/main.cpp`:
- ✅ `generateRandomPassword()` - Generates 8-character letter-only passwords
- ✅ `generateRandomPIN()` - Generates 6-digit number-only PINs
- ✅ Modified `startWebUI()` to generate fresh credentials on every startup

## New Default Settings
- **WiFi SSID**: PassGuard (static, doesn't change)
- **WiFi Password**: Random 8 letters (changes every restart)
- **Pair Code**: Random 6 digits (changes every restart)

## What Stays the Same
- All functionality remains identical
- File structure unchanged (except renamed binary)
- Configuration file formats unchanged
- User data and vault storage format unchanged

## Testing Recommendations
1. Flash the updated firmware to your device
2. Verify the About screen shows "PassGuard"
3. Start the Web UI and confirm:
   - WiFi network appears as "PassGuard"
   - Password and PIN change on each restart
   - Web portal displays "PassGuard" branding
4. Export passwords and verify filename is "passguard_export.csv"

---
**Date**: Rebranding completed successfully
**Version**: All references updated from SecureKey to PassGuard
