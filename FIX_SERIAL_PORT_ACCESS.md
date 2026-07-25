# Fix: Serial Port Access Denied (Error 13)

## Problem
```
SerialException: Cannot configure port, something went wrong.
Original message: PermissionError(13, 'Access is denied.', None, 5)
```

## Common Causes
1. **Serial Monitor is open** - PlatformIO/Arduino Serial Monitor
2. **Another IDE has the port open** - Arduino IDE, another VSCode window
3. **Device Manager/Terminal** - Something is reading from the port
4. **Previous upload didn't release** - Stale lock on the port

## Solutions (Try in order)

### 1. Close Serial Monitor (Most Common)
- In VSCode, close any open Serial Monitor terminals
- Look for tabs at the bottom showing serial output
- Press the 🗑️ (trash) icon to fully close them

### 2. Check PlatformIO Device Monitor
```bash
# If you have monitor_speed set in platformio.ini,
# make sure no monitor is running
# Press Ctrl+C in any terminal showing device output
```

### 3. Close Other IDEs/Tools
- Close Arduino IDE if open
- Close any other VSCode windows with PlatformIO projects
- Close Putty, TeraTerm, or other serial terminals

### 4. Unplug and Replug the Device
```
1. Unplug the NodeMCU USB cable
2. Wait 3 seconds
3. Plug it back in
4. Wait for Windows to detect it
5. Try uploading again
```

### 5. Find What's Using the Port (Windows)
```powershell
# Open PowerShell as Administrator and run:
Get-Process | Where-Object {$_.Name -like "*serial*" -or $_.Name -like "*COM*"}

# Look for processes like:
# - Code.exe (VSCode)
# - arduino.exe
# - python.exe (for esptool)
```

### 6. Kill Python Processes (Nuclear Option)
```powershell
# If Python is locking the port:
taskkill /F /IM python.exe

# Then try uploading again
```

### 7. Reset the COM Port
```
1. Open Device Manager (Win + X, then M)
2. Expand "Ports (COM & LPT)"
3. Find your device (usually "USB-SERIAL CH340" or "CP210x")
4. Right-click → Disable device
5. Wait 2 seconds
6. Right-click → Enable device
7. Try uploading again
```

### 8. Change Upload Port in platformio.ini
Sometimes switching ports helps:
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
; Try commenting out upload_port to let PlatformIO auto-detect
; upload_port = COM3
```

## Quick Fix Command (Windows)
```cmd
# Close everything and upload in one command:
taskkill /F /IM python.exe 2>nul & pio run -t upload
```

## Prevention Tips
1. **Always close Serial Monitor before uploading**
2. **Use VSCode's integrated terminal** - easier to track what's open
3. **Add to platformio.ini** to auto-close monitor:
```ini
[env:nodemcuv2]
monitor_speed = 115200
upload_protocol = esptool
; Auto-disconnect monitor on upload
monitor_dtr = 0
monitor_rts = 0
```

## Still Not Working?
Try uploading with explicit port:
```bash
# Replace COM3 with your actual port
pio run -t upload --upload-port COM3
```

Or check which port it actually is:
```bash
pio device list
```
