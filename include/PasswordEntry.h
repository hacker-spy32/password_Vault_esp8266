#ifndef PASSWORDENTRY_H
#define PASSWORDENTRY_H

#include <Arduino.h>

// Password entry structure - shared across multiple components
struct PasswordEntry {
    char title[32];    // Service name (e.g., "Amazon", "Google")
    char user[32];     // Username or email
    char pass[32];     // Password
    uint32_t id;       // Unique ID for each entry
    bool isFavorite;   // Favorite flag
    bool isOneTime;    // One-time guest password flag (expires on use)
    uint32_t createdAt; // Unix timestamp
};

#endif // PASSWORDENTRY_H
