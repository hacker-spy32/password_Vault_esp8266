#ifndef DELETECONFIRMDIALOG_H
#define DELETECONFIRMDIALOG_H

#include <TFT_eSPI.h>
#include "TouchManager.h"
#include "Config.h"

class DeleteConfirmDialog {
public:
    DeleteConfirmDialog(TFT_eSPI* display, TouchManager* touch);
    
    void begin();
    void reset();
    void update();
    void draw();
    
    // Set the item name to display in dialog
    void setItemName(const char* name);
    
    // Touch handling
    void onTouchEvent(const TouchPoint& point);
    
    // Dialog result
    bool needsToExit() const { return needsExit; }
    bool wasConfirmed() const { return confirmed; }
    bool wasCancelled() const { return cancelled; }
    
private:
    TFT_eSPI* tft;
    TouchManager* touch;
    
    // State
    bool needsExit;
    bool confirmed;
    bool cancelled;
    bool forceFullRedraw;
    bool screenIsDirty;
    
    // Item name
    char itemName[32];
    
    // Layout constants
    static const int DIALOG_WIDTH = 280;
    static const int DIALOG_HEIGHT = 180;
    static const int BUTTON_HEIGHT = 44;
    static const int BUTTON_SPACING = 12;
    
    // Drawing methods
    void drawStaticUI();
    void drawDynamicData();
    
    // Touch detection
    bool isDeleteButtonPressed(int16_t x, int16_t y) const;
    bool isCancelButtonPressed(int16_t x, int16_t y) const;
};

#endif
