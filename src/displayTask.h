// displayTask.h - TFT display task module
// This header declares all functions and variables related to the TFT display task.
// It provides interfaces for initializing the display, drawing overlays, saving photos,
// showing error messages, and managing the gallery and preview modes.
//
// Key features:
// - Real-time camera preview
// - Photo gallery browsing
// - Photo saving with feedback
// - Error display
// - 3x3 grid overlay

#pragma once // Prevent multiple inclusion of this header
#include <Arduino.h> // Arduino core library
#include <TFT_eSPI.h> // TFT display library

// SPI bus object for the TFT display
extern SPIClass spiLcd;
// TFT display object
extern TFT_eSPI tftDisplay;

// Pin definitions for the TFT display (change according to your hardware)
#define LCD_MOSI_PIN 2      // Master Out Slave In (data to display)
#define LCD_MISO_PIN -1     // Master In Slave Out (not used)
#define LCD_SCK_PIN 5       // Serial Clock
#define LCD_CS_PIN 3        // Chip Select
#define LCD_DC_PIN 4        // Data/Command select
#define LCD_RST_PIN -1      // Reset pin (not used)
#define LCD_BLK_PIN 1       // Backlight control

/**
 * @brief Main display task loop. Handles preview/gallery switching and key events.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void DisplayTask(void *pvParameters);

/**
 * @brief Draw a 3x3 grid overlay on an image buffer for composition guidance.
 * @param image Pointer to image buffer (RGB565)
 * @param width Image width
 * @param height Image height
 * @param color Grid line color
 */
void DisplayTask_DrawGrid3x3(uint16_t *image, int width, int height, uint16_t color);

/**
 * @brief Initialize the TFT display, show splash screen, and set up JPEG decoder.
 */
void DisplayTask_Init();

/**
 * @brief Display an error message and halt the system.
 * @param message Error message to display
 */
void DisplayTask_ShowError(const char *message);

// Indicates if the "Saving..." popup should be shown
extern bool isSavingPopupVisible;
// Indicates if the photo save operation is done (not used in this code, but can be used for UI)
extern bool isSaveDone;
// Task handle for the display task (for external access)
extern TaskHandle_t displayTaskHandle;
// Task handle for the camera task (for external access)
extern TaskHandle_t cameraTaskHandle;

/**
 * @brief Save a photo from the camera to the SD card, with UI feedback.
 */
void DisplayTask_SavePhoto();

/**
 * @brief Display a photo from SD card in gallery mode.
 * @param index Photo index (1-based)
 */
void DisplayTask_ShowGallery(int index);