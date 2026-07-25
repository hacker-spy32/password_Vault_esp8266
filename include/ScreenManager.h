#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

// Forward declaration
class CalibrationWizardScreen;
class CalibrationResetScreen;
class FactoryResetScreen;
class OrientationScreen;
class AdvancedOptionsScreen;
class AddPasswordMenuScreen;
class AddPasswordManualScreen;
class AddPasswordWebScreen;
class FloatingScrollButton;
class SettingsManager;
class VaultStorage;
class PasswordsScreen;
class PasswordDetailScreen;
class FavoritesScreen;
class PasswordEditScreen;
class DeleteConfirmDialog;
class SettingsListScreen;
class AboutScreen;
class PINManager;
class PINEntryScreen;
class ChangePINScreen;
class LockScreen;
class WebUIStatusScreen;
class WebUISettingsManager;

// Screen states
enum ScreenState {
    SCREEN_LOCK,               // NEW: Lock screen with "Swipe to Unlock"
    SCREEN_PIN_ENTRY,          // PIN unlock screen
    SCREEN_HOME,
    SCREEN_SETTINGS,
    SCREEN_INFO,
    SCREEN_FAVORITES,
    SCREEN_ADDPASS,
    SCREEN_TOUCH_TEST,
    SCREEN_CALIBRATION,        // Touch calibration wizard
    SCREEN_CALIBRATION_RESET,  // NEW: Reset calibration to defaults
    SCREEN_FACTORY_RESET,      // NEW: Factory reset all settings
    SCREEN_ORIENTATION,        // NEW: Screen orientation settings
    SCREEN_ADVANCED_OPTIONS,   // NEW: Advanced Options submenu
    SCREEN_ADD_PASSWORD_MENU,  // NEW: Add Password menu
    SCREEN_ADD_PASSWORD_MANUAL,  // NEW: Manual password entry
    SCREEN_ADD_PASSWORD_WEB,    // NEW: Web UI password import
    SCREEN_PASSWORDS,          // NEW: Passwords list view
    SCREEN_PASSWORD_DETAIL,    // NEW: Password detail view
    SCREEN_PASSWORD_EDIT,      // NEW: Password edit screen
    SCREEN_DELETE_CONFIRM,     // NEW: Delete confirmation dialog
    SCREEN_ABOUT,              // NEW: About screen
    SCREEN_CHANGE_PIN,         // NEW: Change PIN screen
    SCREEN_WEBUI_STATUS        // NEW: Web UI status and control screen
};

// Base Screen Interface - enforces unified render pattern
class BaseScreen {
public:
    virtual ~BaseScreen() {}
    
    // Pure virtual methods - all screens MUST implement
    virtual void drawStaticUI() = 0;   // Draw headers, buttons, labels
    virtual void drawDynamicData() = 0; // Draw only changing values
    virtual void handleTouch(const TouchPoint& point) = 0;
    virtual void updateData() = 0;      // Update data (no drawing)
};

class ScreenManager {
public:
    ScreenManager(TFT_eSPI* display, TouchManager* touchMgr);
    
    // Initialize screen manager
    void begin();
    
    // Set Web UI pointers (call from main.cpp after WebUI is set up)
    void setWebUIReferences(WebUISettingsManager* webUISettings, bool* webUIActive);
    
    // Update screen logic based on touch events (NO DRAWING)
    void update();
    
    // Render current screen (ONLY DRAWING)
    void render();
    
    // Screen transition - sets forceFullRedraw flag
    void pushScreen(ScreenState newScreen);
    void popScreen(ScreenState previousScreen);
    
    // Get current screen state
    ScreenState getCurrentScreen() const { return currentScreen; }
    
    // Get PIN manager instance
    PINManager* getPINManager() const { return pinManager; }
    
    // Get Settings manager instance
    SettingsManager* getSettingsManager() const { return settingsManager; }
    
    // Request full screen redraw (e.g. after display re-initialization)
    void requestFullRedraw();
    
    // Screen sleep state management
    bool isScreenAsleep() const { return screenAsleep; }
    void sleepScreen();      // Turn off backlight and enter sleep mode
    void wakeScreen();       // Turn on backlight and restore screen
    
    // Auto-lock timer management
    void resetAutoLockTimer();
    void checkAutoLock();
    
    // Manual sleep gesture detection (double-tap while awake)
    void handleManualSleepGesture();
    
    // Wake gesture detection (double-tap while asleep)
    void handleWakeGesture();
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    ScreenState currentScreen;
    ScreenState screenBeforeSleep;  // Store screen to restore after wake
    
