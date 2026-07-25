#include "VaultStorage.h"
#include "Config.h"

const char* VaultStorage::VAULT_FILE = "/vault.dat";

VaultStorage::VaultStorage() : nextId(1) {
}

bool VaultStorage::begin() {
    // Mount LittleFS
    if (!LittleFS.begin()) {
        Serial.println("[VAULT] Failed to mount LittleFS");
        
        // Try formatting and mounting again
        Serial.println("[VAULT] Attempting to format...");
        if (!LittleFS.format()) {
            Serial.println("[VAULT] Format failed");
            return false;
        }
        
        if (!LittleFS.begin()) {
            Serial.println("[VAULT] Mount failed after format");
            return false;
        }
    }
    
    Serial.println("[VAULT] LittleFS mounted successfully");
    
    // Load existing data to determine next ID
    std::vector<PasswordEntry> entries;
    if (loadFromFile(entries)) {
        // Find highest ID
        for (const auto& entry : entries) {
            if (entry.id >= nextId) {
                nextId = entry.id + 1;
            }
        }
        Serial.printf("[VAULT] Loaded %d entries, nextId=%d\n", entries.size(), nextId);
    }
    
    return true;
}

bool VaultStorage::addPassword(const PasswordEntry& entry) {
    std::vector<PasswordEntry> entries;
    loadFromFile(entries);
    
    // Create new entry with generated ID
    PasswordEntry newEntry = entry;
    newEntry.id = generateId();
    newEntry.createdAt = millis() / 1000; // Simple timestamp
    
    entries.push_back(newEntry);
    
    bool success = saveToFile(entries);
    if (success) {
        Serial.printf("[VAULT] Added password: ID=%d, Title=%s, IsOneTime=%d\n", newEntry.id, newEntry.title, newEntry.isOneTime);
    }
    
    return success;
}

bool VaultStorage::updatePassword(uint32_t id, const PasswordEntry& entry) {
    std::vector<PasswordEntry> entries;
    if (!loadFromFile(entries)) {
        return false;
    }
    
    // Find and update entry
    bool found = false;
    for (auto& e : entries) {
        if (e.id == id) {
            // Preserve ID and createdAt
            uint32_t originalId = e.id;
            uint32_t originalCreatedAt = e.createdAt;
            
            e = entry;
            e.id = originalId;
            e.createdAt = originalCreatedAt;
            
            found = true;
            break;
        }
    }
    
    if (!found) {
        Serial.printf("[VAULT] Password ID %d not found for update\n", id);
        return false;
    }
    
    bool success = saveToFile(entries);
    if (success) {
        Serial.printf("[VAULT] Updated password: ID=%d\n", id);
    }
    
    return success;
}

bool VaultStorage::deletePassword(uint32_t id) {
    std::vector<PasswordEntry> entries;
    if (!loadFromFile(entries)) {
        return false;
    }
    
    // Remove entry with matching ID
    size_t originalSize = entries.size();
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                      [id](const PasswordEntry& e) { return e.id == id; }),
        entries.end()
    );
    
    if (entries.size() == originalSize) {
        Serial.printf("[VAULT] Password ID %d not found for delete\n", id);
        return false;
    }
    
    bool success = saveToFile(entries);
    if (success) {
        Serial.printf("[VAULT] Deleted password: ID=%d\n", id);
    }
    
    return success;
}

PasswordEntry VaultStorage::getPassword(uint32_t id) {
    std::vector<PasswordEntry> entries;
    loadFromFile(entries);
    
    for (const auto& entry : entries) {
        if (entry.id == id) {
            return entry;
        }
    }
    
    // Return empty entry if not found
    PasswordEntry empty = {0};
    return empty;
}

std::vector<PasswordEntry> VaultStorage::getAllPasswords() {
    std::vector<PasswordEntry> entries;
    loadFromFile(entries);
    return entries;
}

int VaultStorage::getPasswordCount() {
    std::vector<PasswordEntry> entries;
    if (!loadFromFile(entries)) {
        return 0;
    }
    return entries.size();
}

