// tfCard.cpp - SD card implementation
// This module implements all SD card (TF card) operations for the ESP32 system.
// It handles SD card initialization, photo file writing, and file index management.
//
// Key features:
// - SD card initialization and error handling
// - Photo file writing (JPEG)
// - Automatic file index management for unique filenames

#include "tfCard.h"      // Include header for this module
#include <esp_camera.h>   // For camera frame buffer type
#include "displayTask.h" // For error display

// SPI bus object for the SD card (VSPI bus)
SPIClass spiSd(VSPI);

/**
 * @brief Initialize the SD card and handle errors.
 * Sets up the SPI bus and attempts to mount the SD card.
 * If initialization fails, displays an error and retries.
 */
void TfCard_Init() {
    spiSd.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN); // Initialize SPI bus for SD card
    while (1) {
        if (SD.begin(SD_CS_PIN, spiSd)) { // Try to mount SD card
            Serial.println("[TFCard] SD card initialized successfully."); // Debug output
            break; // Success, exit loop
        } else {
            Serial.println("[TFCard] SD card initialization failed, retrying..."); // Debug output
            DisplayTask_ShowError("TFCard not found!\n\nPlease check the connection and retry."); // Show error on display
            delay(1000); // Wait before retrying
        }
        tftDisplay.fillScreen(TFT_BLACK); // Clear display (optional)
    }
}

/**
 * @brief Get the next available photo index for unique filenames.
 * Scans the SD card for existing photo files and returns the next available index.
 * @return Next photo index (1-based)
 */
int TfCard_GetNextPhotoIndex() {
    int index = 1; // Start with index 1
    char filename[32]; // Buffer for filename
    while (true) {
        sprintf(filename, "/photo_%d.jpg", index); // Generate filename
        if (!SD.exists(filename)) { // If file does not exist
            break; // Found available index
        }
        ++index; // Try next index
    }
    return index; // Return next available index
}

/**
 * @brief Write a photo file to the SD card with a unique filename.
 * @param data Pointer to photo data (JPEG buffer)
 * @param size Size of photo data in bytes
 */
void TfCard_WritePhoto(const uint8_t *data, size_t size) {
    int photoIndex = TfCard_GetNextPhotoIndex(); // Get next available index
    char filename[32]; // Buffer for filename
    sprintf(filename, "/photo_%d.jpg", photoIndex); // Generate unique filename
    File file = SD.open(filename, FILE_WRITE); // Open file for writing
    if (file) { // If file opened successfully
        file.write(data, size); // Write photo data
        file.close(); // Close file
        Serial.printf("[TFCard] Photo saved: %s\n", filename); // Debug output
    } else {
        Serial.println("[TFCard] Photo save failed!"); // Debug output
    }
}