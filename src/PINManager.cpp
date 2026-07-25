#include "PINManager.h"
#include "Config.h"

const char* PINManager::PIN_FILE = "/pin.dat";
const char* PINManager::TEMP_PIN_FILE = "/temp_pins.dat";

PINManager::PINManager()
    : pinSet(false) {
    setDefaults();
}

void PINManager::begin() {
    DEBUG_LOG("PINManager: Initializing...");
    
    // Ensure LittleFS is mounted
    if (!LittleFS.begin()) {
        DEBUG_LOG("PINManager: LittleFS not mounted, attempting mount...");
        if (!LittleFS.begin()) {
            DEBUG_LOG("PINManager: LittleFS mount failed, using default PIN");
            setDefaults();
            return;
        }
    }
    
    // Try to load existing PIN
    if (loadFromFile()) {
        pinSet = true;
        DEBUG_LOG("PINManager: Master PIN loaded from storage");
    } else {
        // First time setup - use default PIN "1234"
        DEBUG_LOG("PINManager: No PIN found, setting default PIN: 1234");
        strcpy(pinData.pin, "1234");
        pinSet = true;
        saveToFile();  // Save default PIN for next boot
    }
    
    // Load temporary keypad PINs
    loadTempPINsFromFile();
}

void PINManager::setDefaults() {
    pinData.magic = PIN_MAGIC;
    pinData.version = PIN_VERSION;
    strcpy(pinData.pin, "1234");  // Default PIN
    memset(pinData.reserved, 0, sizeof(pinData.reserved));
    pinSet = false;
    tempPINs.clear();
}

bool PINManager::validatePIN(const char* pin) {
    if (!pinSet || pin == nullptr) {
        DEBUG_LOG("PINManager: Validation failed - PIN not set or null");
        return false;
    }
    
    // Check length
    if (strlen(pin) != PIN_LENGTH) {
        DEBUG_LOGF("PINManager: Invalid PIN length: %d\n", strlen(pin));
        return false;
    }
    
    // 1. Compare Master PIN
    if (strcmp(pinData.pin, pin) == 0) {
        DEBUG_LOG("PINManager: Master PIN validated successfully");
        return true;
    }
    
    // 2. Check Active Temporary PINs
    for (auto& tp : tempPINs) {
        if (!tp.isUsed && strcmp(tp.pin, pin) == 0) {
            tp.isUsed = true;  // Mark as used/expired immediately
            saveTempPINsToFile();
            Serial.printf("[PINManager] Temp PIN '%s' (%s) used to unlock device - Status updated to USED/EXPIRED\n", 
                          tp.pin, tp.note);
            return true;
        }
    }
    
    DEBUG_LOG("PINManager: PIN validation FAILED (Neither Master PIN nor Active Temp PIN matched)");
    return false;
}

bool PINManager::setPIN(const char* newPin) {
    if (!isValidPIN(newPin)) {
        DEBUG_LOG("PINManager: Invalid PIN format");
        return false;
    }
    
    // Set new PIN
    strncpy(pinData.pin, newPin, PIN_LENGTH);
    pinData.pin[PIN_LENGTH] = '\0';  // Ensure null termination
    
    pinSet = true;
    
    // Save to storage
    if (saveToFile()) {
        DEBUG_LOGF("PINManager: PIN set successfully: %s\n", newPin);
        return true;
    } else {
        DEBUG_LOG("PINManager: Failed to save PIN to storage");
        return false;
    }
}

bool PINManager::changePIN(const char* currentPin, const char* newPin) {
    // Validate current PIN
    if (!validatePIN(currentPin)) {
        DEBUG_LOG("PINManager: Change PIN failed - current PIN incorrect");
        return false;
    }
    
    // Set new PIN
    return setPIN(newPin);
}

bool PINManager::addTempPIN(const char* pin, const char* note) {
    if (!isValidPIN(pin)) {
        Serial.println("[PINManager] Failed to add temp PIN: Invalid PIN format (must be 4 digits)");
        return false;
    }
    
    TempPIN tp;
    tp.id = millis() ^ random(1000, 9999);
    if (tp.id == 0) tp.id = 1;
    
    strncpy(tp.pin, pin, sizeof(tp.pin) - 1);
    tp.pin[sizeof(tp.pin) - 1] = '\0';
    
    if (note && strlen(note) > 0) {
        strncpy(tp.note, note, sizeof(tp.note) - 1);
        tp.note[sizeof(tp.note) - 1] = '\0';
    } else {
        strcpy(tp.note, "Temp PIN");
    }
    
    tp.isUsed = false;
    
    tempPINs.push_back(tp);
    saveTempPINsToFile();
    
    Serial.printf("[PINManager] Added Temp PIN: %s (%s), ID=%u\n", tp.pin, tp.note, tp.id);
    return true;
}

