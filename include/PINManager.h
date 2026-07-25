#ifndef PINMANAGER_H
#define PINMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>

// Temporary PIN data structure for keypad access
struct TempPIN {
    uint32_t id;         // Unique identifier
    char pin[5];         // 4-digit PIN + null terminator
    char note[32];       // Description/Label
    bool isUsed;         // false = Active, true = Used/Expired
};

// PIN Manager - Handles Master PIN & Temporary Keypad PIN storage, validation, and security
class PINManager {
public:
    PINManager();
    
    // Initialize PIN system from storage
    void begin();
    
    // PIN validation (checks Master PIN and active Temporary PINs)
    bool validatePIN(const char* pin);  // Returns true if matches master PIN or active temp PIN
    
    // Master PIN management
    bool setPIN(const char* newPin);    // Set new PIN (must be 4 digits)
    bool changePIN(const char* currentPin, const char* newPin);  // Change PIN with validation
    bool hasPIN() const { return pinSet; }  // Check if PIN is configured
    
    // Temporary PIN management for one-time keypad access
    bool addTempPIN(const char* pin, const char* note);
    std::vector<TempPIN> getTempPINs() const { return tempPINs; }
    bool deleteTempPIN(uint32_t id);
    bool clearExpiredTempPINs();
    
    // Factory reset
    void resetPIN();  // Clear PIN (use with caution)
    
    // Get default PIN for first-time setup
    const char* getDefaultPIN() const { return "1234"; }
    
private:
    static const char* PIN_FILE;
    static const char* TEMP_PIN_FILE;
    static const uint32_t PIN_MAGIC = 0x50494E4B;  // "PINK" (PIN Key)
    static const uint8_t PIN_VERSION = 1;
    static const uint8_t PIN_LENGTH = 4;
    
    struct PINData {
        uint32_t magic;       // Magic number for validation
        uint8_t version;      // Format version
        char pin[5];          // 4-digit PIN + null terminator
        uint8_t reserved[8];  // Reserved for future use
    };
    
    PINData pinData;
    bool pinSet;
    std::vector<TempPIN> tempPINs;
    
    // Internal helpers
    bool saveToFile();
    bool loadFromFile();
    bool saveTempPINsToFile();
    bool loadTempPINsFromFile();
    bool isValidPIN(const char* pin) const;  // Check if PIN format is valid (4 digits)
    void setDefaults();
};

#endif // PINMANAGER_H

