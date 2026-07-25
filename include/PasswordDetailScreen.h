#ifndef PASSWORDDETAILSCREEN_H
#define PASSWORDDETAILSCREEN_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "VaultStorage.h"
#include "Config.h"

class PasswordDetailScreen {
public:
    PasswordDetailScreen(TFT_eSPI* display, TouchManager* touch, VaultStorage* vault);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Set which password to display
    void setPasswordId(uint32_t id);
    
    // Exit management
    bool needsToExit() const { return needsExit; }
    bool needsToDelete() const { return needsDelete; }
    bool needsToEdit() const { return needsEdit; }
    uint32_t getCurrentPasswordId() const { return currentPasswordId; }
    void clearEditFlag() { needsEdit = false; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    VaultStorage* vault;
    
    // State
    bool needsExit;
    bool needsDelete;
    bool needsEdit;
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Current password
    uint32_t currentPasswordId;
    PasswordEntry currentPassword;
    
    // Password visibility toggle
    bool passwordVisible;
    unsigned long eyePressStartTime;
    bool eyeButtonPressed;
    
    // Layout constants
    static const int HEADER_HEIGHT = 56;
    static const int FIELD_HEIGHT = 64;
    static const int FIELD_SPACING = 16;
    static const int BUTTON_HEIGHT = 44;
    static const int SIDE_MARGIN = 16;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawField(int y, const char* label, const char* value, bool isMasked, bool hasEyeIcon);
    void drawEyeIcon(int x, int y, bool isPressed);
    void drawActionButtons();
    void drawHeart(int centerX, int centerY, int size, uint16_t color, bool filled);
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    bool isEyeIconPressed(int16_t x, int16_t y) const;
    bool isEditButtonPressed(int16_t x, int16_t y) const;
    bool isFavoriteButtonPressed(int16_t x, int16_t y) const;
    bool isDeleteButtonPressed(int16_t x, int16_t y) const;
    
    // Data operations
    void toggleFavorite();
};

#endif // PASSWORDDETAILSCREEN_H