    // Screen sleep state
    bool screenAsleep;
    
    // Auto-lock timer
    unsigned long lastInteractionTime;  // Timestamp of last user interaction
    bool autoLockTriggered;             // Flag to prevent repeated lock attempts
    
    // Manual sleep gesture (two-tap while awake)
    unsigned long lastManualTapTime;    // Timestamp of last tap for manual sleep
    uint8_t manualTapCount;             // Number of taps in manual sleep sequence
    static const unsigned long MANUAL_TAP_WINDOW_MS = 400;  // 400ms window for double-tap
    static const unsigned long DEBOUNCE_MS = 100;  // 100ms debounce to filter touch bounces
    
    // Wake gesture (double-tap while asleep)
    unsigned long lastWakeTapTime;      // Timestamp of last tap while asleep
    uint8_t wakeTapCount;               // Number of taps in wake sequence
    static const unsigned long WAKE_TAP_WINDOW_MS = 400;  // 400ms window for double-tap wake
    
    // Global rendering flags - THE STANDARD
    bool forceFullRedraw;  // Screen transition flag
    bool screenIsDirty;    // Dynamic data update flag
    
    // Settings manager (owned by ScreenManager)
    SettingsManager* settingsManager;
    
    // New Settings List Screen (owned by ScreenManager)
    SettingsListScreen* settingsListScreen;
    AboutScreen* aboutScreen;
    
    // Calibration wizard (owned by ScreenManager)
    CalibrationWizardScreen* calibrationWizard;
    
    // Reset screens (owned by ScreenManager)
    CalibrationResetScreen* calibrationResetScreen;
    FactoryResetScreen* factoryResetScreen;
    OrientationScreen* orientationScreen;
    AdvancedOptionsScreen* advancedOptionsScreen;
    
    // Add Password screens (owned by ScreenManager)
    AddPasswordMenuScreen* addPasswordMenuScreen;
    AddPasswordManualScreen* addPasswordManualScreen;
    AddPasswordWebScreen* addPasswordWebScreen;
    
    // Passwords screens (owned by ScreenManager)
    PasswordsScreen* passwordsScreen;
    PasswordDetailScreen* passwordDetailScreen;
    FavoritesScreen* favoritesScreen;
    PasswordEditScreen* passwordEditScreen;
    DeleteConfirmDialog* deleteConfirmDialog;
    
    // Vault storage (owned by ScreenManager)
    VaultStorage* vaultStorage;
    
    // PIN manager (owned by ScreenManager)
    PINManager* pinManager;
    PINEntryScreen* pinEntryScreen;
    ChangePINScreen* changePINScreen;
    LockScreen* lockScreen;
    
    // Web UI Status screen (owned by ScreenManager)
    WebUIStatusScreen* webUIStatusScreen;
    WebUISettingsManager* webUISettingsManager;
    bool* webUIActivePtr;  // Pointer to global webUIActive flag
    
    // Floating Action Button for scroll navigation
    FloatingScrollButton* scrollFAB;
    
    // Touch Test Screen - Shared data (updated in update(), read in render())
    volatile int16_t sharedTouchX;
    volatile int16_t sharedTouchY;
    volatile int16_t sharedTouchP;
    volatile bool sharedTouchActive;
    
    // Unified render function - THE STANDARD
    void renderCurrentScreen();
    
    // Screen-specific static UI drawing (headers, buttons, labels)
    void drawHomeStaticUI();
    void drawSettingsStaticUI();
    void drawInfoStaticUI();
    void drawFavoritesStaticUI();
    void drawAddPassStaticUI();
    void drawTouchTestStaticUI();
    void drawCalibrationStaticUI();
    void drawCalibrationResetStaticUI();  // NEW
    void drawFactoryResetStaticUI();      // NEW
    void drawOrientationStaticUI();       // NEW
    void drawAdvancedOptionsStaticUI();   // NEW
    void drawAddPasswordMenuStaticUI();   // NEW: Add Password Menu
    void drawAddPasswordManualStaticUI(); // NEW: Manual entry form
    void drawAddPasswordWebStaticUI();    // NEW: Web UI import
    void drawWebUIStatusStaticUI();       // NEW: Web UI status screen
    void drawLockStaticUI();              // NEW: Lock screen
    
