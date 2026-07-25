# Web UI Navigation Flow

## Screen Hierarchy

```
PIN Entry (Boot)
    ↓ [Enter 1234]
Home Screen
    ↓ [Tap "Settings"]
Settings Menu
    ├─ Auto-Lock (dropdown)
    ├─ Manual Sleep (toggle)
    ├─ Change PIN → Change PIN Screen
    ├─ Web UI → [NEW] Web UI Status Screen ★
    ├─ Advanced → Advanced Options
    └─ About → About Screen
```

## Web UI Status Screen Detail

```
┌─────────────────────────────────────┐
│ ● [Brass Dot]                       │ ← Status Bar (24px)
├─────────────────────────────────────┤
│ < Back          Web UI              │ ← Header (32px)
├─────────────────────────────────────┤
│                                     │
│         [ACTIVE/INACTIVE]           │ ← Status (Green/Red)
│                                     │
│  IF ACTIVE:                         │
│  ─────────────                      │
│  WiFi Name (SSID):                  │
│  SecureKey                          │
│                                     │
│  WiFi Password:                     │
│  securekey                          │
│                                     │
│  Pair Code:                         │
│      123456                         │ ← Large brass text
│                                     │
│  Open browser to:                   │
│  http://192.168.4.1                 │
│                                     │
│  ┌───────────────────────────┐     │
│  │     Stop Web UI           │     │ ← Red button
│  └───────────────────────────┘     │
│                                     │
│  IF INACTIVE:                       │
│  ────────────                       │
│  Web UI is not running.             │
│  Press the button below to start... │
│                                     │
│  WiFi Name: SecureKey               │
│  Auto-Start: OFF                    │
│                                     │
│  ┌───────────────────────────┐     │
│  │     Start Web UI          │     │ ← Brass button
│  └───────────────────────────┘     │
│                                     │
└─────────────────────────────────────┘
```

## Touch Zones

### Back Button:
- **Area:** X: 0-60, Y: 24-56 (top-left)
- **Action:** Return to Settings Menu

### Start/Stop Button:
- **Area:** X: 100-220, Y: 180-214 (centered, bottom)
- **Action:** 
  - If INACTIVE → Call `startWebUI()` → Launch WiFi hotspot + web server
  - If ACTIVE → Call `stopWebUI()` → Shutdown hotspot + web server

## Web UI Control Flow

### Starting Web UI:
```
User taps "Start Web UI"
    ↓
handleWebUIStatusTouch() detects button press
    ↓
Calls startWebUI() from main.cpp
    ↓
WiFi.softAP(SSID, Password) → Starts hotspot
    ↓
ESP8266WebServer.begin() → Starts web server on port 80
    ↓
webUIActive = true
    ↓
Screen refreshes → Shows ACTIVE status with credentials
```

### Stopping Web UI:
```
User taps "Stop Web UI"
    ↓
handleWebUIStatusTouch() detects button press
    ↓
Calls stopWebUI() from main.cpp
    ↓
webServer->stop() → Stops web server
    ↓
WiFi.softAPdisconnect(true) → Stops hotspot
    ↓
WiFi.mode(WIFI_OFF) → Turns off WiFi
    ↓
webUIActive = false
    ↓
Screen refreshes → Shows INACTIVE status
```

## Integration Points

### 1. ScreenManager.cpp
- **Update Loop:** Checks `webUIStatusScreen->needsToExit()` and pops to Settings
- **Render Loop:** Calls `webUIStatusScreen->draw()` for SCREEN_WEBUI_STATUS
- **Touch Handler:** Routes touches to `webUIStatusScreen->onTouchEvent()`

### 2. SettingsListScreen.cpp
- **Row 4:** "Web UI" with chevron (>)
- **Touch Detection:** Sets `selectedAction = SETTINGS_ROW_WEBUI`
- **Navigation:** ScreenManager pushes SCREEN_WEBUI_STATUS

### 3. main.cpp
- **Setup:** Calls `screenManager->setWebUIReferences(&webUISettings, &webUIActive)`
- **Loop:** Calls `webServer->handleClient()` if `webUIActive == true`
- **Global Functions:** `startWebUI()` and `stopWebUI()` control the Web UI lifecycle

## Code References

### Key Files:
- **WebUIStatusScreen:** `src/WebUIStatusScreen.cpp` (234 lines)
- **ScreenManager Integration:** `src/ScreenManager.cpp` (added ~60 lines)
- **Settings Menu:** `src/SettingsListScreen.cpp` (modified 3 sections)
- **Main Control:** `src/main.cpp` (1 line added to setup)

### Key Methods:
```cpp
// ScreenManager.cpp
void ScreenManager::setWebUIReferences(WebUISettingsManager*, bool*)
void ScreenManager::drawWebUIStatusStaticUI()
void ScreenManager::drawWebUIStatusDynamicData()
void ScreenManager::updateWebUIStatusData()
void ScreenManager::handleWebUIStatusTouch(const TouchPoint&)

// WebUIStatusScreen.cpp
void WebUIStatusScreen::draw()
void WebUIStatusScreen::onTouchEvent(const TouchPoint&)
bool WebUIStatusScreen::needsToExit()

// main.cpp
void startWebUI()
void stopWebUI()
bool isWebUIActive()
```

## Testing Sequence

1. **Flash firmware** to ESP8266
2. **Device boots** → PIN screen
3. **Enter PIN:** 1234
4. **Navigate:** Home → Settings → Web UI
5. **Verify:** INACTIVE status shown
6. **Tap:** "Start Web UI" button
7. **Verify:** Status changes to ACTIVE
8. **Check:** SSID "SecureKey" appears in WiFi networks
9. **Connect:** Phone/laptop to "SecureKey" (password: securekey)
10. **Open browser:** http://192.168.4.1
11. **Enter pair code:** 123456
12. **Verify:** Password manager interface loads
13. **Return to device:** Tap "Stop Web UI"
14. **Verify:** Status changes to INACTIVE
15. **Check:** WiFi network disappears

## Success Criteria

✅ All files compile without errors  
✅ Navigation from Settings → Web UI Status works  
✅ Screen displays correct status (ACTIVE/INACTIVE)  
✅ Start button launches Web UI  
✅ Stop button shuts down Web UI  
✅ Credentials display correctly when active  
✅ Back button returns to Settings  
✅ Visual theme matches rest of app  
✅ Touch zones respond correctly  
✅ No memory leaks or crashes  
