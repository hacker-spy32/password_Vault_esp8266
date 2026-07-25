#include "ScreenManager.h"
#include "CalibrationWizardScreen.h"
#include "CalibrationResetScreen.h"
#include "FactoryResetScreen.h"
#include "OrientationScreen.h"
#include "AdvancedOptionsScreen.h"
#include "AddPasswordMenuScreen.h"
#include "AddPasswordManualScreen.h"
#include "AddPasswordWebScreen.h"
#include "PasswordsScreen.h"
#include "PasswordDetailScreen.h"
#include "FavoritesScreen.h"
#include "PasswordEditScreen.h"
#include "DeleteConfirmDialog.h"
#include "SettingsListScreen.h"
#include "AboutScreen.h"
#include "VaultStorage.h"
#include "FloatingScrollButton.h"
#include "SettingsManager.h"
#include "PINManager.h"
#include "PINEntryScreen.h"
#include "ChangePINScreen.h"
#include "LockScreen.h"
#include "WebUIStatusScreen.h"
#include "Hardware.h"

// External reference to global touch controller
extern XPT2046_Touchscreen ts;

// ============================================================================
// DEBUG LAYER - Comprehensive UI Navigation Debugging
// ============================================================================

// Helper function to get screen name as string
const char* getScreenName(ScreenState screen) {
    switch (screen) {
        case SCREEN_LOCK: return "LOCK";
        case SCREEN_HOME: return "HOME";
        case SCREEN_SETTINGS: return "SETTINGS";
        case SCREEN_INFO: return "INFO";
        case SCREEN_FAVORITES: return "FAVORITES";
        case SCREEN_ADDPASS: return "ADDPASS";
        case SCREEN_TOUCH_TEST: return "TOUCH_TEST";
        case SCREEN_CALIBRATION: return "CALIBRATION";
        case SCREEN_CALIBRATION_RESET: return "CALIBRATION_RESET";
        case SCREEN_FACTORY_RESET: return "FACTORY_RESET";
        case SCREEN_ORIENTATION: return "ORIENTATION";
        case SCREEN_ADVANCED_OPTIONS: return "ADVANCED_OPTIONS";
        case SCREEN_ADD_PASSWORD_MENU: return "ADD_PASSWORD_MENU";
        case SCREEN_ADD_PASSWORD_MANUAL: return "ADD_PASSWORD_MANUAL";
        case SCREEN_ADD_PASSWORD_WEB: return "ADD_PASSWORD_WEB";
        case SCREEN_PASSWORDS: return "PASSWORDS";
        case SCREEN_PASSWORD_DETAIL: return "PASSWORD_DETAIL";
        case SCREEN_PASSWORD_EDIT: return "PASSWORD_EDIT";
        case SCREEN_DELETE_CONFIRM: return "DELETE_CONFIRM";
        case SCREEN_ABOUT: return "ABOUT";
        case SCREEN_PIN_ENTRY: return "PIN_ENTRY";
        case SCREEN_CHANGE_PIN: return "CHANGE_PIN";
        case SCREEN_WEBUI_STATUS: return "WEBUI_STATUS";
        default: return "UNKNOWN";
    }
}

// Input debugger - logs all touch events with coordinates
void printInputDebug(int16_t x, int16_t y, const char* eventType, const char* screenName) {
    Serial.printf("[DEBUG_INPUT] X: %d, Y: %d, Type: %s, Screen: %s\n", 
                  x, y, eventType, screenName);
}

// Button hit-test debugger - logs button press attempts
void printButtonDebug(const char* buttonName, const char* screenName, bool hit) {
    if (hit) {
        Serial.printf("[DEBUG_BUTTON] HIT: '%s' on %s screen\n", buttonName, screenName);
    } else {
        Serial.printf("[DEBUG_BUTTON] MISS: '%s' on %s screen\n", buttonName, screenName);
    }
}

// Navigation debugger - logs screen transitions
void printNavigationDebug(ScreenState oldScreen, ScreenState newScreen, const char* transitionType) {
    Serial.printf("[NAV] Transition (%s): %s -> %s\n", 
                  transitionType, getScreenName(oldScreen), getScreenName(newScreen));
}

// ============================================================================
// SCREENMAMAGER IMPLEMENTATION
// ============================================================================

ScreenManager::ScreenManager(TFT_eSPI* display, TouchManager* touchMgr)
    : tft(display),
      touch(touchMgr),
      currentScreen(SCREEN_LOCK),  // Start with lock screen
      screenBeforeSleep(SCREEN_LOCK),
      screenAsleep(false),
      lastInteractionTime(0),
      autoLockTriggered(false),
      lastManualTapTime(0),
      manualTapCount(0),
      lastWakeTapTime(0),
      wakeTapCount(0),
      forceFullRedraw(true),   // Force initial draw
      screenIsDirty(true),     // Force initial data update
      settingsManager(nullptr),  // Will be created on demand
      settingsListScreen(nullptr),  // Will be created on demand
      aboutScreen(nullptr),  // Will be created on demand
      calibrationWizard(nullptr),  // Will be created on demand
      calibrationResetScreen(nullptr),  // Will be created on demand
      factoryResetScreen(nullptr),  // Will be created on demand
      orientationScreen(nullptr),  // Will be created on demand
      advancedOptionsScreen(nullptr),  // Will be created on demand
      addPasswordMenuScreen(nullptr),  // Will be created on demand
      addPasswordManualScreen(nullptr),  // Will be created on demand
      addPasswordWebScreen(nullptr),  // Will be created on demand
      passwordsScreen(nullptr),  // Will be created on demand
      passwordDetailScreen(nullptr),  // Will be created on demand
      favoritesScreen(nullptr),  // Will be created on demand
      passwordEditScreen(nullptr),  // Will be created on demand
      deleteConfirmDialog(nullptr),  // Will be created on demand
      vaultStorage(nullptr),  // Will be created on demand
      pinManager(nullptr),  // Will be created on demand
      pinEntryScreen(nullptr),  // Will be created on demand
      changePINScreen(nullptr),  // Will be created on demand
      lockScreen(nullptr),  // Will be created on demand
      webUIStatusScreen(nullptr),  // Will be created on demand
      webUISettingsManager(nullptr),  // Set via setWebUIReferences()
      webUIActivePtr(nullptr),  // Set via setWebUIReferences()
      scrollFAB(nullptr),  // Will be created on demand
      sharedTouchX(0),
      sharedTouchY(0),
      sharedTouchP(0),
      sharedTouchActive(false) {
}

void ScreenManager::begin() {
    // Create PIN manager FIRST (security layer)
    pinManager = new PINManager();
    pinManager->begin();
    
    // Create vault storage (needed by password screens)
    vaultStorage = new VaultStorage();
    if (!vaultStorage->begin()) {
        Serial.println("[SCREEN_MGR] Failed to initialize vault storage!");
    }
    
    // Create settings manager
    settingsManager = new SettingsManager();
    settingsManager->begin();
    
    // Create PIN entry screen
    pinEntryScreen = new PINEntryScreen(tft, touch, pinManager);
    pinEntryScreen->begin();
    
    // Create lock screen
    lockScreen = new LockScreen(tft, touch);
    lockScreen->begin();
    
    // Link settings manager to touch manager for orientation awareness
    touch->setSettingsManager(settingsManager);
    
    // Apply saved orientation from settings
    applyDisplayOrientation(settingsManager);
    
    // Initialize auto-lock timer
    resetAutoLockTimer();
    
    DEBUG_LOG("ScreenManager initialized - Global Standard Active");
}

void ScreenManager::setWebUIReferences(WebUISettingsManager* webUISettings, bool* webUIActive) {
    webUISettingsManager = webUISettings;
    webUIActivePtr = webUIActive;
    Serial.println("[SCREEN_MGR] Web UI references set");
}

void ScreenManager::requestFullRedraw() {
    forceFullRedraw = true;
    screenIsDirty = true;
    if (currentScreen == SCREEN_LOCK && lockScreen != nullptr) {
        lockScreen->reset();
    }
}

// ============================================================================
// SCREEN TRANSITION METHODS - Set forceFullRedraw flag
// ============================================================================

void ScreenManager::pushScreen(ScreenState newScreen) {
    ScreenState oldScreen = currentScreen;  // Capture old state for debug
    
    currentScreen = newScreen;
    forceFullRedraw = true;  // THE STANDARD: force full redraw on transition
    
    // Navigation debug
    printNavigationDebug(oldScreen, newScreen, "PUSH");
    
    DEBUG_LOGF("pushScreen -> %d (forceFullRedraw=true)\n", newScreen);
}

void ScreenManager::popScreen(ScreenState previousScreen) {
    ScreenState oldScreen = currentScreen;  // Capture old state for debug
    
    currentScreen = previousScreen;
    forceFullRedraw = true;  // THE STANDARD: force full redraw on transition
    
    // Navigation debug
    printNavigationDebug(oldScreen, previousScreen, "POP");
    
    DEBUG_LOGF("popScreen -> %d (forceFullRedraw=true)\n", previousScreen);
}

// ============================================================================
// UPDATE METHOD - Input sampling and state updates (NO DRAWING)
// ============================================================================