    // Screen-specific dynamic data drawing (changing values only)
    void drawHomeDynamicData();
    void drawSettingsDynamicData();
    void drawInfoDynamicData();
    void drawFavoritesDynamicData();
    void drawAddPassDynamicData();
    void drawTouchTestDynamicData();
    void drawCalibrationDynamicData();
    void drawCalibrationResetDynamicData();  // NEW
    void drawFactoryResetDynamicData();      // NEW
    void drawOrientationDynamicData();       // NEW
    void drawAdvancedOptionsDynamicData();   // NEW
    void drawAddPasswordMenuDynamicData();   // NEW: Add Password Menu
    void drawAddPasswordManualDynamicData(); // NEW: Manual entry form
    void drawAddPasswordWebDynamicData();    // NEW: Web UI import
    void drawWebUIStatusDynamicData();       // NEW: Web UI status screen
    void drawLockDynamicData();              // NEW: Lock screen
    
    // Screen-specific data updates (NO DRAWING)
    void updateHomeData();
    void updateSettingsData();
    void updateInfoData();
    void updateFavoritesData();
    void updateAddPassData();
    void updateTouchTestData();
    void updateCalibrationData();
    void updateCalibrationResetData();  // NEW
    void updateFactoryResetData();      // NEW
    void updateOrientationData();       // NEW
    void updateAdvancedOptionsData();   // NEW
    void updateAddPasswordMenuData();   // NEW: Add Password Menu
    void updateAddPasswordManualData(); // NEW: Manual entry form
    void updateAddPasswordWebData();    // NEW: Web UI import
    void updateWebUIStatusData();       // NEW: Web UI status screen
    void updateLockData();              // NEW: Lock screen
    
    // Touch handling for each screen
    void handleHomeTouch(const TouchPoint& point);
    void handleSettingsTouch(const TouchPoint& point);
    void handleInfoTouch(const TouchPoint& point);
    void handleFavoritesTouch(const TouchPoint& point);
    void handleAddPassTouch(const TouchPoint& point);
    void handleTouchTestTouch(const TouchPoint& point);
    void handleCalibrationTouch(const TouchPoint& point);
    void handleCalibrationResetTouch(const TouchPoint& point);  // NEW
    void handleFactoryResetTouch(const TouchPoint& point);      // NEW
    void handleOrientationTouch(const TouchPoint& point);       // NEW
    void handleAdvancedOptionsTouch(const TouchPoint& point);   // NEW
    void handleAddPasswordMenuTouch(const TouchPoint& point);   // NEW: Add Password Menu
    void handleAddPasswordManualTouch(const TouchPoint& point); // NEW: Manual entry form
    void handleAddPasswordWebTouch(const TouchPoint& point);    // NEW: Web UI import
    void handlePasswordsTouch(const TouchPoint& point);         // NEW: Passwords list
    void handlePasswordDetailTouch(const TouchPoint& point);    // NEW: Password detail
    void handlePasswordEditTouch(const TouchPoint& point);      // NEW: Password edit
    void handleDeleteConfirmTouch(const TouchPoint& point);     // NEW: Delete confirm
    void handleAboutTouch(const TouchPoint& point);             // NEW: About screen
    void handlePINEntryTouch(const TouchPoint& point);          // NEW: PIN entry screen
    void handleChangePINTouch(const TouchPoint& point);         // NEW: Change PIN screen
    void handleWebUIStatusTouch(const TouchPoint& point);       // NEW: Web UI status screen
    void handleLockTouch(const TouchPoint& point);              // NEW: Lock screen
    
    // Passwords screen methods
    void drawPasswordsStaticUI();
    void drawPasswordsDynamicData();
    void updatePasswordsData();
    
    // Password detail screen methods
    void drawPasswordDetailStaticUI();
    void drawPasswordDetailDynamicData();
    void updatePasswordDetailData();
    
    // Password edit screen methods
    void drawPasswordEditStaticUI();
    void drawPasswordEditDynamicData();
    void updatePasswordEditData();
    
    // Delete confirm dialog methods
    void drawDeleteConfirmStaticUI();
    void drawDeleteConfirmDynamicData();
    void updateDeleteConfirmData();
    
    // About screen methods
    void drawAboutStaticUI();
    void drawAboutDynamicData();
    void updateAboutData();
    
    // PIN Entry screen methods
    void drawPINEntryStaticUI();
    void drawPINEntryDynamicData();
    void updatePINEntryData();
    
    // Change PIN screen methods
    void drawChangePINStaticUI();
    void drawChangePINDynamicData();
    void updateChangePINData();
};

#endif // SCREENMANAGER_H
