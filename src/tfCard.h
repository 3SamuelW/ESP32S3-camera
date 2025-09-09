// tfCard.h - SD card module
// This header declares all functions and variables related to the SD card (TF card) module.
// It provides interfaces for initializing the SD card, writing photo files, and managing file indices.
//
// Key features:
// - SD card initialization and error handling
// - Photo file writing (JPEG)
// - Automatic file index management for unique filenames

#pragma once // Prevent multiple inclusion of this header
#include <SD.h> // SD card library
#include <SPI.h> // SPI library for SD card communication

// SPI bus object for the SD card
extern SPIClass spiSd;

// Pin definitions for SD card (change according to your hardware)
#define SD_SCK_PIN 13   // Serial Clock
#define SD_MISO_PIN 14  // Master In Slave Out
#define SD_MOSI_PIN 12  // Master Out Slave In
#define SD_CS_PIN 11    // Chip Select

/**
 * @brief Initialize the SD card and handle errors.
 */
void TfCard_Init();

/**
 * @brief Write a photo file to the SD card with a unique filename.
 * @param data Pointer to photo data (JPEG buffer)
 * @param size Size of photo data in bytes
 */
void TfCard_WritePhoto(const uint8_t *data, size_t size);

/**
 * @brief Get the next available photo index for unique filenames.
 * @return Next photo index (1-based)
 */
int TfCard_GetNextPhotoIndex();