bool PINManager::deleteTempPIN(uint32_t id) {
    for (auto it = tempPINs.begin(); it != tempPINs.end(); ++it) {
        if (it->id == id) {
            Serial.printf("[PINManager] Deleted Temp PIN ID=%u\n", id);
            tempPINs.erase(it);
            saveTempPINsToFile();
            return true;
        }
    }
    return false;
}

bool PINManager::clearExpiredTempPINs() {
    size_t initialSize = tempPINs.size();
    tempPINs.erase(
        std::remove_if(tempPINs.begin(), tempPINs.end(), [](const TempPIN& tp) { return tp.isUsed; }),
        tempPINs.end()
    );
    size_t removed = initialSize - tempPINs.size();
    if (removed > 0) {
        saveTempPINsToFile();
        Serial.printf("[PINManager] Cleared %d expired Temp PINs\n", removed);
    }
    return true;
}

void PINManager::resetPIN() {
    DEBUG_LOG("PINManager: Resetting PIN to default (1234)");
    
    setDefaults();
    strcpy(pinData.pin, "1234");
    pinSet = true;
    saveToFile();
    saveTempPINsToFile();
}

bool PINManager::isValidPIN(const char* pin) const {
    if (pin == nullptr) {
        return false;
    }
    
    // Check length
    if (strlen(pin) != PIN_LENGTH) {
        return false;
    }
    
    // Check all characters are digits
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (!isdigit(pin[i])) {
            return false;
        }
    }
    
    return true;
}

bool PINManager::saveToFile() {
    File file = LittleFS.open(PIN_FILE, "w");
    if (!file) {
        Serial.println("[PIN] Failed to open PIN file for writing");
        return false;
    }
    
    size_t written = file.write((uint8_t*)&pinData, sizeof(PINData));
    file.close();
    
    if (written != sizeof(PINData)) {
        Serial.println("[PIN] Failed to write complete PIN data");
        return false;
    }
    
    Serial.printf("[PIN] Saved %d bytes to %s\n", written, PIN_FILE);
    return true;
}

bool PINManager::loadFromFile() {
    if (!LittleFS.exists(PIN_FILE)) {
        Serial.println("[PIN] PIN file does not exist");
        return false;
    }
    
    File file = LittleFS.open(PIN_FILE, "r");
    if (!file) {
        Serial.println("[PIN] Failed to open PIN file for reading");
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize != sizeof(PINData)) {
        Serial.printf("[PIN] PIN file size mismatch: %d != %d\n", fileSize, sizeof(PINData));
        file.close();
        return false;
    }
    
    size_t read = file.read((uint8_t*)&pinData, sizeof(PINData));
    file.close();
    
    if (read != sizeof(PINData)) {
        Serial.println("[PIN] Failed to read complete PIN data");
        return false;
    }
    
    // Validate magic number
    if (pinData.magic != PIN_MAGIC) {
        Serial.printf("[PIN] Invalid magic number: 0x%08X\n", pinData.magic);
        return false;
    }
    
    // Validate version
    if (pinData.version != PIN_VERSION) {
        Serial.printf("[PIN] Unsupported PIN version: %d\n", pinData.version);
        return false;
    }
    
    Serial.printf("[PIN] Loaded %d bytes from %s\n", read, PIN_FILE);
    return true;
}

bool PINManager::saveTempPINsToFile() {
    File file = LittleFS.open(TEMP_PIN_FILE, "w");
    if (!file) {
        Serial.println("[PIN] Failed to open temp PIN file for writing");
        return false;
    }
    
    uint16_t count = (uint16_t)tempPINs.size();
    file.write((uint8_t*)&count, sizeof(count));
    
    for (const auto& tp : tempPINs) {
        file.write((uint8_t*)&tp, sizeof(TempPIN));
    }
    
    file.close();
    Serial.printf("[PIN] Saved %d Temp PINs to %s\n", count, TEMP_PIN_FILE);
    return true;
}

bool PINManager::loadTempPINsFromFile() {
    tempPINs.clear();
    
    if (!LittleFS.exists(TEMP_PIN_FILE)) {
        Serial.println("[PIN] Temp PIN file does not exist (will create when added)");
        return false;
    }
    
    File file = LittleFS.open(TEMP_PIN_FILE, "r");
    if (!file) {
        Serial.println("[PIN] Failed to open Temp PIN file for reading");
        return false;
    }
    
    uint16_t count = 0;
    if (file.read((uint8_t*)&count, sizeof(count)) != sizeof(count)) {
        file.close();
        return false;
    }
    
    for (uint16_t i = 0; i < count; i++) {
        TempPIN tp;
        if (file.read((uint8_t*)&tp, sizeof(TempPIN)) == sizeof(TempPIN)) {
            tempPINs.push_back(tp);
        }
    }
    
    file.close();
    Serial.printf("[PIN] Loaded %d Temp PINs from %s\n", tempPINs.size(), TEMP_PIN_FILE);
    return true;
}

