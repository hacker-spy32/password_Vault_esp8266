# Passwords Tab - Complete Implementation Plan

## Overview
Building out the complete Passwords management system with list view, detail view, data persistence, and integration with Add Manually screen.

## Architecture

### 1. Data Layer
**VaultStorage** (✓ Created)
- LittleFS-based persistent storage
- CRUD operations for password entries
- Search, favorites, and filtering
- File format: Binary serialization of PasswordEntry structs
- Location: `/vault.dat` on LittleFS

### 2. UI Screens

#### PasswordsScreen (List View)
- **Purpose**: Display all saved passwords in a scrollable list
- **Layout**:
  - Header with back button and "N TOTAL" badge
  - Scrollable list of password rows
  - Each row: Avatar initial + Service name + Username + Chevron
  - Empty state when no passwords
  - FAB for step-scrolling (50px steps)
  
#### PasswordDetailScreen (Detail View)
- **Purpose**: Show full password details with edit/delete options
- **Layout**:
  - Header with back button and service name
  - Field containers: Title, User, Pass (masked by default)
  - Eye icon button next to password (toggle visibility)
  - Action buttons: Edit | Favorite (heart) | Delete (coral)
  
### 3. Integration Points

#### Add Manually → Vault Storage
```
AddPasswordManualScreen
  ↓ User fills fields + clicks Save
ScreenManager::updateAddPasswordManualData()
  ↓ Checks wasPasswordSaved()
vaultStorage->addPassword(entry)
  ↓ Saves to LittleFS
Returns to HOME
```

#### Home → Passwords List
```
HOME screen
  ↓ User clicks "Passwords" tile
ScreenManager pushScreen(SCREEN_PASSWORDS)
  ↓ Loads from vault
PasswordsScreen displays list
```

#### Passwords List → Password Detail
```
PasswordsScreen
  ↓ User taps a password row
ScreenManager pushScreen(SCREEN_PASSWORD_DETAIL)
  ↓ Passes password ID
PasswordDetailScreen loads entry from vault
```

## Design System Compliance

### Colors
- **Ink**: #0A0C10 (0x0841) - Background
- **Surface**: #141821 (0x18C3) - Cards
- **Line**: #262C3A (0x2945) - Borders
- **Brass**: #E8B564 (0xFD60) - Primary actions
- **Ivory**: #F4EFE4 (0xFFDE) - Text
- **Muted**: #8A8FA3 (0x8C51) - Subtext
- **Coral**: #FF6B6B (0xFB2D) - Delete/danger

### Typography
- **Headers**: Size 2, Ivory
- **Service names**: Size 2, Ivory
- **Usernames**: Size 1, Muted
- **Labels**: Size 1, Brass (uppercase)

### Layout
- **Border radius**: 10px (cards), 8px (fields), 6px (buttons)
- **Row height**: 64px
- **Row spacing**: 8px
- **Side margins**: 16px
- **Avatar size**: 40×40px circle

## Implementation Steps

### Phase 1: Data Layer (✓ DONE)
- [x] VaultStorage.h
- [x] VaultStorage.cpp
- [x] LittleFS integration
- [x] CRUD operations

### Phase 2: Passwords List Screen
- [ ] PasswordsScreen.h
- [ ] PasswordsScreen.cpp
- [ ] Row rendering with avatars
- [ ] Empty state
- [ ] FAB integration
- [ ] Touch handling

### Phase 3: Password Detail Screen
- [ ] PasswordDetailScreen.h
- [ ] PasswordDetailScreen.cpp
- [ ] Masked password display
- [ ] Eye toggle for visibility
- [ ] Edit/Favorite/Delete buttons
- [ ] Touch handling

### Phase 4: ScreenManager Integration
- [ ] Add SCREEN_PASSWORDS enum
- [ ] Add SCREEN_PASSWORD_DETAIL enum
- [ ] Create screen instances
- [ ] Wire up navigation
- [ ] Handle data flow

### Phase 5: Add Manually Integration
- [ ] Wire addPassword() call in ScreenManager
- [ ] Pass PasswordEntry from AddPasswordManualScreen
- [ ] Verify data persistence
- [ ] Test round-trip (add → save → list → detail)

### Phase 6: Testing
- [ ] Add password via manual entry
- [ ] Verify appears in list
- [ ] Tap to view details
- [ ] Toggle password visibility
- [ ] Toggle favorite
- [ ] Delete password
- [ ] Verify persistence across reboots

## File Structure
```
include/
  ├── VaultStorage.h          ✓ Data persistence
  ├── PasswordsScreen.h       → List view
  └── PasswordDetailScreen.h  → Detail view

src/
  ├── VaultStorage.cpp        ✓ Implementation
  ├── PasswordsScreen.cpp     → List implementation
  ├── PasswordDetailScreen.cpp → Detail implementation
  └── ScreenManager.cpp       → Integration (modify)
```

## Next Steps
1. Implement PasswordsScreen.cpp (list view with scrolling)
2. Implement PasswordDetailScreen.h/.cpp (detail with eye toggle)
3. Integrate with ScreenManager
4. Wire Add Manually save to VaultStorage
5. Test complete flow

## Security Notes
- Passwords stored in plaintext on LittleFS (consider encryption in future)
- Password visibility toggle requires active user interaction
- Default state: always masked
- No clipboard operations yet (add in future)