void ScreenManager::update() {
    // Step 1: Update screen-specific data (NO DRAWING)
    switch (currentScreen) {
        case SCREEN_LOCK:
            if (lockScreen != nullptr) {
                lockScreen->update();
                
                // Check if unlocked
                if (lockScreen->shouldUnlock()) {
                    lockScreen->clearUnlockRequest();
                    Serial.println("[SCREEN_MGR] LockScreen unlock requested - transitioning to PIN_ENTRY");
                    pushScreen(SCREEN_PIN_ENTRY);
                }
            }
            updateLockData();
            break;
        case SCREEN_HOME:
            updateHomeData();
            break;
        case SCREEN_SETTINGS:
            updateSettingsData();
            break;
        case SCREEN_INFO:
            updateInfoData();
            break;
        case SCREEN_FAVORITES:
            updateFavoritesData();
            break;
        case SCREEN_ADDPASS:
            updateAddPassData();
            break;
        case SCREEN_TOUCH_TEST:
            updateTouchTestData();
            break;
        case SCREEN_CALIBRATION:
            updateCalibrationData();
            break;
        case SCREEN_CALIBRATION_RESET:
            updateCalibrationResetData();
            break;
        case SCREEN_FACTORY_RESET:
            updateFactoryResetData();
            break;
        case SCREEN_ORIENTATION:
            updateOrientationData();
            break;
        case SCREEN_ADVANCED_OPTIONS:
            updateAdvancedOptionsData();
            break;
        case SCREEN_ADD_PASSWORD_MENU:
            updateAddPasswordMenuData();
            break;
        case SCREEN_ADD_PASSWORD_MANUAL:
            updateAddPasswordManualData();
            break;
        case SCREEN_ADD_PASSWORD_WEB:
            updateAddPasswordWebData();
            break;
        case SCREEN_PASSWORDS:
            updatePasswordsData();
            break;
        case SCREEN_PASSWORD_DETAIL:
            updatePasswordDetailData();
            break;
        case SCREEN_PASSWORD_EDIT:
            updatePasswordEditData();
            break;
        case SCREEN_DELETE_CONFIRM:
            updateDeleteConfirmData();
            break;
        case SCREEN_ABOUT:
            updateAboutData();
            break;
        case SCREEN_PIN_ENTRY:
            if (pinEntryScreen != nullptr) {
                pinEntryScreen->update();
                
                // Check if unlocked
                if (pinEntryScreen->isUnlocked()) {
                    Serial.println("[SCREEN_MGR] PIN correct - transitioning to HOME");
                    pushScreen(SCREEN_HOME);
                }
            }
            updatePINEntryData();
            break;
        case SCREEN_CHANGE_PIN:
            if (changePINScreen != nullptr) {
                changePINScreen->update();
                
                // Check if should exit
                if (changePINScreen->shouldExit()) {
                    Serial.println("[SCREEN_MGR] ChangePIN complete - returning to settings");
                    popScreen(SCREEN_SETTINGS);
                }
            }
            updateChangePINData();
            break;
        case SCREEN_WEBUI_STATUS:
            if (webUIStatusScreen != nullptr) {
                webUIStatusScreen->update();
                
                // Check if should exit
                if (webUIStatusScreen->needsExit()) {
                    Serial.println("[SCREEN_MGR] WebUI Status - returning to Add Password menu");
                    popScreen(SCREEN_ADD_PASSWORD_MENU);
                }
            }
            updateWebUIStatusData();
            break;
    }
    
    // Step 2: Handle touch input (sets screenIsDirty if navigation occurs)
    TouchState touchState = touch->getState();
    
    // Handle wake gesture when screen is asleep - use TOUCH_DOWN for immediate response
    if (screenAsleep) {
        if (touchState == TOUCH_DOWN) {
            Serial.println("[SLEEP_WAKE] *** TOUCH_DOWN detected while asleep ***");
            TouchPoint point = touch->getPoint();
            Serial.printf("[SLEEP_WAKE] Touch point valid: %s, coords: (%d, %d), pressure: %d\n", 
                         point.valid ? "YES" : "NO", point.x, point.y, point.pressure);
            if (point.valid) {
                handleWakeGesture();
                return;  // Don't process normal touch events while asleep
            } else {
                Serial.println("[SLEEP_WAKE] *** Touch point INVALID - check calibration ***");
            }
        }
        // Early return if asleep - don't process any other touch events
        return;
    }
    
    // Process normal touch events only when awake
    if (touchState == TOUCH_DOWN) {
        TouchPoint point = touch->getPoint();
        
        if (point.valid) {
            // Reset auto-lock timer on any valid touch
            resetAutoLockTimer();
            
            // Check for manual sleep gesture (if enabled and NOT on a keypad/keyboard screen)
            bool isKeypadScreen = (currentScreen == SCREEN_PIN_ENTRY || 
                                   currentScreen == SCREEN_CHANGE_PIN || 
                                   currentScreen == SCREEN_LOCK || 
                                   currentScreen == SCREEN_ADD_PASSWORD_MANUAL || 
                                   currentScreen == SCREEN_PASSWORD_EDIT);
                                   
            if (settingsManager != nullptr && settingsManager->getTwoTapSleepEnabled() && !isKeypadScreen) {
                Serial.println("[TOUCH] Two Tap Sleep is ENABLED - checking for manual sleep gesture");
                handleManualSleepGesture();
                // Continue processing touch - manual sleep detection doesn't block normal touches
                Serial.println("[TOUCH] Continuing to normal touch processing...");
            } else if (isKeypadScreen) {
                // Reset tap counter when on keypad screen so taps aren't accumulated
                manualTapCount = 0;
                lastManualTapTime = 0;
            }
            
            // Input debugger - log all valid touch events
            printInputDebug(point.x, point.y, "TOUCH_DOWN", getScreenName(currentScreen));
            
            switch (currentScreen) {
                case SCREEN_HOME:
                    handleHomeTouch(point);
                    break;
                case SCREEN_SETTINGS:
                    handleSettingsTouch(point);
                    break;
                case SCREEN_INFO:
                    handleInfoTouch(point);
                    break;
                case SCREEN_FAVORITES:
                    handleFavoritesTouch(point);
                    break;
                case SCREEN_ADDPASS:
                    handleAddPassTouch(point);
                    break;
                case SCREEN_TOUCH_TEST:
                    handleTouchTestTouch(point);
                    break;
                case SCREEN_CALIBRATION:
                    handleCalibrationTouch(point);
                    break;
                case SCREEN_CALIBRATION_RESET:
                    handleCalibrationResetTouch(point);
                    break;
                case SCREEN_FACTORY_RESET:
                    handleFactoryResetTouch(point);
                    break;
                case SCREEN_ORIENTATION:
                    handleOrientationTouch(point);
                    break;
                case SCREEN_ADVANCED_OPTIONS:
                    handleAdvancedOptionsTouch(point);
                    break;
                case SCREEN_ADD_PASSWORD_MENU:
                    handleAddPasswordMenuTouch(point);
                    break;
                case SCREEN_ADD_PASSWORD_MANUAL:
                    handleAddPasswordManualTouch(point);
                    break;
                case SCREEN_ADD_PASSWORD_WEB:
                    handleAddPasswordWebTouch(point);
                    break;
                case SCREEN_PASSWORDS:
                    handlePasswordsTouch(point);
                    break;
                case SCREEN_PASSWORD_DETAIL:
                    handlePasswordDetailTouch(point);
                    break;
                case SCREEN_PASSWORD_EDIT:
                    handlePasswordEditTouch(point);
                    break;
                case SCREEN_DELETE_CONFIRM:
                    handleDeleteConfirmTouch(point);
                    break;
                case SCREEN_ABOUT:
                    handleAboutTouch(point);
                    break;
                case SCREEN_PIN_ENTRY:
                    handlePINEntryTouch(point);
                    break;
                case SCREEN_CHANGE_PIN:
                    handleChangePINTouch(point);
                    break;
                case SCREEN_WEBUI_STATUS:
                    handleWebUIStatusTouch(point);
                    break;
                case SCREEN_LOCK:
                    handleLockTouch(point);
                    break;
            }
        } else {
            // Log invalid touch attempts
            Serial.println("[DEBUG_INPUT] Touch event INVALID - check TouchManager calibration");
        }
    }
}

// ============================================================================
// RENDER METHOD - THE GLOBAL STANDARD
// ============================================================================

void ScreenManager::render() {
    // Don't render anything if screen is asleep
    if (screenAsleep) {
        return;
    }
    
    // THE STANDARD: Unified render pattern
    if (forceFullRedraw) {
        // Render trigger debug
        Serial.printf("[DEBUG_RENDER] Full Clear Triggered on %s screen\n", getScreenName(currentScreen));
        
        tft->fillScreen(TFT_BLACK);  // Clear entire screen
        
        // Draw static UI based on current screen
        switch (currentScreen) {
            case SCREEN_LOCK:
                drawLockStaticUI();
                if (lockScreen != nullptr) {
                    lockScreen->draw();
                }
                break;
            case SCREEN_HOME:
                drawHomeStaticUI();
                break;
            case SCREEN_SETTINGS:
                drawSettingsStaticUI();
                break;
            case SCREEN_INFO:
                drawInfoStaticUI();
                break;
            case SCREEN_FAVORITES:
                drawFavoritesStaticUI();
                break;
            case SCREEN_ADDPASS:
                drawAddPassStaticUI();
                break;
            case SCREEN_TOUCH_TEST:
                drawTouchTestStaticUI();
                break;
            case SCREEN_CALIBRATION:
                drawCalibrationStaticUI();
                break;
            case SCREEN_CALIBRATION_RESET:
                drawCalibrationResetStaticUI();
                break;
            case SCREEN_FACTORY_RESET:
                drawFactoryResetStaticUI();
                break;
            case SCREEN_ORIENTATION:
                drawOrientationStaticUI();
                break;
            case SCREEN_ADVANCED_OPTIONS:
                drawAdvancedOptionsStaticUI();
                break;
            case SCREEN_ADD_PASSWORD_MENU:
                drawAddPasswordMenuStaticUI();
                break;
            case SCREEN_ADD_PASSWORD_MANUAL:
                drawAddPasswordManualStaticUI();
                break;
            case SCREEN_ADD_PASSWORD_WEB:
                drawAddPasswordWebStaticUI();
                break;
            case SCREEN_PASSWORDS:
                drawPasswordsStaticUI();
                break;
            case SCREEN_PASSWORD_DETAIL:
                drawPasswordDetailStaticUI();
                break;
            case SCREEN_PASSWORD_EDIT:
                drawPasswordEditStaticUI();
                break;
            case SCREEN_DELETE_CONFIRM:
                drawDeleteConfirmStaticUI();
                break;
            case SCREEN_ABOUT:
                drawAboutStaticUI();
                break;
            case SCREEN_PIN_ENTRY:
                if (pinEntryScreen != nullptr) {
                    pinEntryScreen->draw();
                } else {
                    drawPINEntryStaticUI();
                }
                break;
            case SCREEN_CHANGE_PIN:
                if (changePINScreen != nullptr) {
                    changePINScreen->draw();
                } else {
                    drawChangePINStaticUI();
                }
                break;
            case SCREEN_WEBUI_STATUS:
                if (webUIStatusScreen != nullptr) {
                    webUIStatusScreen->draw();
                } else {
                    drawWebUIStatusStaticUI();
                }
                break;
        }
        
        forceFullRedraw = false;
        screenIsDirty = true;  // Force initial data update after static UI draw
        
        Serial.printf("[DEBUG_RENDER] Static UI complete on %s screen\n", getScreenName(currentScreen));
    }
    
    // SPECIAL CASE: These screens manage their own internal dirty flags
    // Always call their draw() methods so they can handle internal state changes
    if (currentScreen == SCREEN_CALIBRATION) {
        drawCalibrationDynamicData();  // Always render for calibration
    } else if (currentScreen == SCREEN_CALIBRATION_RESET || currentScreen == SCREEN_FACTORY_RESET || 
               currentScreen == SCREEN_ORIENTATION || currentScreen == SCREEN_ADVANCED_OPTIONS ||
               currentScreen == SCREEN_ADD_PASSWORD_MENU || currentScreen == SCREEN_ADD_PASSWORD_MANUAL ||
               currentScreen == SCREEN_ADD_PASSWORD_WEB || currentScreen == SCREEN_PASSWORDS ||
               currentScreen == SCREEN_PASSWORD_DETAIL || currentScreen == SCREEN_FAVORITES ||
               currentScreen == SCREEN_PASSWORD_EDIT || currentScreen == SCREEN_DELETE_CONFIRM ||
               currentScreen == SCREEN_ABOUT || currentScreen == SCREEN_SETTINGS ||
               currentScreen == SCREEN_PIN_ENTRY || currentScreen == SCREEN_CHANGE_PIN ||
               currentScreen == SCREEN_WEBUI_STATUS || currentScreen == SCREEN_LOCK) {
        // These screens manage their own rendering
        if (currentScreen == SCREEN_CALIBRATION_RESET) {
            drawCalibrationResetDynamicData();
        } else if (currentScreen == SCREEN_FACTORY_RESET) {
            drawFactoryResetDynamicData();
        } else if (currentScreen == SCREEN_ORIENTATION) {
            drawOrientationDynamicData();
        } else if (currentScreen == SCREEN_ADVANCED_OPTIONS) {
            drawAdvancedOptionsDynamicData();
        } else if (currentScreen == SCREEN_ADD_PASSWORD_MENU) {
            drawAddPasswordMenuDynamicData();
        } else if (currentScreen == SCREEN_ADD_PASSWORD_MANUAL) {
            drawAddPasswordManualDynamicData();
        } else if (currentScreen == SCREEN_ADD_PASSWORD_WEB) {
            drawAddPasswordWebDynamicData();
        } else if (currentScreen == SCREEN_PASSWORDS) {
            drawPasswordsDynamicData();
        } else if (currentScreen == SCREEN_PASSWORD_DETAIL) {
            drawPasswordDetailDynamicData();
        } else if (currentScreen == SCREEN_FAVORITES) {
            drawFavoritesDynamicData();
        } else if (currentScreen == SCREEN_PASSWORD_EDIT) {
            drawPasswordEditDynamicData();
        } else if (currentScreen == SCREEN_DELETE_CONFIRM) {
            drawDeleteConfirmDynamicData();
        } else if (currentScreen == SCREEN_ABOUT) {
            drawAboutDynamicData();
        } else if (currentScreen == SCREEN_SETTINGS) {
            drawSettingsDynamicData();
        } else if (currentScreen == SCREEN_WEBUI_STATUS) {
            drawWebUIStatusDynamicData();
        } else if (currentScreen == SCREEN_LOCK) {
            drawLockDynamicData();
        }
    } else if (screenIsDirty) {
        Serial.printf("[DEBUG_RENDER] Dynamic data update on %s screen\n", getScreenName(currentScreen));
        
        // Draw dynamic data based on current screen
        switch (currentScreen) {
            case SCREEN_HOME:
                drawHomeDynamicData();
                break;
            case SCREEN_SETTINGS:
                // Handled in special case above
                break;
            case SCREEN_INFO:
                drawInfoDynamicData();
                break;
            case SCREEN_FAVORITES:
                // Handled in special case above
                break;
            case SCREEN_ADDPASS:
                drawAddPassDynamicData();
                break;
            case SCREEN_TOUCH_TEST:
                drawTouchTestDynamicData();
                break;
            case SCREEN_CALIBRATION:
                // Handled in special case above
                break;
            case SCREEN_CALIBRATION_RESET:
                // Handled in special case above
                break;
            case SCREEN_FACTORY_RESET:
                // Handled in special case above
                break;
            case SCREEN_ORIENTATION:
                // Handled in special case above
                break;
            case SCREEN_ADVANCED_OPTIONS:
                // Handled in special case above
                break;
            case SCREEN_ADD_PASSWORD_MENU:
                // Handled in special case above
                break;
            case SCREEN_ADD_PASSWORD_MANUAL:
                // Handled in special case above
                break;
            case SCREEN_ADD_PASSWORD_WEB:
                // Handled in special case above
                break;
            case SCREEN_PASSWORDS:
                // Handled in special case above
                break;
            case SCREEN_PASSWORD_DETAIL:
                // Handled in special case above
                break;
        }
        
        screenIsDirty = false;
    }
}

