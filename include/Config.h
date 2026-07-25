#ifndef CONFIG_H
#define CONFIG_H

// Development Mode (0 = production, 1 = debug logging)
#define DEV_MODE 0

// Pin Definitions for NodeMCU ESP8266 (ESP-12E)
// Display ILI9341
#define TFT_CS    15  // D8 (GPIO15)
#define TFT_RST   2   // D4 (GPIO2)
#define TFT_DC    0   // D3 (GPIO0)
#define TFT_MOSI  13  // D7 (GPIO13) - SDI
#define TFT_SCLK  14  // D5 (GPIO14) - SCK
#define TFT_MISO  12  // D6 (GPIO12) - SDO
#define TFT_BL    16  // D0 (GPIO16) - Backlight control (optional, may not be connected)

// Touch XPT2046
#define TOUCH_CS  4   // D2 (GPIO4) - T_CS

// SPI Frequency - 40MHz for maximum performance
#define SPI_FREQUENCY 40000000
#define TFT_SPI_FREQUENCY 40000000

// Touch Calibration Defaults
#define TOUCH_CAL_X_MIN 249
#define TOUCH_CAL_X_MAX 3810
#define TOUCH_CAL_Y_MIN 388
#define TOUCH_CAL_Y_MAX 3686

// Touch Pressure Thresholds
#define TOUCH_MIN_PRESSURE 5
#define TOUCH_MAX_PRESSURE 4000

// Screen Dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touch Test Screen - Dot rendering constants
#define DOT_CLEAR_RADIUS 7
#define DOT_FILL_RADIUS 5
#define DOT_OUTLINE_RADIUS 6
#define DOT_MOVEMENT_THRESHOLD 2

// Button Zone Coordinates - Top Row
#define ADDPASS_BTN_X 20
#define ADDPASS_BTN_Y 60
#define ADDPASS_BTN_W 120
#define ADDPASS_BTN_H 50

#define PASSWORDS_BTN_X 180
#define PASSWORDS_BTN_Y 60
#define PASSWORDS_BTN_W 120
#define PASSWORDS_BTN_H 50

// Button Zone Coordinates - Bottom Row
#define FAVORITES_BTN_X 20
#define FAVORITES_BTN_Y 130
#define FAVORITES_BTN_W 120
#define FAVORITES_BTN_H 50

#define SETTINGS_BTN_X 180
#define SETTINGS_BTN_Y 130
#define SETTINGS_BTN_W 120
#define SETTINGS_BTN_H 50

// Debug logging macro
#if DEV_MODE
  #define DEBUG_LOG(x) Serial.println(x)
  #define DEBUG_LOGF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_LOG(x)
  #define DEBUG_LOGF(x, ...)
#endif

#endif // CONFIG_H
