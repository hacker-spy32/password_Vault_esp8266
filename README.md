# NodeMCU ESP8266 ILI9341 Touch Display Project - Hardware Test

## Hardware Configuration

### NodeMCU ESP8266 (ESP-12E)
- CPU: 160MHz
- Flash: 4MB

### Display: ILI9341 (240x320)
- CS: D8 (GPIO15)
- RESET: D4 (GPIO2)
- DC: D3 (GPIO0)
- SDI (MOSI): D7 (GPIO13)
- SCK: D5 (GPIO14)
- SDO (MISO): D6 (GPIO12)
- SPI Frequency: 40MHz

### Touch Controller: XPT2046
- T_CLK: D5 (GPIO14) - shared with display
- T_CS: D2 (GPIO4)
- T_DIN: D7 (GPIO13) - shared with display
- T_DO: D6 (GPIO12) - shared with display

## Project Structure

```
├── platformio.ini          # PlatformIO configuration
├── include/
│   └── Hardware.h          # Hardware pin definitions and initialization
└── src/
    ├── main.cpp            # Main program with hardware test
    └── Hardware.cpp        # Hardware initialization implementation
```

## Current Status: Hardware Handshake Test

This is a minimal test to verify:
- TFT_eSPI is correctly configured
- Display pins are working
- Screen can be initialized and drawn to

## Expected Behavior

When uploaded, you should see:
1. Black screen background
2. White filled rectangle at position (50, 50)
3. Text "Hardware Init OK" in black inside the white rectangle

## Building and Uploading

1. Install PlatformIO
2. Connect your NodeMCU via USB
3. Build and upload:
   ```
   pio run --target upload
   ```
4. Monitor serial output:
   ```
   pio device monitor
   ```

## Serial Output

You should see:
```
Starting NodeMCU ESP8266 System...
CPU Frequency: 160 MHz
Initializing hardware...
Hardware initialized
Display test complete!
```
