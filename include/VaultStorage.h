#ifndef VAULTSTORAGE_H
#define VAULTSTORAGE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>
#include "PasswordEntry.h"

// Vault storage manager - handles persistent storage of passwords
class VaultStorage {
public:
    VaultStorage();
    
    // Initialize storage (mount LittleFS)
    bool begin();
    
    // Password CRUD operations
    bool addPassword(const PasswordEntry& entry);
    bool updatePassword(uint32_t id, const PasswordEntry& entry);
    bool deletePassword(uint32_t id);
    PasswordEntry getPassword(uint32_t id);
    
    // List operations
    std::vector<PasswordEntry> getAllPasswords();
    int getPasswordCount();
    
    // Search/filter
    std::vector<PasswordEntry> getFavorites();
    std::vector<PasswordEntry> searchByTitle(const char* query);
    
    // Favorites toggle
    bool toggleFavorite(uint32_t id);
    
    // Storage management
    bool clearAll();
    bool formatStorage();
    
private:
    static const char* VAULT_FILE;
    uint32_t nextId;
    
    // Internal helpers
    bool loadFromFile(std::vector<PasswordEntry>& entries);
    bool saveToFile(const std::vector<PasswordEntry>& entries);
    uint32_t generateId();
};

#endif // VAULTSTORAGE_H
