# Quick Start Guide - Web UI Feature

## 🚀 Flash the Firmware

```bash
platformio run --target upload
```

## 📱 Access Web UI from Device

### Step 1: Navigate to Web UI Screen
```
1. Power on device
2. Enter PIN: 1234
3. Tap "Settings"
4. Scroll to 4th item: "Web UI"
5. Tap "Web UI"
```

### Step 2: Start Web UI
```
1. Screen shows "INACTIVE" status
2. Tap "Start Web UI" button
3. Wait 2-3 seconds
4. Status changes to "ACTIVE" (green)
5. Connection details appear:
   - SSID: SecureKey
   - Password: securekey
   - Pair Code: 123456
   - IP: http://192.168.4.1
```

### Step 3: Connect from Phone/Laptop
```
1. Open WiFi settings
2. Connect to "SecureKey"
3. Enter password: securekey
4. Open browser → http://192.168.4.1
5. Enter pair code: 123456
6. Password manager loads
```

### Step 4: Stop Web UI (When Done)
```
1. Return to device
2. Tap "Stop Web UI" button
3. Status changes to "INACTIVE"
4. WiFi hotspot shuts down
```

## 🔑 Default Credentials

| Setting | Value |
|---------|-------|
| Device PIN | 1234 |
| WiFi SSID | SecureKey |
| WiFi Password | securekey |
| Web Pair Code | 123456 |
| Auto-Start | OFF |

## 📊 Build Stats

- **RAM:** 58.3% (47,784 bytes)
- **Flash:** 41.5% (432,987 bytes)
- **Status:** ✅ Build successful

## 🎨 Screen Preview

```
┌─────────────────────────────────────┐
│ ●                                   │
│ < Back          Web UI              │
├─────────────────────────────────────┤
│                                     │
│            ACTIVE                   │ ← Green
│                                     │
│  WiFi Name (SSID):                  │
│  SecureKey                          │
│                                     │
│  WiFi Password:                     │
│  securekey                          │
│                                     │
│  Pair Code:                         │
│     123456                          │ ← Large brass
│                                     │
│  Open browser to:                   │
│  http://192.168.4.1                 │
│                                     │
│  ┌───────────────────────────┐     │
│  │     Stop Web UI           │     │ ← Red
│  └───────────────────────────┘     │
│                                     │
└─────────────────────────────────────┘
```

## ⚠️ Troubleshooting

### Can't see "Web UI" menu item
→ Reflash firmware

### "Start Web UI" button doesn't work
→ Check Serial Monitor (115200 baud)
→ Power cycle device

### Can't connect to WiFi
→ Verify SSID is "SecureKey"
→ Password is "securekey" (lowercase)
→ Device shows "ACTIVE" status

### Browser can't load 192.168.4.1
→ Use http:// not https://
→ Phone connected to "SecureKey" WiFi
→ Disable mobile data temporarily

## 📚 More Info

- **Full Documentation:** `WEBUI_INTEGRATION_COMPLETE.md`
- **Navigation Flow:** `WEBUI_NAVIGATION_FLOW.md`
- **Build Details:** `BUILD_SUCCESS.md`

## ✅ You're Done!

The Web UI feature is fully integrated and ready to use. Flash, test, and enjoy! 🎉