// ============================================================================
// HOME SCREEN - Static UI + Dynamic Data
// ============================================================================

void ScreenManager::drawHomeStaticUI() {
    // Draw title
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print("HOME SCREEN");
    
    // Draw Add Pass button (top left) - Unified dark theme
    tft->fillRoundRect(ADDPASS_BTN_X, ADDPASS_BTN_Y, ADDPASS_BTN_W, ADDPASS_BTN_H, 6, 0x18C3);  // COLOR_SURFACE
    tft->drawRoundRect(ADDPASS_BTN_X, ADDPASS_BTN_Y, ADDPASS_BTN_W, ADDPASS_BTN_H, 6, 0x2945);  // COLOR_LINE
    tft->setTextColor(0xFFDE, 0x18C3);  // COLOR_IVORY on COLOR_SURFACE
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Add Pass", ADDPASS_BTN_X + ADDPASS_BTN_W/2, ADDPASS_BTN_Y + ADDPASS_BTN_H/2);
    
    // Draw Passwords button (top right) - Unified dark theme
    tft->fillRoundRect(PASSWORDS_BTN_X, PASSWORDS_BTN_Y, PASSWORDS_BTN_W, PASSWORDS_BTN_H, 6, 0x18C3);
    tft->drawRoundRect(PASSWORDS_BTN_X, PASSWORDS_BTN_Y, PASSWORDS_BTN_W, PASSWORDS_BTN_H, 6, 0x2945);
    tft->setTextColor(0xFFDE, 0x18C3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Passwords", PASSWORDS_BTN_X + PASSWORDS_BTN_W/2, PASSWORDS_BTN_Y + PASSWORDS_BTN_H/2);
    
    // Draw Favorites button (bottom left) - Unified dark theme
    tft->fillRoundRect(FAVORITES_BTN_X, FAVORITES_BTN_Y, FAVORITES_BTN_W, FAVORITES_BTN_H, 6, 0x18C3);
    tft->drawRoundRect(FAVORITES_BTN_X, FAVORITES_BTN_Y, FAVORITES_BTN_W, FAVORITES_BTN_H, 6, 0xFD60);  // Brass border
    tft->setTextColor(0xFFDE, 0x18C3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Favorites", FAVORITES_BTN_X + FAVORITES_BTN_W/2, FAVORITES_BTN_Y + FAVORITES_BTN_H/2);
    
    // Draw Settings button (bottom right) - Unified dark theme
    tft->fillRoundRect(SETTINGS_BTN_X, SETTINGS_BTN_Y, SETTINGS_BTN_W, SETTINGS_BTN_H, 6, 0x18C3);
    tft->drawRoundRect(SETTINGS_BTN_X, SETTINGS_BTN_Y, SETTINGS_BTN_W, SETTINGS_BTN_H, 6, 0x2945);
    tft->setTextColor(0xFFDE, 0x18C3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Settings", SETTINGS_BTN_X + SETTINGS_BTN_W/2, SETTINGS_BTN_Y + SETTINGS_BTN_H/2);
    
    // Reset text datum
    tft->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("Home - Static UI drawn");
}

void ScreenManager::drawHomeDynamicData() {
    // Home screen has no dynamic data currently
    // This is where you'd draw changing values like password count, etc.
    DEBUG_LOG("Home - Dynamic data drawn");
}

void ScreenManager::updateHomeData() {
    // Update home screen data here (NO DRAWING)
    // Set screenIsDirty = true if data changes
}

void ScreenManager::handleHomeTouch(const TouchPoint& point) {
    Serial.println("[DEBUG_BUTTON] Home screen - checking button hits");
    
    // Add Pass button - Navigate to Add Password Menu
    if (point.x >= ADDPASS_BTN_X && point.x <= (ADDPASS_BTN_X + ADDPASS_BTN_W) && 
        point.y >= ADDPASS_BTN_Y && point.y <= (ADDPASS_BTN_Y + ADDPASS_BTN_H)) {
        printButtonDebug("Add Pass", "HOME", true);
        pushScreen(SCREEN_ADD_PASSWORD_MENU);
        return;
    }
    // Passwords button
    else if (point.x >= PASSWORDS_BTN_X && point.x <= (PASSWORDS_BTN_X + PASSWORDS_BTN_W) && 
             point.y >= PASSWORDS_BTN_Y && point.y <= (PASSWORDS_BTN_Y + PASSWORDS_BTN_H)) {
        printButtonDebug("Passwords", "HOME", true);
        pushScreen(SCREEN_PASSWORDS);  // Navigate to Passwords list
        return;
    }
    // Favorites button
    else if (point.x >= FAVORITES_BTN_X && point.x <= (FAVORITES_BTN_X + FAVORITES_BTN_W) && 
             point.y >= FAVORITES_BTN_Y && point.y <= (FAVORITES_BTN_Y + FAVORITES_BTN_H)) {
        printButtonDebug("Favorites", "HOME", true);
        pushScreen(SCREEN_FAVORITES);
        return;
    }
    // Settings button
    else if (point.x >= SETTINGS_BTN_X && point.x <= (SETTINGS_BTN_X + SETTINGS_BTN_W) && 
             point.y >= SETTINGS_BTN_Y && point.y <= (SETTINGS_BTN_Y + SETTINGS_BTN_H)) {
        printButtonDebug("Settings", "HOME", true);
        pushScreen(SCREEN_SETTINGS);
        return;
    }
    
    // No button hit
    Serial.printf("[DEBUG_BUTTON] No button hit at (%d,%d) on HOME screen\n", point.x, point.y);
    Serial.println("[DEBUG_BUTTON] Valid areas:");
    Serial.printf("  Add Pass: [%d-%d, %d-%d]\n", ADDPASS_BTN_X, ADDPASS_BTN_X+ADDPASS_BTN_W, ADDPASS_BTN_Y, ADDPASS_BTN_Y+ADDPASS_BTN_H);
    Serial.printf("  Passwords: [%d-%d, %d-%d]\n", PASSWORDS_BTN_X, PASSWORDS_BTN_X+PASSWORDS_BTN_W, PASSWORDS_BTN_Y, PASSWORDS_BTN_Y+PASSWORDS_BTN_H);
    Serial.printf("  Favorites: [%d-%d, %d-%d]\n", FAVORITES_BTN_X, FAVORITES_BTN_X+FAVORITES_BTN_W, FAVORITES_BTN_Y, FAVORITES_BTN_Y+FAVORITES_BTN_H);
    Serial.printf("  Settings: [%d-%d, %d-%d]\n", SETTINGS_BTN_X, SETTINGS_BTN_X+SETTINGS_BTN_W, SETTINGS_BTN_Y, SETTINGS_BTN_Y+SETTINGS_BTN_H);
}

// ============================================================================
// SETTINGS SCREEN - List-based settings with modern design
// ============================================================================

void ScreenManager::drawSettingsStaticUI() {
    // Lazy init Settings List screen
    if (settingsListScreen == nullptr) {
        settingsListScreen = new SettingsListScreen(tft, touch, settingsManager);
        settingsListScreen->begin();
    }
    
    // Reset screen state on entry
    settingsListScreen->reset();
    
    DEBUG_LOG("Settings - Initialized for static UI draw");
}

void ScreenManager::drawSettingsDynamicData() {
    if (settingsListScreen != nullptr) {
        settingsListScreen->draw();
    }
}

void ScreenManager::updateSettingsData() {
    if (settingsListScreen == nullptr) return;
    
    // Update screen logic
    settingsListScreen->update();
    
    // Check if user wants to exit
    if (settingsListScreen->needsToExit()) {
        DEBUG_LOG("Settings: Exit requested");
        popScreen(SCREEN_HOME);
        return;
    }
    
    // Check if user selected an action
    SettingsRow action = settingsListScreen->getSelectedAction();
    if (action != SETTINGS_ROW_NONE) {
        DEBUG_LOGF("Settings: Action selected: %d\n", action);
        
        switch (action) {
            case SETTINGS_ROW_ADVANCED:
                DEBUG_LOG("Navigating to Advanced Options");
                pushScreen(SCREEN_ADVANCED_OPTIONS);
                break;
            case SETTINGS_ROW_ABOUT:
                DEBUG_LOG("Navigating to About");
                pushScreen(SCREEN_ABOUT);
                break;
            case SETTINGS_ROW_CHANGE_PIN:
                DEBUG_LOG("Navigating to Change PIN");
                pushScreen(SCREEN_CHANGE_PIN);
                break;
            default:
                break;
        }
        
        // Clear the action after handling
        settingsListScreen->clearAction();
    }
}

void ScreenManager::handleSettingsTouch(const TouchPoint& point) {
    if (settingsListScreen != nullptr) {
        settingsListScreen->onTouchEvent(point);
    }
}

// ============================================================================
// INFO SCREEN (Passwords) - Static UI + Dynamic Data
// ============================================================================

void ScreenManager::drawInfoStaticUI() {
    // Draw title
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print("PASSWORDS");
    
    // Draw static labels
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    tft->setCursor(10, 50);
    tft->print("Password Manager");
    tft->setCursor(10, 62);
    tft->print("Secure Storage");
    tft->setCursor(10, 74);
    tft->print("Touch: XPT2046");
    tft->setCursor(10, 86);
    tft->print("Event-Driven");
    
    // Draw Back button
    tft->fillRect(20, 180, 100, 40, TFT_MAROON);
    tft->drawRect(20, 180, 100, 40, TFT_RED);
    tft->setTextColor(TFT_WHITE, TFT_MAROON);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Back", 70, 200);
    
    // Reset text datum
    tft->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("Info - Static UI drawn");
}

void ScreenManager::drawInfoDynamicData() {
    // Info screen has no dynamic data currently
    DEBUG_LOG("Info - Dynamic data drawn");
}

void ScreenManager::updateInfoData() {
    // Update info data here (NO DRAWING)
}

void ScreenManager::handleInfoTouch(const TouchPoint& point) {
    Serial.println("[DEBUG_BUTTON] Info screen - checking button hits");
    
    // Back button
    if (point.x >= 20 && point.x <= 120 && point.y >= 180 && point.y <= 220) {
        printButtonDebug("Back", "INFO", true);
        popScreen(SCREEN_HOME);
        return;
    }
    
    // No button hit
    Serial.printf("[DEBUG_BUTTON] No button hit at (%d,%d) on INFO screen\n", point.x, point.y);
    Serial.println("[DEBUG_BUTTON] Valid areas:");
    Serial.println("  Back: [20-120, 180-220]");
}

// ============================================================================
// FAVORITES SCREEN - List of favorited passwords
// ============================================================================

void ScreenManager::drawFavoritesStaticUI() {
    // Lazy init Favorites screen
    if (favoritesScreen == nullptr) {
        favoritesScreen = new FavoritesScreen(tft, touch, vaultStorage);
        favoritesScreen->begin();
    }
    
    // Reset screen state on entry
    favoritesScreen->reset();
    
    DEBUG_LOG("Favorites - Initialized for static UI draw");
}

void ScreenManager::drawFavoritesDynamicData() {
    if (favoritesScreen != nullptr) {
        favoritesScreen->draw();
    }
}

void ScreenManager::updateFavoritesData() {
    if (favoritesScreen == nullptr) return;
    
    // Update screen logic
    favoritesScreen->update();
    
    // Check if user wants to exit
    if (favoritesScreen->needsToExit()) {
        DEBUG_LOG("Favorites: Exit requested");
        popScreen(SCREEN_HOME);
        return;
    }
    
    // Check if user selected a password to view
    if (favoritesScreen->hasSelection()) {
        uint32_t selectedId = favoritesScreen->getSelectedPasswordId();
        DEBUG_LOGF("Favorites: Password ID %d selected\n", selectedId);
        
        // Set the password ID for detail screen
        if (passwordDetailScreen == nullptr) {
            passwordDetailScreen = new PasswordDetailScreen(tft, touch, vaultStorage);
            passwordDetailScreen->begin();
        }
        passwordDetailScreen->setPasswordId(selectedId);
        
        pushScreen(SCREEN_PASSWORD_DETAIL);
        return;
    }
}

void ScreenManager::handleFavoritesTouch(const TouchPoint& point) {
    if (favoritesScreen != nullptr) {
        favoritesScreen->onTouchEvent(point);
    }
}

// ============================================================================
// ADD PASSWORD SCREEN - Static UI + Dynamic Data (OLD - TO BE DEPRECATED)
// ============================================================================

void ScreenManager::drawAddPassStaticUI() {
    // This old ADDPASS screen is now replaced by ADD_PASSWORD_MENU
    // Keeping for backwards compatibility during transition
    // Draw title
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 10);
    tft->print("ADD PASSWORD");
    
    // Draw Back button
    tft->fillRect(20, 180, 100, 40, TFT_MAROON);
    tft->drawRect(20, 180, 100, 40, TFT_RED);
    tft->setTextColor(TFT_WHITE, TFT_MAROON);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Back", 70, 200);
    
    // Reset text datum
    tft->setTextDatum(TL_DATUM);
    
    DEBUG_LOG("AddPass - Static UI drawn");
}

void ScreenManager::drawAddPassDynamicData() {
    // Add password screen has no dynamic data currently
    // This is where you'd draw input fields and virtual keyboard
    DEBUG_LOG("AddPass - Dynamic data drawn");
}

void ScreenManager::updateAddPassData() {
    // Update add password data here (NO DRAWING)
}

void ScreenManager::handleAddPassTouch(const TouchPoint& point) {
    Serial.println("[DEBUG_BUTTON] AddPass screen - checking button hits");
    
    // Back button
    if (point.x >= 20 && point.x <= 120 && point.y >= 180 && point.y <= 220) {
        printButtonDebug("Back", "ADDPASS", true);
        popScreen(SCREEN_HOME);
        return;
    }
    
    // No button hit
    Serial.printf("[DEBUG_BUTTON] No button hit at (%d,%d) on ADDPASS screen\n", point.x, point.y);
    Serial.println("[DEBUG_BUTTON] Valid areas:");
    Serial.println("  Back: [20-120, 180-220]");
}

// ============================================================================
// ADD PASSWORD MENU SCREEN - New system with method selection
// ============================================================================

void ScreenManager::drawAddPasswordMenuStaticUI() {
    // Lazy init Add Password Menu screen
    if (addPasswordMenuScreen == nullptr) {
        addPasswordMenuScreen = new AddPasswordMenuScreen(tft, touch);
        addPasswordMenuScreen->begin();
    }
    
    // Reset screen state on entry
    addPasswordMenuScreen->reset();
    
    DEBUG_LOG("AddPasswordMenu - Initialized for static UI draw");
}

void ScreenManager::drawAddPasswordMenuDynamicData() {
    if (addPasswordMenuScreen != nullptr) {
        addPasswordMenuScreen->draw();
    }
}

void ScreenManager::updateAddPasswordMenuData() {
    if (addPasswordMenuScreen == nullptr) return;
    
    // Update screen logic
    addPasswordMenuScreen->update();
    
    // Check if user wants to exit
    if (addPasswordMenuScreen->needsToExit()) {
        DEBUG_LOG("AddPasswordMenu: Exit requested");
        popScreen(SCREEN_HOME);
        return;
    }
    
    // Check if user selected a method
    AddPasswordMethod method = addPasswordMenuScreen->getSelectedMethod();
    if (method != METHOD_NONE) {
        DEBUG_LOGF("AddPasswordMenu: Method %d selected\n", method);
        
        if (method == METHOD_MANUAL) {
            pushScreen(SCREEN_ADD_PASSWORD_MANUAL);
        } else if (method == METHOD_WEBUI_CONTROL) {
            DEBUG_LOG("Navigating to Web UI Status from Add Password menu");
            pushScreen(SCREEN_WEBUI_STATUS);
        }
        
        // Clear selection after handling
        addPasswordMenuScreen->clearSelection();
    }
}

void ScreenManager::handleAddPasswordMenuTouch(const TouchPoint& point) {
    if (addPasswordMenuScreen != nullptr) {
        addPasswordMenuScreen->onTouchEvent(point);
    }
}

// ============================================================================
// ADD PASSWORD MANUAL SCREEN - Manual entry form
// ============================================================================

void ScreenManager::drawAddPasswordManualStaticUI() {
    // Lazy init Add Password Manual screen
    if (addPasswordManualScreen == nullptr) {
        addPasswordManualScreen = new AddPasswordManualScreen(tft, touch);
        addPasswordManualScreen->begin();
    }
    
    // Reset screen state on entry
    addPasswordManualScreen->reset();
    
    DEBUG_LOG("AddPasswordManual - Initialized for static UI draw");
}

void ScreenManager::drawAddPasswordManualDynamicData() {
    if (addPasswordManualScreen != nullptr) {
        addPasswordManualScreen->draw();
    }
}

void ScreenManager::updateAddPasswordManualData() {
    if (addPasswordManualScreen == nullptr) return;
    
    // Update screen logic
    addPasswordManualScreen->update();
    
    // Check if password was saved FIRST (before checking needsToExit)
    if (addPasswordManualScreen->wasPasswordSaved()) {
        Serial.println("[ADD_MANUAL] Password saved!");
        
        const PasswordEntry& savedEntry = addPasswordManualScreen->getPasswordData();
        Serial.printf("[ADD_MANUAL] Title: %s\n", savedEntry.title);
        Serial.printf("[ADD_MANUAL] User: %s\n", savedEntry.user);
        Serial.println("[ADD_MANUAL] Pass: [REDACTED]");
        
        // Save to vault storage
        if (vaultStorage) {
            if (vaultStorage->addPassword(savedEntry)) {
                Serial.println("[ADD_MANUAL] Password saved to vault successfully!");
            } else {
                Serial.println("[ADD_MANUAL] ERROR: Failed to save password to vault!");
            }
        } else {
            Serial.println("[ADD_MANUAL] ERROR: Vault storage not initialized!");
        }
        
        // Return to home after save
        popScreen(SCREEN_HOME);
        return;
    }
    
    // Check if user wants to exit/cancel
    if (addPasswordManualScreen->needsToExit()) {
        DEBUG_LOG("AddPasswordManual: Exit requested (cancelled)");
        popScreen(SCREEN_ADD_PASSWORD_MENU);
        return;
    }
}

void ScreenManager::handleAddPasswordManualTouch(const TouchPoint& point) {
    if (addPasswordManualScreen != nullptr) {
        addPasswordManualScreen->onTouchEvent(point);
    }
}

// ============================================================================
// ADD PASSWORD WEB UI SCREEN - Web UI placeholder
// ============================================================================

void ScreenManager::drawAddPasswordWebStaticUI() {
    // Lazy init Add Password Web screen
    if (addPasswordWebScreen == nullptr) {
        addPasswordWebScreen = new AddPasswordWebScreen(tft, touch);
        addPasswordWebScreen->begin();
    }
    
    // Reset screen state on entry
    addPasswordWebScreen->reset();
    
    DEBUG_LOG("AddPasswordWeb - Initialized for static UI draw");
}

void ScreenManager::drawAddPasswordWebDynamicData() {
    if (addPasswordWebScreen != nullptr) {
        addPasswordWebScreen->draw();
    }
}

void ScreenManager::updateAddPasswordWebData() {
    if (addPasswordWebScreen == nullptr) return;
    
    // Update screen logic
    addPasswordWebScreen->update();
    
    // Check if user wants to exit
    if (addPasswordWebScreen->needsToExit()) {
        DEBUG_LOG("AddPasswordWeb: Exit requested");
        popScreen(SCREEN_ADD_PASSWORD_MENU);
        return;
    }
}

void ScreenManager::handleAddPasswordWebTouch(const TouchPoint& point) {
    if (addPasswordWebScreen != nullptr) {
        addPasswordWebScreen->onTouchEvent(point);
    }
}

// ============================================================================
// TOUCH TEST SCREEN - Static UI + Dynamic Data (SUCCESS PATTERN)
// ============================================================================

void ScreenManager::drawTouchTestStaticUI() {
    // Draw Back button
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    tft->setCursor(10, 15);
    tft->print("< Back");
    
    // Draw title
    tft->setTextSize(2);
    tft->setCursor(100, 10);
    tft->print("Touch Test");
    
    // Draw instructions
    tft->setTextColor(0x8410, TFT_BLACK); // Gray
    tft->setTextSize(1);
    tft->setCursor(70, 40);
    tft->print("Touch anywhere on screen");
    tft->setCursor(60, 55);
    tft->print("Watch coordinates and dot move");
    
    // Draw static labels
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setCursor(10, 80);
    tft->print("X:");
    tft->setCursor(10, 100);
    tft->print("Y:");
    tft->setCursor(10, 120);
    tft->print("Pressure:");
    
    DEBUG_LOG("Touch Test - Static UI drawn");
}

void ScreenManager::drawTouchTestDynamicData() {
    // Persistent render tracking (NOT shared state)
    static int16_t lastDotX = -1;
    static int16_t lastDotY = -1;
    static int16_t lastDisplayX = -1;
    static int16_t lastDisplayY = -1;
    static int16_t lastDisplayZ = -1;
    
    // Read from shared state (set by updateTouchTestData)
    int16_t currentX = sharedTouchX;
    int16_t currentY = sharedTouchY;
    int16_t currentZ = sharedTouchP;
    bool isActive = sharedTouchActive;
    
    if (isActive) {
        // Update coordinates (difference-only)
        char buf[16];
        
        if (currentX != lastDisplayX) {
            tft->fillRect(90, 80, 80, 8, TFT_BLACK);
            tft->setTextColor(TFT_GREEN, TFT_BLACK);
            tft->setTextSize(1);
            tft->setCursor(90, 80);
            snprintf(buf, sizeof(buf), "%d", currentX);
            tft->print(buf);
            lastDisplayX = currentX;
        }
        
        if (currentY != lastDisplayY) {
            tft->fillRect(90, 100, 80, 8, TFT_BLACK);
            tft->setTextColor(TFT_GREEN, TFT_BLACK);
            tft->setTextSize(1);
            tft->setCursor(90, 100);
            snprintf(buf, sizeof(buf), "%d", currentY);
            tft->print(buf);
            lastDisplayY = currentY;
        }
        
        if (currentZ != lastDisplayZ) {
            tft->fillRect(90, 120, 80, 8, TFT_BLACK);
            tft->setTextColor(TFT_GREEN, TFT_BLACK);
            tft->setTextSize(1);
            tft->setCursor(90, 120);
            snprintf(buf, sizeof(buf), "%d", currentZ);
            tft->print(buf);
            lastDisplayZ = currentZ;
        }
        
        // Update dot (unified radius, no halo)
        if (currentY >= 70) {
            // Clear old dot if position changed
            if (lastDotX >= 0 && lastDotY >= 70 && 
                (currentX != lastDotX || currentY != lastDotY)) {
                tft->fillCircle(lastDotX, lastDotY, DOT_CLEAR_RADIUS, 0x0841);  // COLOR_INK
            }
            
            // Draw new dot with brass accent (unified theme)
            tft->fillCircle(currentX, currentY, DOT_FILL_RADIUS, 0xFD60);  // COLOR_BRASS
            tft->drawCircle(currentX, currentY, DOT_OUTLINE_RADIUS, 0xFFDE);  // COLOR_IVORY
            
            lastDotX = currentX;
            lastDotY = currentY;
        } else {
            // Touch in header - clear dot
            if (lastDotX >= 0 && lastDotY >= 70) {
                tft->fillCircle(lastDotX, lastDotY, DOT_CLEAR_RADIUS, TFT_BLACK);
                lastDotX = -1;
                lastDotY = -1;
            }
        }
        
    } else {
        // No touch display
        tft->fillRect(90, 80, 80, 8, TFT_BLACK);
        tft->fillRect(90, 100, 80, 8, TFT_BLACK);
        tft->fillRect(90, 120, 80, 8, TFT_BLACK);
        
        tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        tft->setTextSize(1);
        tft->setCursor(90, 100);
        tft->print("No touch");
        
        if (lastDotX >= 0 && lastDotY >= 70) {
            tft->fillCircle(lastDotX, lastDotY, DOT_CLEAR_RADIUS, TFT_BLACK);
            lastDotX = -1;
            lastDotY = -1;
        }
        
        lastDisplayX = -1;
        lastDisplayY = -1;
        lastDisplayZ = -1;
    }
    
    DEBUG_LOG("Touch Test - Dynamic data drawn");
}

void ScreenManager::updateTouchTestData() {
    // Persistent tracking variables for deadzone/jitter prevention
    static int16_t lastSampleX = -1;
    static int16_t lastSampleY = -1;
    static int16_t lastSampleP = -1;
    static bool lastTouchState = false;
    
    // INPUT PHASE: Read touch sensor (NO DRAWING)
    digitalWrite(TFT_CS, HIGH);  // Ensure TFT_CS high before touch read
    bool currentlyTouched = ts.touched();
    
    if (currentlyTouched) {
        TS_Point p = ts.getPoint();
        
        // Map to screen coordinates
        int16_t newX = map(p.x, TOUCH_CAL_X_MIN, TOUCH_CAL_X_MAX, 0, SCREEN_WIDTH);
        int16_t newY = map(p.y, TOUCH_CAL_Y_MIN, TOUCH_CAL_Y_MAX, 0, SCREEN_HEIGHT);
        newX = constrain(newX, 0, SCREEN_WIDTH - 1);
        newY = constrain(newY, 0, SCREEN_HEIGHT - 1);
        
        // JITTER PREVENTION: 2-pixel deadzone
        int16_t deltaX = abs(newX - lastSampleX);
        int16_t deltaY = abs(newY - lastSampleY);
        int16_t deltaP = abs(p.z - lastSampleP);
        
        // Determine if update is needed
        bool positionChanged = (deltaX > DOT_MOVEMENT_THRESHOLD || deltaY > DOT_MOVEMENT_THRESHOLD);
        bool pressureChanged = (deltaP > 10);
        bool stateChanged = !lastTouchState;
        
        if (positionChanged || pressureChanged || stateChanged) {
            // STATE UPDATE ONLY - NO DRAWING
            sharedTouchX = newX;
            sharedTouchY = newY;
            sharedTouchP = p.z;
            sharedTouchActive = true;
            screenIsDirty = true;  // Mark for render
            
            // Update tracking
            if (positionChanged || stateChanged) {
                lastSampleX = newX;
                lastSampleY = newY;
            }
            if (pressureChanged || stateChanged) {
                lastSampleP = p.z;
            }
        }
        
        lastTouchState = true;
        
    } else {
        // No touch - only process if state changed
        if (lastTouchState) {
            // STATE UPDATE ONLY - NO DRAWING
            sharedTouchActive = false;
            screenIsDirty = true;  // Mark for "No touch" display
            
            // Reset tracking
            lastSampleX = -1;
            lastSampleY = -1;
            lastSampleP = -1;
            lastTouchState = false;
        }
    }
}

void ScreenManager::handleTouchTestTouch(const TouchPoint& point) {
    Serial.println("[DEBUG_BUTTON] TouchTest screen - checking button hits");
    
    // Back button: top-left corner [0-80], y[0-40]
    if (point.x >= 0 && point.x <= 80 && point.y >= 0 && point.y <= 40) {
        printButtonDebug("Back", "TOUCH_TEST", true);
        DEBUG_LOG("Back button pressed - returning to Advanced Options");
        popScreen(SCREEN_ADVANCED_OPTIONS);
        return;
    }
    
    // No button hit (touches in TouchTest area are expected - not logging as miss)
    if (point.y > 40) {
        // Touch in test area - not a button, this is normal
        return;
    }
    
    Serial.printf("[DEBUG_BUTTON] No button hit at (%d,%d) on TOUCH_TEST screen\n", point.x, point.y);
    Serial.println("[DEBUG_BUTTON] Valid areas:");
    Serial.println("  Back: [0-80, 0-40]");
}

// ============================================================================
// CALIBRATION SCREEN - Delegate to CalibrationWizardScreen
// ============================================================================

void ScreenManager::drawCalibrationStaticUI() {
    // Create calibration wizard on first access
    if (calibrationWizard == nullptr) {
        calibrationWizard = new CalibrationWizardScreen(tft, touch);
        calibrationWizard->begin();
        DEBUG_LOG("CalibrationWizard created");
    }
    
    // Wizard handles its own rendering
    // Call draw() here to handle the wizard's forceFullRedraw
    if (calibrationWizard != nullptr) {
        calibrationWizard->draw();
    }
    
    DEBUG_LOG("Calibration - Static UI (delegated to wizard)");
}

void ScreenManager::drawCalibrationDynamicData() {
    // ALWAYS call wizard draw() - it manages its own internal dirty flags
    // This ensures the wizard can update itself when state changes internally
    if (calibrationWizard != nullptr) {
        calibrationWizard->draw();
    }
    
    DEBUG_LOG("Calibration - Dynamic data (delegated to wizard)");
}

void ScreenManager::updateCalibrationData() {
    // Update calibration wizard logic
    if (calibrationWizard != nullptr) {
        calibrationWizard->update();
        
        // Check if user wants to exit (Back button pressed)
        if (calibrationWizard->needsToExit()) {
            DEBUG_LOG("Calibration wizard - user pressed Back, exiting");
            
            // Clean up wizard
            delete calibrationWizard;
            calibrationWizard = nullptr;
            
            // Return to Advanced Options
            popScreen(SCREEN_ADVANCED_OPTIONS);
            return;
        }
        
        // Check if calibration complete
        if (calibrationWizard->isComplete()) {
            DEBUG_LOG("Calibration complete - returning to Settings");
            
            // Get calibration data
            CalibrationData calData = calibrationWizard->getCalibrationData();
            
            // TODO: Save calibration data to EEPROM/SPIFFS
            // saveCalibrationData(calData);
            
            // TODO: Apply calibration to TouchManager
            // applyCalibration(calData);
            
            // Print results for now
            Serial.println("\n✓ Calibration Results:");
            for (int i = 0; i < 5; i++) {
                Serial.printf("  Point %d: raw(%d,%d) -> screen(%d,%d)\n",
                             i, calData.rawX[i], calData.rawY[i],
                             calData.screenX[i], calData.screenY[i]);
            }
            
            // Clean up wizard
            delete calibrationWizard;
            calibrationWizard = nullptr;
            
            // Return to Advanced Options
            popScreen(SCREEN_ADVANCED_OPTIONS);
        }
    }
}

void ScreenManager::handleCalibrationTouch(const TouchPoint& point) {
    // Delegate touch handling to wizard
    if (calibrationWizard != nullptr) {
        calibrationWizard->onTouchEvent(point);
    }
}



// ============================================================================
// CALIBRATION RESET SCREEN - Delegates to CalibrationResetScreen class
// ============================================================================

void ScreenManager::drawCalibrationResetStaticUI() {
    // Create screen on first access
    if (calibrationResetScreen == nullptr) {
        calibrationResetScreen = new CalibrationResetScreen(tft, touch, settingsManager);
        calibrationResetScreen->begin();
        DEBUG_LOG("CalibrationResetScreen created");
    }
    
    // Delegate drawing to the screen class
    if (calibrationResetScreen != nullptr) {
        calibrationResetScreen->draw();
    }
    
    DEBUG_LOG("CalibrationReset - Static UI (delegated)");
}

void ScreenManager::drawCalibrationResetDynamicData() {
    // Always call draw() - screen manages its own rendering
    if (calibrationResetScreen != nullptr) {
        calibrationResetScreen->draw();
    }
    
    DEBUG_LOG("CalibrationReset - Dynamic data (delegated)");
}

void ScreenManager::updateCalibrationResetData() {
    // Check if user wants to exit
    if (calibrationResetScreen != nullptr && calibrationResetScreen->needsToExit()) {
        popScreen(SCREEN_ADVANCED_OPTIONS);  // Return to Advanced Options
        calibrationResetScreen->reset();  // Clear exit flag
    }
    
    // Call update on the screen
    if (calibrationResetScreen != nullptr) {
        calibrationResetScreen->update();
    }
}

void ScreenManager::handleCalibrationResetTouch(const TouchPoint& point) {
    // Delegate touch handling to the screen class
    if (calibrationResetScreen != nullptr) {
        calibrationResetScreen->onTouchEvent(point);
    }
}

// ============================================================================
// FACTORY RESET SCREEN - Delegates to FactoryResetScreen class
// ============================================================================

void ScreenManager::drawFactoryResetStaticUI() {
    // Create screen on first access
    if (factoryResetScreen == nullptr) {
        factoryResetScreen = new FactoryResetScreen(tft, touch, settingsManager);
        factoryResetScreen->begin();
        DEBUG_LOG("FactoryResetScreen created");
    }
    
    // Delegate drawing to the screen class
    if (factoryResetScreen != nullptr) {
        factoryResetScreen->draw();
    }
    
    DEBUG_LOG("FactoryReset - Static UI (delegated)");
}

void ScreenManager::drawFactoryResetDynamicData() {
    // Always call draw() - screen manages its own rendering
    if (factoryResetScreen != nullptr) {
        factoryResetScreen->draw();
    }
    
    DEBUG_LOG("FactoryReset - Dynamic data (delegated)");
}

void ScreenManager::updateFactoryResetData() {
    // Check if user wants to exit
    if (factoryResetScreen != nullptr && factoryResetScreen->needsToExit()) {
        popScreen(SCREEN_ADVANCED_OPTIONS);  // Return to Advanced Options
        factoryResetScreen->reset();  // Clear exit flag
    }
    
    // Call update on the screen
    if (factoryResetScreen != nullptr) {
        factoryResetScreen->update();
    }
}

void ScreenManager::handleFactoryResetTouch(const TouchPoint& point) {
    // Delegate touch handling to the screen class
    if (factoryResetScreen != nullptr) {
        factoryResetScreen->onTouchEvent(point);
    }
}

// ============================================================================
// ORIENTATION SCREEN - Static UI + Dynamic Data
// ============================================================================

void ScreenManager::drawOrientationStaticUI() {
    // Create screen on first access
    if (orientationScreen == nullptr) {
        orientationScreen = new OrientationScreen(tft, touch, settingsManager);
        orientationScreen->begin();
        DEBUG_LOG("OrientationScreen created");
    } else {
        orientationScreen->reset();
    }
    
    // Delegate static UI drawing to the screen class
    orientationScreen->draw();
    
    DEBUG_LOG("Orientation - Static UI drawn");
}

void ScreenManager::drawOrientationDynamicData() {
    // Delegate dynamic rendering to the screen class
    if (orientationScreen != nullptr) {
        orientationScreen->draw();
    }
}

void ScreenManager::updateOrientationData() {
    // Delegate updates to the screen class
    if (orientationScreen != nullptr) {
        orientationScreen->update();
        
        // Check if orientation was changed and apply it system-wide
        if (orientationScreen->orientationChanged()) {
            uint8_t newOrientation = settingsManager->getOrientation();
            
            // Update touch manager rotation
            touch->updateRotation(newOrientation);
            
            // Force full screen redraw with new orientation
            forceFullRedraw = true;
            
            DEBUG_LOGF("Orientation changed applied: %d\n", newOrientation);
        }
        
        // Check if user wants to exit
        if (orientationScreen->needsToExit()) {
            DEBUG_LOG("Exiting Orientation screen");
            popScreen(SCREEN_ADVANCED_OPTIONS);
        }
    }
}

void ScreenManager::handleOrientationTouch(const TouchPoint& point) {
    // Delegate touch handling to the screen class
    if (orientationScreen != nullptr) {
        orientationScreen->onTouchEvent(point);
    }
}

// ============================================================================
// ADVANCED OPTIONS SCREEN - Static UI + Dynamic Data
// ============================================================================

void ScreenManager::drawAdvancedOptionsStaticUI() {
    // Create screen on first access
    if (advancedOptionsScreen == nullptr) {
        advancedOptionsScreen = new AdvancedOptionsScreen(tft, touch, settingsManager);
        advancedOptionsScreen->begin();
        DEBUG_LOG("AdvancedOptionsScreen created");
    } else {
        advancedOptionsScreen->reset();
    }
    
    // Create FAB on first access
    if (scrollFAB == nullptr) {
        scrollFAB = new FloatingScrollButton(tft);
        scrollFAB->begin();
        DEBUG_LOG("FloatingScrollButton created");
    }
    
    // Delegate static UI drawing to the screen class
    advancedOptionsScreen->draw();
    
    DEBUG_LOG("AdvancedOptions - Static UI drawn");
}

void ScreenManager::drawAdvancedOptionsDynamicData() {
    // Delegate dynamic rendering to the screen class
    if (advancedOptionsScreen != nullptr) {
        advancedOptionsScreen->draw();
    }
    
    // Draw FAB on top of everything (ALWAYS LAST)
    if (scrollFAB != nullptr) {
        scrollFAB->draw();
    }
}

void ScreenManager::updateAdvancedOptionsData() {
    // Update FAB state
    if (scrollFAB != nullptr) {
        TouchPoint currentTouch = touch->getPoint();
        TouchState currentState = touch->getState();
        
        scrollFAB->update(currentTouch, currentState);
        
        // Check if FAB was tapped
        if (scrollFAB->wasTapped()) {
            DEBUG_LOG("FAB tapped - scrolling down");
            if (advancedOptionsScreen != nullptr) {
                advancedOptionsScreen->scrollDown();
            }
            scrollFAB->clearTap();
        }
    }
    
    // Delegate updates to the screen class
    if (advancedOptionsScreen != nullptr) {
        advancedOptionsScreen->update();
        
        // Check if user selected an action
        AdvancedOptionAction action = advancedOptionsScreen->getSelectedAction();
        if (action != ACTION_NONE) {
            DEBUG_LOGF("AdvancedOptions: Action selected: %d\n", action);
            
            // Navigate to the appropriate screen based on action
            switch (action) {
                case ACTION_TOUCH_TEST:
                    DEBUG_LOG("Navigating to Touch Test");
                    pushScreen(SCREEN_TOUCH_TEST);
                    break;
                case ACTION_CALIBRATION:
                    DEBUG_LOG("Navigating to Touch Calibration");
                    pushScreen(SCREEN_CALIBRATION);
                    break;
                case ACTION_ORIENTATION:
                    DEBUG_LOG("Navigating to Orientation");
                    pushScreen(SCREEN_ORIENTATION);
                    break;
                case ACTION_RESET_CALIBRATION:
                    DEBUG_LOG("Navigating to Reset Calibration");
                    pushScreen(SCREEN_CALIBRATION_RESET);
                    break;
                case ACTION_FACTORY_RESET:
                    DEBUG_LOG("Navigating to Factory Reset");
                    pushScreen(SCREEN_FACTORY_RESET);
                    break;
                default:
                    DEBUG_LOG("Unknown action");
                    break;
            }
            
            // Clear the action after handling
            advancedOptionsScreen->clearAction();
        }
        
        // Check if user wants to exit
        if (advancedOptionsScreen->needsToExit()) {
            DEBUG_LOG("Exiting Advanced Options screen");
            popScreen(SCREEN_SETTINGS);
        }
    }
}

void ScreenManager::handleAdvancedOptionsTouch(const TouchPoint& point) {
    // Check if touch is on FAB first (prevent menu item selection when touching FAB)
    if (scrollFAB != nullptr && scrollFAB->isTouchOnButton(point.x, point.y)) {
        // FAB handles its own touch in updateAdvancedOptionsData
        return;
    }
    
    // Delegate touch handling to the screen class
    if (advancedOptionsScreen != nullptr) {
        advancedOptionsScreen->onTouchEvent(point);
    }
}

// ============================================================================
// PASSWORDS SCREEN - List of all saved passwords
// ============================================================================

void ScreenManager::drawPasswordsStaticUI() {
    // Lazy init Passwords screen
    if (passwordsScreen == nullptr) {
        passwordsScreen = new PasswordsScreen(tft, touch, vaultStorage);
        passwordsScreen->begin();
    }
    
    // Reset screen state on entry
    passwordsScreen->reset();
    
    DEBUG_LOG("Passwords - Initialized for static UI draw");
}

void ScreenManager::drawPasswordsDynamicData() {
    if (passwordsScreen != nullptr) {
        passwordsScreen->draw();
    }
}

void ScreenManager::updatePasswordsData() {
    if (passwordsScreen == nullptr) return;
    
    // Update screen logic
    passwordsScreen->update();
    
    // Check if user wants to exit
    if (passwordsScreen->needsToExit()) {
        DEBUG_LOG("Passwords: Exit requested");
        popScreen(SCREEN_HOME);
        return;
    }
    
    // Check if user selected a password to view
    if (passwordsScreen->hasSelection()) {
        uint32_t selectedId = passwordsScreen->getSelectedPasswordId();
        DEBUG_LOGF("Passwords: Password ID %d selected\n", selectedId);
        
        // Set the password ID for detail screen
        if (passwordDetailScreen == nullptr) {
            passwordDetailScreen = new PasswordDetailScreen(tft, touch, vaultStorage);
            passwordDetailScreen->begin();
        }
        passwordDetailScreen->setPasswordId(selectedId);
        
        pushScreen(SCREEN_PASSWORD_DETAIL);
        return;
    }
}

void ScreenManager::handlePasswordsTouch(const TouchPoint& point) {
    if (passwordsScreen != nullptr) {
        passwordsScreen->onTouchEvent(point);
    }
}

// ============================================================================
// PASSWORD DETAIL SCREEN - View/edit/delete a single password
// ============================================================================

void ScreenManager::drawPasswordDetailStaticUI() {
    // Screen should already be initialized from Passwords screen
    if (passwordDetailScreen == nullptr) {
        DEBUG_LOG("PasswordDetail: Screen not initialized!");
        popScreen(SCREEN_PASSWORDS);
        return;
    }
    
    // Reset screen state on entry
    passwordDetailScreen->reset();
    
    DEBUG_LOG("PasswordDetail - Initialized for static UI draw");
}

void ScreenManager::drawPasswordDetailDynamicData() {
    if (passwordDetailScreen != nullptr) {
        passwordDetailScreen->draw();
    }
}

void ScreenManager::updatePasswordDetailData() {
    if (passwordDetailScreen == nullptr) return;
    
    // Update screen logic
    passwordDetailScreen->update();
    
    // Check if user wants to delete
    if (passwordDetailScreen->needsToDelete()) {
        uint32_t id = passwordDetailScreen->getCurrentPasswordId();
        DEBUG_LOGF("PasswordDetail: Delete password ID %d requested\n", id);
        
        // Get password title for confirmation dialog
        PasswordEntry entry = vaultStorage->getPassword(id);
        
        // Open delete confirmation dialog
        if (deleteConfirmDialog == nullptr) {
            deleteConfirmDialog = new DeleteConfirmDialog(tft, touch);
            deleteConfirmDialog->begin();
        }
        deleteConfirmDialog->setItemName(entry.title);
        
        pushScreen(SCREEN_DELETE_CONFIRM);
        return;
    }
    
    // Check if user wants to edit
    if (passwordDetailScreen->needsToEdit()) {
        uint32_t id = passwordDetailScreen->getCurrentPasswordId();
        DEBUG_LOGF("PasswordDetail: Edit password ID %d\n", id);
        
        // Get password data for editing
        PasswordEntry entry = vaultStorage->getPassword(id);
        
        // Open edit screen
        if (passwordEditScreen == nullptr) {
            passwordEditScreen = new PasswordEditScreen(tft, touch);
            passwordEditScreen->begin();
        }
        passwordEditScreen->setPasswordData(entry);
        
        pushScreen(SCREEN_PASSWORD_EDIT);
        
        // Clear the edit flag
        passwordDetailScreen->clearEditFlag();
        return;
    }
    
    // Check if user wants to exit
    if (passwordDetailScreen->needsToExit()) {
        DEBUG_LOG("PasswordDetail: Exit requested");
        popScreen(SCREEN_PASSWORDS);
        return;
    }
}

void ScreenManager::handlePasswordDetailTouch(const TouchPoint& point) {
    if (passwordDetailScreen != nullptr) {
        passwordDetailScreen->onTouchEvent(point);
    }
}

// ============================================================================
// PASSWORD EDIT SCREEN - Edit existing password
// ============================================================================

void ScreenManager::drawPasswordEditStaticUI() {
    // Lazy init Password Edit screen
    if (passwordEditScreen == nullptr) {
        passwordEditScreen = new PasswordEditScreen(tft, touch);
        passwordEditScreen->begin();
    }
    
    // Reset screen state on entry
    passwordEditScreen->reset();
    
    DEBUG_LOG("PasswordEdit - Initialized for static UI draw");
}

void ScreenManager::drawPasswordEditDynamicData() {
    if (passwordEditScreen != nullptr) {
        passwordEditScreen->draw();
    }
}

void ScreenManager::updatePasswordEditData() {
    if (passwordEditScreen == nullptr) return;
    
    // Update screen logic
    passwordEditScreen->update();
    
    // Check if password was saved
    if (passwordEditScreen->wasPasswordSaved()) {
        const PasswordEntry& updatedEntry = passwordEditScreen->getPasswordData();
        DEBUG_LOGF("PasswordEdit: Password saved: %s\n", updatedEntry.title);
        
        // Update in vault
        if (vaultStorage) {
            if (vaultStorage->updatePassword(updatedEntry.id, updatedEntry)) {
                Serial.println("[PASSWORD_EDIT] Password updated in vault successfully!");
            } else {
                Serial.println("[PASSWORD_EDIT] ERROR: Failed to update password in vault!");
            }
        }
        
        // Return to password detail view
        if (passwordDetailScreen != nullptr) {
            passwordDetailScreen->setPasswordId(updatedEntry.id);
        }
        popScreen(SCREEN_PASSWORD_DETAIL);
        return;
    }
    
    // Check if user wants to exit/cancel
    if (passwordEditScreen->needsToExit()) {
        DEBUG_LOG("PasswordEdit: Exit requested (cancelled)");
        popScreen(SCREEN_PASSWORD_DETAIL);
        return;
    }
}

void ScreenManager::handlePasswordEditTouch(const TouchPoint& point) {
    if (passwordEditScreen != nullptr) {
        passwordEditScreen->onTouchEvent(point);
    }
}

// ============================================================================
// DELETE CONFIRM DIALOG - Confirmation before deleting password
// ============================================================================

void ScreenManager::drawDeleteConfirmStaticUI() {
    // Lazy init Delete Confirm Dialog
    if (deleteConfirmDialog == nullptr) {
        deleteConfirmDialog = new DeleteConfirmDialog(tft, touch);
        deleteConfirmDialog->begin();
    }
    
    // Reset dialog state on entry
    deleteConfirmDialog->reset();
    
    DEBUG_LOG("DeleteConfirm - Initialized for static UI draw");
}

void ScreenManager::drawDeleteConfirmDynamicData() {
    if (deleteConfirmDialog != nullptr) {
        deleteConfirmDialog->draw();
    }
}

void ScreenManager::updateDeleteConfirmData() {
    if (deleteConfirmDialog == nullptr) return;
    
    // Update dialog logic
    deleteConfirmDialog->update();
    
    // Check if dialog needs to exit
    if (deleteConfirmDialog->needsToExit()) {
        if (deleteConfirmDialog->wasConfirmed()) {
            // User confirmed delete
            DEBUG_LOG("DeleteConfirm: Delete confirmed");
            
            // Get the password ID from detail screen
            if (passwordDetailScreen != nullptr) {
                uint32_t id = passwordDetailScreen->getCurrentPasswordId();
                
                // Delete from vault
                if (vaultStorage) {
                    vaultStorage->deletePassword(id);
                    Serial.printf("[DELETE_CONFIRM] Password ID %d deleted\n", id);
                }
                
                // Return to passwords list
                popScreen(SCREEN_PASSWORD_DETAIL);  // Close detail screen
                popScreen(SCREEN_PASSWORDS);        // Return to list (will auto-refresh)
            }
        } else {
            // User cancelled
            DEBUG_LOG("DeleteConfirm: Delete cancelled");
            popScreen(SCREEN_PASSWORD_DETAIL);  // Just close dialog, stay on detail
        }
        return;
    }
}

void ScreenManager::handleDeleteConfirmTouch(const TouchPoint& point) {
    if (deleteConfirmDialog != nullptr) {
        deleteConfirmDialog->onTouchEvent(point);
    }
}

// ============================================================================
// ABOUT SCREEN - Project information and credits
// ============================================================================

void ScreenManager::drawAboutStaticUI() {
    // Lazy init About screen
    if (aboutScreen == nullptr) {
        aboutScreen = new AboutScreen(tft, touch);
        aboutScreen->begin();
    }
    
    // Reset screen state on entry
    aboutScreen->reset();
    
    DEBUG_LOG("About - Initialized for static UI draw");
}

void ScreenManager::drawAboutDynamicData() {
    if (aboutScreen != nullptr) {
        aboutScreen->draw();
    }
}

void ScreenManager::updateAboutData() {
    if (aboutScreen == nullptr) return;
    
    // Update screen logic
    aboutScreen->update();
    
    // Check if user wants to exit
    if (aboutScreen->needsToExit()) {
        DEBUG_LOG("About: Exit requested");
        popScreen(SCREEN_SETTINGS);
        return;
    }
}

void ScreenManager::handleAboutTouch(const TouchPoint& point) {
    if (aboutScreen != nullptr) {
        aboutScreen->onTouchEvent(point);
    }
}


// ============================================================================
// SCREEN SLEEP STATE MANAGEMENT
// ============================================================================

void ScreenManager::sleepScreen() {
    if (screenAsleep) {
        return;  // Already asleep
    }
    
    Serial.println("[SCREEN_SLEEP] Entering sleep mode");
    
    // Store current screen to restore on wake
    screenBeforeSleep = currentScreen;
    
    // Turn off display (fill with black)
    tft->fillScreen(TFT_BLACK);
    
    // Turn off backlight
    digitalWrite(TFT_BL, LOW);
    Serial.println("[SCREEN_SLEEP] Backlight turned OFF");
    
    screenAsleep = true;
    
    // Reset manual tap counter when entering sleep
    manualTapCount = 0;
    lastManualTapTime = 0;
    
    Serial.println("[SCREEN_SLEEP] Screen is now asleep (backlight OFF, touch active)");
    Serial.println("[SCREEN_SLEEP] Double-tap within 400ms to wake");
}

void ScreenManager::wakeScreen() {
    if (!screenAsleep) {
        return;  // Already awake
    }
    
    Serial.println("[SCREEN_WAKE] Waking up screen");
    
    // Turn on backlight FIRST for immediate visual feedback
    digitalWrite(TFT_BL, HIGH);
    Serial.println("[SCREEN_WAKE] Backlight turned ON");
    
    // Restore previous screen
    currentScreen = screenBeforeSleep;
    forceFullRedraw = true;
    screenIsDirty = true;
    screenAsleep = false;
    
    // CRITICAL FIX: Force full redraw of screens that manage their own state
    // These screens have internal dirty flags that must be reset to ensure
    // complete UI restoration after wake from sleep
    Serial.printf("[SCREEN_WAKE] Forcing full redraw for screen: %s\n", getScreenName(currentScreen));
    
    switch (currentScreen) {
        case SCREEN_LOCK:
            if (lockScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Lock screen internal state");
                lockScreen->reset();
            }
            break;
        case SCREEN_PIN_ENTRY:
            if (pinEntryScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting PIN entry screen internal state");
                pinEntryScreen->reset();
            }
            break;
        case SCREEN_CHANGE_PIN:
            if (changePINScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Change PIN screen internal state");
                changePINScreen->reset();
            }
            break;
        case SCREEN_PASSWORDS:
            if (passwordsScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Passwords screen internal state");
                passwordsScreen->reset();
            }
            break;
        case SCREEN_PASSWORD_DETAIL:
            if (passwordDetailScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Password Detail screen internal state");
                passwordDetailScreen->reset();
            }
            break;
        case SCREEN_FAVORITES:
            if (favoritesScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Favorites screen internal state");
                favoritesScreen->reset();
            }
            break;
        case SCREEN_PASSWORD_EDIT:
            if (passwordEditScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Password Edit screen internal state");
                passwordEditScreen->reset();
            }
            break;
        case SCREEN_DELETE_CONFIRM:
            if (deleteConfirmDialog != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Delete Confirm dialog internal state");
                deleteConfirmDialog->reset();
            }
            break;
        case SCREEN_SETTINGS:
            if (settingsListScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Settings screen internal state");
                settingsListScreen->reset();
            }
            break;
        case SCREEN_ABOUT:
            if (aboutScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting About screen internal state");
                aboutScreen->reset();
            }
            break;
        case SCREEN_CALIBRATION:
            if (calibrationWizard != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Calibration wizard internal state");
                calibrationWizard->reset();
            }
            break;
        case SCREEN_CALIBRATION_RESET:
            if (calibrationResetScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Calibration Reset screen internal state");
                calibrationResetScreen->reset();
            }
            break;
        case SCREEN_FACTORY_RESET:
            if (factoryResetScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Factory Reset screen internal state");
                factoryResetScreen->reset();
            }
            break;
        case SCREEN_ORIENTATION:
            if (orientationScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Orientation screen internal state");
                orientationScreen->reset();
            }
            break;
        case SCREEN_ADVANCED_OPTIONS:
            if (advancedOptionsScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Advanced Options screen internal state");
                advancedOptionsScreen->reset();
            }
            break;
        case SCREEN_ADD_PASSWORD_MENU:
            if (addPasswordMenuScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Add Password Menu screen internal state");
                addPasswordMenuScreen->reset();
            }
            break;
        case SCREEN_ADD_PASSWORD_MANUAL:
            if (addPasswordManualScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Add Password Manual screen internal state");
                addPasswordManualScreen->reset();
            }
            break;
        case SCREEN_ADD_PASSWORD_WEB:
            if (addPasswordWebScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Add Password Web screen internal state");
                addPasswordWebScreen->reset();
            }
            break;
        case SCREEN_WEBUI_STATUS:
            if (webUIStatusScreen != nullptr) {
                Serial.println("[SCREEN_WAKE] Resetting Web UI Status screen internal state");
                webUIStatusScreen->reset();
            }
            break;
        case SCREEN_HOME:
        case SCREEN_INFO:
        case SCREEN_ADDPASS:
        case SCREEN_TOUCH_TEST:
            // These screens don't manage their own draw() - forceFullRedraw is sufficient
            Serial.println("[SCREEN_WAKE] Screen uses standard render flow");
            break;
    }
    
    // Reset wake tap counter
    wakeTapCount = 0;
    lastWakeTapTime = 0;
    
    // Reset auto-lock timer
    resetAutoLockTimer();
    
    Serial.printf("[SCREEN_WAKE] Screen awake, restored to %s\n", getScreenName(currentScreen));
}

// ============================================================================
// AUTO-LOCK TIMER - Automatic screen sleep functionality
// ============================================================================

void ScreenManager::resetAutoLockTimer() {
    lastInteractionTime = millis();
    autoLockTriggered = false;
    Serial.println("[AUTO_LOCK] Timer reset");
}

void ScreenManager::checkAutoLock() {
    // Skip if already asleep
    if (screenAsleep) {
        return;
    }
    
    // Skip on PIN Entry screen - don't auto-lock during PIN entry
    if (currentScreen == SCREEN_PIN_ENTRY) {
        return;
    }
    
    // Skip if settings manager not ready
    if (settingsManager == nullptr) {
        return;
    }
    
    // Check if auto-lock is enabled
    if (!settingsManager->isAutoLockEnabled()) {
        return;
    }
    
    // Check if already triggered
    if (autoLockTriggered) {
        return;
    }
    
    // Calculate elapsed time since last interaction
    unsigned long currentTime = millis();
    unsigned long elapsedSeconds = (currentTime - lastInteractionTime) / 1000;
    
    // Get timeout setting
    uint8_t timeoutSeconds = settingsManager->getAutoLockTimeout();
    
    // Check if timeout has been exceeded
    if (elapsedSeconds >= timeoutSeconds) {
        Serial.printf("[AUTO_LOCK] Timeout triggered after %lu seconds (limit: %d)\n", 
                     elapsedSeconds, timeoutSeconds);
        
        // Put device to sleep instead of returning to HOME
        sleepScreen();
        autoLockTriggered = true;
        
        Serial.println("[AUTO_LOCK] Device entered sleep mode");
    }
}

// ============================================================================
// WAKE GESTURE - Double-tap to wake from sleep
// ============================================================================

void ScreenManager::handleWakeGesture() {
    // This should only be called when screen is asleep
    if (!screenAsleep) {
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long elapsedSinceLastTap = currentTime - lastWakeTapTime;
    
    Serial.println("========================================");
    Serial.printf("[WAKE_GESTURE] ENTRY: wakeTapCount=%d, lastWakeTapTime=%lu, currentTime=%lu, elapsed=%lums\n",
                 wakeTapCount, lastWakeTapTime, currentTime, elapsedSinceLastTap);
    
    // First tap ever or after long timeout
    if (lastWakeTapTime == 0 || elapsedSinceLastTap > WAKE_TAP_WINDOW_MS) {
        wakeTapCount = 1;
        lastWakeTapTime = currentTime;
        Serial.println("[WAKE_GESTURE] START NEW SEQUENCE: Tap 1/2 (tap again within 400ms)");
        Serial.println("========================================");
        return;
    }
    
    // DEBOUNCE: Ignore touches within 100ms to filter touch noise
    if (elapsedSinceLastTap < DEBOUNCE_MS) {
        Serial.printf("[WAKE_GESTURE] *** DEBOUNCE: Ignoring touch (elapsed %lums < debounce %lums) ***\n", 
                     elapsedSinceLastTap, DEBOUNCE_MS);
        Serial.println("========================================");
        return;
    }
    
    // Within valid window (100-400ms) - count as second tap
    if (elapsedSinceLastTap >= DEBOUNCE_MS && elapsedSinceLastTap <= WAKE_TAP_WINDOW_MS) {
        wakeTapCount++;
        
        Serial.printf("[WAKE_GESTURE] WITHIN WINDOW: Tap %d (elapsed: %lums, window: %lu-%lums)\n", 
                     wakeTapCount, elapsedSinceLastTap, DEBOUNCE_MS, WAKE_TAP_WINDOW_MS);
        
        if (wakeTapCount >= 2) {
            // Two taps detected - wake the device
            Serial.println("[WAKE_GESTURE] *** DOUBLE-TAP CONFIRMED - WAKING DEVICE ***");
            
            wakeScreen();
            
            // Reset tap counter
            wakeTapCount = 0;
            lastWakeTapTime = 0;
            
            Serial.println("[WAKE_GESTURE] Device awake");
            Serial.println("========================================");
            return;
        }
        
        // Update timestamp for next tap
        lastWakeTapTime = currentTime;
    }
    
    Serial.printf("[WAKE_GESTURE] EXIT: wakeTapCount=%d, lastWakeTapTime=%lu\n", 
                 wakeTapCount, lastWakeTapTime);
    Serial.println("========================================");
}


// ============================================================================
// MANUAL SLEEP GESTURE - Double-tap while awake to manually sleep
// ============================================================================

void ScreenManager::handleManualSleepGesture() {
    // This should only be called when screen is awake and feature is enabled
    if (screenAsleep) {
        Serial.println("[MANUAL_SLEEP] ERROR: Called while screen asleep - ignoring");
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long elapsedSinceLastTap = currentTime - lastManualTapTime;
    
    Serial.println("========================================");
    Serial.printf("[MANUAL_SLEEP] ENTRY: manualTapCount=%d, lastManualTapTime=%lu, currentTime=%lu, elapsed=%lums\n",
                 manualTapCount, lastManualTapTime, currentTime, elapsedSinceLastTap);
    
    // DEBOUNCE: Ignore touches within 100ms (touch bounces/noise)
    if (lastManualTapTime > 0 && elapsedSinceLastTap < DEBOUNCE_MS) {
        Serial.printf("[MANUAL_SLEEP] *** DEBOUNCE: Ignoring touch (elapsed %lums < debounce %lums) ***\n", 
                     elapsedSinceLastTap, DEBOUNCE_MS);
        Serial.println("========================================");
        return;  // Filter out touch bounce
    }
    
    // Check if within double-tap window (after debounce period)
    if (lastManualTapTime > 0 && elapsedSinceLastTap >= DEBOUNCE_MS && elapsedSinceLastTap <= MANUAL_TAP_WINDOW_MS) {
        // Within window - increment counter
        manualTapCount++;
        
        Serial.printf("[MANUAL_SLEEP] WITHIN WINDOW: Tap %d/2 (elapsed: %lums, window: %lu-%lums)\n", 
                     manualTapCount, elapsedSinceLastTap, DEBOUNCE_MS, MANUAL_TAP_WINDOW_MS);
        
        if (manualTapCount >= 2) {
            // Two taps detected - sleep the device
            Serial.println("[MANUAL_SLEEP] *** DOUBLE-TAP CONFIRMED - ENTERING SLEEP MODE ***");
            
            sleepScreen();
            
            // Reset tap counter
            manualTapCount = 0;
            lastManualTapTime = 0;
            
            Serial.println("========================================");
            return;
        }
    } else {
        // Outside window or first tap - reset counter
        if (manualTapCount > 0) {
            Serial.printf("[MANUAL_SLEEP] TIMEOUT: Elapsed %lums > window %lums - resetting counter\n", 
                         elapsedSinceLastTap, MANUAL_TAP_WINDOW_MS);
        }
        manualTapCount = 1;
        Serial.printf("[MANUAL_SLEEP] START NEW SEQUENCE: Tap 1/2 (window: %lu-%lums)\n", DEBOUNCE_MS, MANUAL_TAP_WINDOW_MS);
    }
    
    lastManualTapTime = currentTime;
    
    Serial.printf("[MANUAL_SLEEP] EXIT: manualTapCount=%d, lastManualTapTime=%lu\n", 
                 manualTapCount, lastManualTapTime);
    Serial.println("========================================");
}

// ============================================================================
// PIN ENTRY SCREEN - Lock screen with PIN unlock
// ============================================================================

void ScreenManager::drawPINEntryStaticUI() {
    // Fallback if PIN entry screen not initialized
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 100);
    tft->print("Initializing...");
    DEBUG_LOG("PIN Entry - Fallback static UI drawn");
}

void ScreenManager::updatePINEntryData() {
    // PIN entry screen manages its own updates
    // Handled in main update() switch
}

void ScreenManager::handlePINEntryTouch(const TouchPoint& point) {
    if (pinEntryScreen != nullptr) {
        pinEntryScreen->handleTouch(point);
    }
}

// ============================================================================
// CHANGE PIN SCREEN - Guided PIN change flow
// ============================================================================

void ScreenManager::drawChangePINStaticUI() {
    // Lazy init Change PIN screen
    if (changePINScreen == nullptr) {
        changePINScreen = new ChangePINScreen(tft, touch, pinManager);
        changePINScreen->begin();
    }
    
    DEBUG_LOG("Change PIN - Initialized for static UI draw");
}

void ScreenManager::drawChangePINDynamicData() {
    if (changePINScreen != nullptr) {
        changePINScreen->draw();
    }
}

void ScreenManager::updateChangePINData() {
    // Change PIN screen manages its own updates
    // Handled in main update() switch
}

void ScreenManager::handleChangePINTouch(const TouchPoint& point) {
    if (changePINScreen != nullptr) {
        changePINScreen->handleTouch(point);
    }
}


// ============================================================================
// WEB UI STATUS SCREEN
// ============================================================================

void ScreenManager::drawWebUIStatusStaticUI() {
    // Lazy init Web UI Status screen
    if (webUIStatusScreen == nullptr) {
        webUIStatusScreen = new WebUIStatusScreen(tft, touch, webUISettingsManager, webUIActivePtr);
        webUIStatusScreen->begin();
    }
    
    // Reset screen state on entry
    webUIStatusScreen->reset();
    
    DEBUG_LOG("WebUI Status - Initialized for static UI draw");
}

void ScreenManager::drawWebUIStatusDynamicData() {
    if (webUIStatusScreen != nullptr) {
        webUIStatusScreen->draw();
    }
}

void ScreenManager::updateWebUIStatusData() {
    // No continuous updates needed - screen is event-driven
}

void ScreenManager::handleWebUIStatusTouch(const TouchPoint& point) {
    if (webUIStatusScreen != nullptr) {
        webUIStatusScreen->onTouchEvent(point);
    }
}

// ============================================================================
// LOCK SCREEN
// ============================================================================

void ScreenManager::drawLockStaticUI() {
    // Lazy init Lock screen
    if (lockScreen == nullptr) {
        lockScreen = new LockScreen(tft, touch);
        lockScreen->begin();
    } else {
        lockScreen->reset();
    }
    
    DEBUG_LOG("Lock Screen - Initialized for static UI draw");
}

void ScreenManager::drawLockDynamicData() {
    if (lockScreen != nullptr) {
        lockScreen->draw();
    }
}

void ScreenManager::updateLockData() {
    // Lock screen manages its own updates
    // Handled in main update() switch
}

void ScreenManager::handleLockTouch(const TouchPoint& point) {
    if (lockScreen != nullptr) {
        lockScreen->onTouchEvent(point);
    }
}