std::vector<PasswordEntry> VaultStorage::getFavorites() {
    std::vector<PasswordEntry> entries;
    loadFromFile(entries);
    
    std::vector<PasswordEntry> favorites;
    for (const auto& entry : entries) {
        if (entry.isFavorite) {
            favorites.push_back(entry);
        }
    }
    
    return favorites;
}

std::vector<PasswordEntry> VaultStorage::searchByTitle(const char* query) {
    std::vector<PasswordEntry> entries;
    loadFromFile(entries);
    
    std::vector<PasswordEntry> results;
    String queryLower = String(query);
    queryLower.toLowerCase();
    
    for (const auto& entry : entries) {
        String titleLower = String(entry.title);
        titleLower.toLowerCase();
        
        if (titleLower.indexOf(queryLower) >= 0) {
            results.push_back(entry);
        }
    }
    
    return results;
}

bool VaultStorage::toggleFavorite(uint32_t id) {
    std::vector<PasswordEntry> entries;
    if (!loadFromFile(entries)) {
        return false;
    }
    
    bool found = false;
    for (auto& entry : entries) {
        if (entry.id == id) {
            entry.isFavorite = !entry.isFavorite;
            found = true;
            Serial.printf("[VAULT] Toggled favorite: ID=%d, isFavorite=%d\n", id, entry.isFavorite);
            break;
        }
    }
    
    if (!found) {
        return false;
    }
    
    return saveToFile(entries);
}

bool VaultStorage::clearAll() {
    std::vector<PasswordEntry> empty;
    bool success = saveToFile(empty);
    
    if (success) {
        Serial.println("[VAULT] Cleared all passwords");
        nextId = 1;
    }
    
    return success;
}

bool VaultStorage::formatStorage() {
    LittleFS.end();
    
    if (!LittleFS.format()) {
        Serial.println("[VAULT] Format failed");
        return false;
    }
    
    if (!LittleFS.begin()) {
        Serial.println("[VAULT] Mount failed after format");
        return false;
    }
    
    Serial.println("[VAULT] Storage formatted");
    nextId = 1;
    return true;
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================

bool VaultStorage::loadFromFile(std::vector<PasswordEntry>& entries) {
    entries.clear();
    
    if (!LittleFS.exists(VAULT_FILE)) {
        Serial.println("[VAULT] Vault file doesn't exist (empty vault)");
        return true; // Empty vault is valid
    }
    
    File file = LittleFS.open(VAULT_FILE, "r");
    if (!file) {
        Serial.println("[VAULT] Failed to open vault file for reading");
        return false;
    }
    
    // Read entry count
    uint32_t count = 0;
    if (file.read((uint8_t*)&count, sizeof(count)) != sizeof(count)) {
        Serial.println("[VAULT] Failed to read entry count");
        file.close();
        return false;
    }
    
    // Read entries
    for (uint32_t i = 0; i < count; i++) {
        PasswordEntry entry;
        if (file.read((uint8_t*)&entry, sizeof(entry)) != sizeof(entry)) {
            Serial.printf("[VAULT] Failed to read entry %d\n", i);
            file.close();
            return false;
        }
        entries.push_back(entry);
    }
    
    file.close();
    Serial.printf("[VAULT] Loaded %d entries from file\n", count);
    return true;
}

bool VaultStorage::saveToFile(const std::vector<PasswordEntry>& entries) {
    File file = LittleFS.open(VAULT_FILE, "w");
    if (!file) {
        Serial.println("[VAULT] Failed to open vault file for writing");
        return false;
    }
    
    // Write entry count
    uint32_t count = entries.size();
    if (file.write((uint8_t*)&count, sizeof(count)) != sizeof(count)) {
        Serial.println("[VAULT] Failed to write entry count");
        file.close();
        return false;
    }
    
    // Write entries
    for (const auto& entry : entries) {
        if (file.write((uint8_t*)&entry, sizeof(entry)) != sizeof(entry)) {
            Serial.println("[VAULT] Failed to write entry");
            file.close();
            return false;
        }
    }
    
    file.close();
    Serial.printf("[VAULT] Saved %d entries to file\n", count);
    return true;
}

uint32_t VaultStorage::generateId() {
    return nextId++;
}
