# 🔐 PassGuard - ESP8266 Hardware Password Vault

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build-orange?logo=platformio)](https://platformio.org/)
[![Board](https://img.shields.io/badge/Board-ESP8266%20NodeMCU-blue?logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-ILI9341%20320x240-brightgreen)](https://github.com/Bodmer/TFT_eSPI)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**PassGuard** is a standalone, offline hardware password vault built for the **NodeMCU ESP8266 (ESP-12E)** microcontroller equipped with an **ILI9341 320x240 TFT LCD Touchscreen** and **XPT2046 touch controller**. 

It features an ultra-sleek, pixel-perfect Dark Mode user interface, master PIN security protection, an on-screen ABCD touch keyboard with intelligent auto-suggestions, LittleFS encrypted vault storage, a built-in Web UI management portal, and a 5-point touch calibration wizard.

---

## ✨ Features

- 🛡️ **Master PIN Security Protection**:
  - Secure 4-digit master PIN lock screen required on startup.
  - Automatic auto-lock timer and sleep mode on idle.
  - Incorrect PIN shake/error feedback animation.
  - Guided Master PIN change interface.

- ⌨️ **On-Screen ABCD Touch Keyboard**:
  - Custom full-featured ABCD touchscreen keyboard optimized for 2.8" displays.
  - Intelligent auto-suggestions engine for popular domain extensions (`@gmail.com`, `@outlook.com`) and title names (`Amazon`, `Google`, `GitHub`).
  - Shift/Caps mode toggle, spacebar, backspace, and one-tap field completion.

- 📱 **Pixel-Perfect 320x240 Touch Interface**:
  - Unified Dark Theme palette (`#0A0C10` Ink background, `#E8B564` Brass accent, `#F4EFE4` Ivory text).
  - Compact input fields designed to fit 100% on 320x240 screen without overflow.
  - Smooth page navigation stack with anti-flicker rendering pipeline.

- 📁 **Encrypted Vault Storage (LittleFS)**:
  - Persistent password entry storage saved directly in ESP8266 SPI flash memory.
  - Supports entry titles, usernames, passwords, favorite bookmarks, and one-time access entries.
  - Fast search, view, edit, and delete management.

- 🌐 **Web UI & Captive Portal**:
  - Integrated SoftAP WiFi access point (`PassGuard-Vault`) with Captive Portal DNS auto-popup.
  - Responsive Web UI dashboard for bulk CSV password import, export, and wireless editing.
  - Background execution mode option allowing Web UI and hardware screen to operate simultaneously.

- 🎯 **Interactive Touch Calibration Wizard**:
  - 5-point precision crosshair touch calibration system.
  - Saves calibration data persistently to hardware storage.
  - Real-time touch coordinates debug layer and test screen.

- 🌙 **Power Management & Gestures**:
  - Double-tap manual sleep gesture and double-tap screen wake gesture.
  - Configurable auto-lock timeout.

---

## 🛠️ Hardware Requirements & Wiring

### Hardware Components
- **Microcontroller**: NodeMCU ESP8266 (ESP-12E / ESP-07)
- **Display**: 2.4" or 2.8" ILI9341 SPI TFT Display (320x240 resolution)
- **Touch Controller**: XPT2046 SPI Touch Controller (integrated on TFT board)

### Complete Pinout Mapping

#### ILI9341 Display Wiring
| TFT Display Pin | NodeMCU Pin | GPIO Pin | Function |
| :--- | :--- | :--- | :--- |
| **VCC** | 3V3 | 3.3V | Display Power (3.3V) |
| **GND** | GND | GND | Ground |
| **CS** | D8 | GPIO15 | TFT Chip Select (`TFT_CS`) |
| **RESET / RST** | D4 | GPIO2 | TFT Reset (`TFT_RST`) |
| **DC / RS** | D3 | GPIO0 | Data / Command Select (`TFT_DC`) |
| **SDI / MOSI** | D7 | GPIO13 | Hardware SPI MOSI (`TFT_MOSI`) |
| **SCK / CLK** | D5 | GPIO14 | Hardware SPI Clock (`TFT_SCLK`) |
| **LED / BL** | D0 | GPIO16 | Backlight Control (`TFT_BL`) |
| **SDO / MISO** | D6 | GPIO12 | Hardware SPI MISO (`TFT_MISO`) |

#### XPT2046 Touch Controller Wiring
| Touch Module Pin | NodeMCU Pin | GPIO Pin | Function |
| :--- | :--- | :--- | :--- |
| **T_CS** | D2 | GPIO4 | Dedicated Touch Chip Select (`TOUCH_CS`) |
| **T_CLK** | D5 | GPIO14 | Shared SPI Clock (connects to TFT SCK / D5) |
| **T_DIN** | D7 | GPIO13 | Shared SPI MOSI (connects to TFT SDI / D7) |
| **T_DO** | D6 | GPIO12 | Shared SPI MISO (connects to TFT SDO / D6) |
| **T_IRQ** | *NC* | -- | Unused / Optional interrupt pin |

---

## 📂 Project Structure

```
.
├── bin/                         # Pre-compiled firmware binaries
│   ├── firmware.bin             # Production binary release
│   └── passguard_nodemcuv2.bin  # NodeMCU ESP8266 flashed binary
├── include/                     # C++ Header Files
│   ├── LockScreen.h             # Static lock screen interface
│   ├── PINEntryScreen.h         # Master PIN entry & security
│   ├── PasswordsScreen.h        # Saved passwords list view
│   ├── PasswordDetailScreen.h   # Individual password viewer
│   ├── AddPasswordManualScreen.h# Manual password entry form
│   ├── PasswordEditScreen.h     # Password editing screen
│   ├── TouchKeyboard.h          # ABCD touchscreen keyboard engine
│   ├── ScreenManager.h          # Global screen state machine & render loop
│   ├── VaultStorage.h           # LittleFS password storage manager
│   ├── SettingsManager.h        # Persistent system settings & calibration
│   ├── TouchManager.h           # XPT2046 touch debouncing & scaling
│   └── Config.h                 # Hardware pins & screen dimensions
├── src/                         # C++ Source Code Implementations
│   ├── main.cpp                 # Entry point & main event loop
│   ├── Hardware.cpp             # Hardware SPI & display initialization
│   ├── ScreenManager.cpp        # Navigation render engine
│   ├── TouchKeyboard.cpp        # On-screen keyboard implementation
│   ├── VaultStorage.cpp         # Flash storage file handler
│   └── ...                      # Screen implementations
├── platformio.ini               # PlatformIO build configuration
└── README.md                    # Project documentation
```

---

## 🚀 Getting Started

### Option 1: Flash Pre-Compiled Binary (`bin/`)

Ready-to-flash binaries are stored in the [`bin/`](bin/) folder:

Using `esptool.py`:
```bash
esptool.py --port COM3 --baud 460800 write_flash 0x0 bin/firmware.bin
```

---

### Option 2: Build & Flash with PlatformIO

1. **Clone the repository**:
   ```bash
   git clone https://github.com/hacker-spy32/password_Vault_esp8266.git
   cd password_Vault_esp8266
   ```

2. **Build Firmware**:
   ```bash
   pio run
   ```

3. **Upload to ESP8266**:
   Connect your NodeMCU board via USB and run:
   ```bash
   pio run --target upload
   ```

4. **Monitor Serial Output**:
   ```bash
   pio device monitor
   ```

---

## 📜 License

This project is released under the [MIT License](LICENSE).
