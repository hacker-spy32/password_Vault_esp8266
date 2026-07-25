#ifndef PASSWORDSSCREEN_H
#define PASSWORDSSCREEN_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "VaultStorage.h"
#include "SidebarScrollButtons.h"
#include "Config.h"
#include <vector>

class PasswordsScreen {
public:
    PasswordsScreen(TFT_eSPI* display, TouchManager* touch, VaultStorage* vault);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Exit management
    bool needsToExit() const { return needsExit; }
    uint32_t getSelectedPasswordId() const { return selectedPasswordId; }
    bool hasSelection() const { return selectedPasswordId != 0; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    VaultStorage* vault;
    SidebarScrollButtons* sidebarButtons;
    
    // State
    bool needsExit;
    uint32_t selectedPasswordId;  // ID of password to view details
    bool forceFullRedraw;
    bool screenIsDirty;
    bool fabVisible;  // Track FAB visibility
    
    // Scroll state
    int scrollOffset;
    int maxScrollOffset;
    
    // Data
    std::vector<PasswordEntry> passwords;
    int totalCount;
    
    // Layout constants
    static const int HEADER_HEIGHT = 56;  // Status bar + header
    static const int ROW_HEIGHT = 64;
    static const int ROW_SPACING = 8;
    static const int AVATAR_SIZE = 40;
    static const int SIDE_MARGIN = 16;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    void drawPasswordRow(int y, const PasswordEntry& entry, int index);
    void drawEmptyState();
    
    // Touch detection
    bool isBackButtonPressed(int16_t x, int16_t y) const;
    int getPasswordIndexAtPosition(int16_t x, int16_t y) const;
    
    // Data management
    void refreshPasswordList();
    void calculateMaxScroll();
    
    // Helper methods
    char getInitialLetter(const char* title);
    uint16_t getAvatarColor(const char* title);
    void drawHeart(int centerX, int centerY, int size, uint16_t color, bool filled);
};

#endif // PASSWORDSSCREEN_H
