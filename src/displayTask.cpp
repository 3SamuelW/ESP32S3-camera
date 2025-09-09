// displayTask.cpp - TFT display task implementation
// This module handles all display-related operations for the TFT screen, including:
// - Real-time camera preview with FPS and mode info
// - Photo gallery browsing with navigation
// - Photo saving with visual feedback
// - Error display for hardware issues
// - 3x3 grid overlay for composition
//
// The display task uses double buffering (TFT_eSprite) to avoid flicker and provides a user-friendly interface.
// All functions and variables are documented for beginner understanding.

#include <Arduino.h>           // Arduino core library for basic types and functions
#include <TJpg_Decoder.h>      // JPEG decoder library for displaying images
#include "displayTask.h"       // Header for display task functions and variables
#include "cameraTask.h"        // Header for camera task functions and variables
#include "image.h"             // Header for image data arrays (icons, splash, etc.)
#include "tfCard.h"            // Header for SD card functions
#include "keyTask.h"           // Header for key/button input functions
#include "config.h"            // Global configuration header

// Indicates if the "Saving..." popup should be shown on the display
bool isSavingPopupVisible = false;
// SPI bus object for the TFT display (HSPI bus)
SPIClass spiLcd(HSPI);
// TFT display object (TFT_eSPI library)
TFT_eSPI tftDisplay;
// Off-screen sprite buffer for double buffering (avoids flicker)
TFT_eSprite spriteBuffer = TFT_eSprite(&tftDisplay);

// Index of the currently displayed photo in gallery mode (1-based)
volatile int currentPhotoIdx = -1;
// True if in photo gallery mode, false for live preview mode
volatile bool isPhotoViewMode = false;
// Timing variables for FPS calculation
static unsigned long lastFrameMillis = 0; // Last time FPS was updated
static int frameCount = 0;                // Frame count for FPS
float frameRate = 0;                      // Calculated FPS value
// Pointer to the current camera frame buffer
camera_fb_t *frameBuffer = NULL;
// External task handle for camera task (used to pause/resume camera)
extern TaskHandle_t cameraTaskHandle;
// Mutex for camera access (if needed for thread safety)
extern SemaphoreHandle_t camMutex;

/**
 * @brief Save a photo from the camera to the SD card.
 * This function pauses the camera preview, switches to high-res photo mode,
 * captures a frame, saves it to SD, and restores preview mode.
 * Visual feedback is provided on the display, and all steps are logged.
 */
void DisplayTask_SavePhoto() {
    Serial.println("[DisplayTask] Photo save started.");
    isSavingPopupVisible = true; // Show "Saving..." popup
    vTaskDelay(100 / portTICK_PERIOD_MS); // Allow popup to be visible
    // Pause camera task to avoid resource conflict
    if (cameraTaskHandle != NULL) {
        vTaskDelete(cameraTaskHandle); // Stop camera task
        cameraTaskHandle = NULL;
        Serial.println("[DisplayTask] Camera task deleted.");
        vTaskDelay(30 / portTICK_PERIOD_MS); // Wait for resources to be released
    }
    esp_camera_deinit(); // Deinitialize camera hardware
    vTaskDelay(100 / portTICK_PERIOD_MS); // Ensure hardware is released
    CameraTask_InitPhotoConfig(); // Switch to photo mode (high-res JPEG)
    Serial.println("[DisplayTask] Picture mode activated.");
    esp_err_t err = esp_camera_init(&cameraConfig); // Re-initialize camera
    CameraTask_InitSensorConfig(); // Set sensor parameters
    if (err != ESP_OK) {
        Serial.printf("[DisplayTask] Camera reinit JPEG failed: %d\n", err);
        return;
    }
    camera_fb_t *fb = esp_camera_fb_get(); // Capture a frame
    TfCard_WritePhoto(fb->buf, fb->len); // Save to SD card
    esp_camera_fb_return(fb); // Return frame buffer to driver
    Serial.println("[DisplayTask] Photo taken.");
    esp_camera_deinit(); // Deinitialize camera again
    vTaskDelay(100 / portTICK_PERIOD_MS);
    CameraTask_InitPreviewConfig(); // Restore preview mode (low-res RGB565)
    esp_camera_init(&cameraConfig); // Re-initialize camera for preview
    CameraTask_InitSensorConfig(); // Set sensor parameters
    Serial.println("[DisplayTask] Switched back to camera mode.");
    KeyTask_SetLED(false); // Ensure flash LED is off
    Serial.println("[DisplayTask] LED closed.");
    isSavingPopupVisible = false; // Hide "Saving..." popup
    xTaskCreatePinnedToCore(CameraTask, "CameraTask", 4096, NULL, 1, &cameraTaskHandle, 0); // Restart camera task
    Serial.println("[DisplayTask] Camera task restarted.");
}

/**
 * @brief JPEG decoder pixel output callback for TFT_eSPI.
 * This function is called by the JPEG decoder for each block of pixels.
 * It pushes the decoded block to the TFT display at the specified position.
 * @param x X coordinate of the block
 * @param y Y coordinate of the block
 * @param w Width of the block
 * @param h Height of the block
 * @param data Pointer to pixel data (RGB565 format)
 * @return true if successful, false otherwise
 */
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *data) {
    if (y >= tftDisplay.height()) return 0; // Ignore blocks outside display
    tftDisplay.pushImage(x, y, w, h, data); // Draw block to display
    return 1; // Success
}

/**
 * @brief Display a photo from SD card in gallery mode.
 * Decodes the JPEG file and renders it to the TFT display.
 * Shows file name and resolution, and overlays navigation icons for user guidance.
 * @param index Photo index (1-based)
 */
void DisplayTask_ShowGallery(int index) {
#if defined(OV2640)
    TJpgDec.setJpgScale(4); // Use 1/4 scale for OV2640
#elif defined(OV5640)
    TJpgDec.setJpgScale(8); // Use 1/8 scale for OV5640
#else
    TJpgDec.setJpgScale(8); // Default scale
#endif
    Serial.println("[DisplayTask] Photo reading started.");
    TJpgDec.setCallback(tft_output); // Set JPEG decoder callback
    tftDisplay.setSwapBytes(true); // Required for color order (RGB565)
    char filename[32];
    sprintf(filename, "/photo_%d.jpg", index); // Generate filename
    tftDisplay.fillScreen(TFT_BLACK); // Clear display
    tftDisplay.setCursor(5, 220); // Set cursor for loading text
    tftDisplay.setTextSize(2);
    tftDisplay.setTextColor(TFT_YELLOW);
    tftDisplay.printf("Photo loading...\n"); // Show loading text
    TJpgDec.drawSdJpg(0, 0, filename); // Decode and draw JPEG
    Serial.printf("[DisplayTask] File '%s' printed.\n", filename);
    tftDisplay.setCursor(5, 0); // Set cursor for filename
    tftDisplay.setTextColor(TFT_WHITE);
    tftDisplay.setTextSize(2);
    uint16_t w = 0, h = 0;
    TJpgDec.getSdJpgSize(&w, &h, filename); // Get image size
    tftDisplay.printf(filename); // Show filename
    tftDisplay.setCursor(5, 220); // Set cursor for DPI info
    tftDisplay.setTextSize(2);
    tftDisplay.setTextColor(TFT_YELLOW);
    tftDisplay.printf("DPI: %dx%d\n", w, h); // Show resolution
    // Overlay navigation icons for user guidance
    tftDisplay.pushImage(290, 105, 30, 30, camera); // Camera icon
    tftDisplay.pushImage(290, 5, 30, 30, up);       // Up icon
    tftDisplay.pushImage(290, 205, 30, 30, down);   // Down icon
    Serial.printf("[DisplayTask] Showing: %s\n", filename);
}

/**
 * @brief Initialize the TFT display and show the startup logo.
 * Sets up SPI, display rotation, backlight, and JPEG decoder.
 * Shows a splash screen for 3 seconds, then clears the display.
 */
void DisplayTask_Init() {
    spiLcd.begin(LCD_SCK_PIN, LCD_MOSI_PIN, LCD_CS_PIN); // Initialize SPI bus
    tftDisplay.begin(); // Initialize TFT display
    tftDisplay.setRotation(1); // Landscape mode
    pinMode(LCD_BLK_PIN, OUTPUT); // Set backlight pin as output
    digitalWrite(LCD_BLK_PIN, HIGH); // Turn on backlight
    tftDisplay.fillScreen(TFT_BLACK); // Clear display
    tftDisplay.pushImage(0, 0, 320, 240, logo); // Show logo image
    delay(3000); // Show splash screen for 3 seconds
    tftDisplay.fillScreen(TFT_BLACK); // Clear display
    TJpgDec.setJpgScale(8); // Default JPEG scale
    TJpgDec.setCallback(tft_output); // Set JPEG decoder callback
    Serial.println("[DisplayTask] Screen init done.");
}

/**
 * @brief Draw a 3x3 grid overlay on an image buffer.
 * Useful for composition guidance in photography.
 * @param image Pointer to image buffer (RGB565)
 * @param width Image width
 * @param height Image height
 * @param color Grid line color
 */
void DisplayTask_DrawGrid3x3(uint16_t *image, int width, int height, uint16_t color) {
    int cellW = width / 3; // Width of each cell
    int cellH = height / 3; // Height of each cell
    for (int y = 0; y < height; y++) {
        image[y * width + cellW] = color; // Vertical line 1
        image[y * width + 2 * cellW] = color; // Vertical line 2
    }
    for (int x = 0; x < width; x++) {
        image[cellH * width + x] = color; // Horizontal line 1
        image[2 * cellH * width + x] = color; // Horizontal line 2
    }
}

/**
 * @brief Display an error message and halt the system.
 * Fills the screen with black, shows the error text in red, and displays an icon.
 * Used for critical errors such as missing hardware. The system halts in this state.
 * @param message Error message to display
 */
void DisplayTask_ShowError(const char *message) {
    tftDisplay.fillScreen(TFT_BLACK); // Clear display
    tftDisplay.setTextColor(TFT_RED); // Set text color to red
    tftDisplay.setTextSize(2); // Set text size
    tftDisplay.setCursor(5, 170); // Set cursor position
    tftDisplay.println(message); // Print error message
    tftDisplay.pushImage(100, 10, 128, 128, tfcard); // Show SD card icon
    while (1) {
        delay(1000); // Halt in error state
    }
}

/**
 * @brief Show the live camera preview on the TFT display.
 * Receives frames from the camera queue, overlays grid and info, and displays FPS.
 * Uses double buffering to avoid flicker. All UI overlays are drawn on the sprite buffer.
 */
void DisplayTask_ShowCamera() {
    if (xQueueReceive(cameraFrameQueue, &frameBuffer, portMAX_DELAY) == pdTRUE) {
        frameCount++; // Increment frame count for FPS
        unsigned long now = millis(); // Get current time
        if (now - lastFrameMillis >= 1000) { // Update FPS every second
            frameRate = frameCount * 1000.0f / (now - lastFrameMillis);
            frameCount = 0;
            lastFrameMillis = now;
        }
        uint16_t *img = (uint16_t *)frameBuffer->buf; // Pointer to image data
        int w = frameBuffer->width; // Image width
        int h = frameBuffer->height; // Image height
        spriteBuffer.createSprite(w, h); // Create off-screen buffer
        spriteBuffer.setSwapBytes(false); // Set byte order for RGB565
        spriteBuffer.pushImage(0, 0, w, h, img); // Draw camera image
        DisplayTask_DrawGrid3x3((uint16_t *)spriteBuffer.getPointer(), w, h, TFT_WHITE); // Draw grid
        spriteBuffer.setTextColor(TFT_WHITE); // Set text color for FPS
        spriteBuffer.setTextSize(2); // Set text size
        char infoStr[32]; // Buffer for info strings
        sprintf(infoStr, "FPS: %d", (int)frameRate); // Format FPS string
        spriteBuffer.drawString(infoStr, 5, 200); // Draw FPS
        spriteBuffer.setTextColor(TFT_YELLOW); // Set text color for mode info
        sprintf(infoStr, "Mode:%d", cameraEffectMode); // Format mode string
        spriteBuffer.drawString(infoStr, 210, 15); // Draw mode info
        spriteBuffer.pushImage(290, 105, 30, 30, photo); // Photo icon
        spriteBuffer.pushImage(290, 5, 30, 30, color);   // Color icon
        spriteBuffer.pushImage(290, 205, 30, 30, sun);   // Sun icon
        spriteBuffer.setTextColor(TFT_YELLOW); // Set text color for light info
        sprintf(infoStr, "light: %d", cameraParamLevel); // Format light string
        spriteBuffer.drawString(infoStr, 195, 220); // Draw light info
        sprintf(infoStr, "DPI: %dx%d", w, h); // Format DPI string
        spriteBuffer.drawString(infoStr, 5, 220); // Draw DPI info
        if (isSavingPopupVisible) { // If saving popup should be shown
            spriteBuffer.setTextColor(TFT_YELLOW, TFT_BLACK); // Yellow text on black
            spriteBuffer.drawString("S A V I N G ...", 80, 110); // Draw saving popup
        }
        spriteBuffer.pushSprite(0, 0); // Push sprite to display
        spriteBuffer.deleteSprite(); // Delete sprite to free memory
        esp_camera_fb_return(frameBuffer); // Return frame buffer to driver
    }
}

/**
 * @brief Main display task loop.
 * Handles switching between live preview and gallery mode, and responds to key events.
 * In preview mode, shows live camera feed and allows mode/parameter adjustment.
 * In gallery mode, allows browsing saved photos. All state changes are logged.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void DisplayTask(void *pvParameters) {
    static bool galleryLoaded = false; // True if gallery has been loaded
    while (1) {
        if (keyMidState == 0) { // If in preview mode
            tftDisplay.setSwapBytes(false); // Set byte order for preview
            galleryLoaded = false; // Reset gallery state
            isPhotoViewMode = false; // Not in gallery mode
            DisplayTask_ShowCamera(); // Show live camera preview
            // Handle top key: change camera effect mode
            if (keyTopState == 1) {
                keyTopState = 0;
                if (cameraEffectMode > 5) {
                    cameraEffectMode = 0;
                } else {
                    ++cameraEffectMode;
                }
                CameraTask_InitSensorConfig(); // Update sensor config
                Serial.printf("[DisplayTask] Effect mode changed: %d\n", cameraEffectMode);
            }
            // Handle down key: change camera parameter level
            if (keyDownState == 1) {
                keyDownState = 0;
                if (cameraParamLevel > 1) {
                    cameraParamLevel = -2;
                } else {
                    ++cameraParamLevel;
                }
                CameraTask_InitSensorConfig(); // Update sensor config
                Serial.printf("[DisplayTask] Param level changed: %d\n", cameraParamLevel);
            }
        } else { // If in gallery mode
            // Enter gallery mode on mid key
            if (!galleryLoaded) {
                int lastIdx = TfCard_GetNextPhotoIndex() - 1; // Get last photo index
                if (lastIdx >= 1) {
                    currentPhotoIdx = lastIdx; // Set current photo index
                    DisplayTask_ShowGallery(currentPhotoIdx); // Show last photo
                    isPhotoViewMode = true; // In gallery mode
                    Serial.printf("[DisplayTask] Entered gallery mode, showing photo %d.\n", currentPhotoIdx);
                } else {
                    DisplayTask_ShowError("  No photos found. \n\n  Please take a photo first. "); // Show error
                    Serial.println("[DisplayTask] No photos found, error displayed.");
                }
                galleryLoaded = true; // Mark gallery as loaded
            }
            // Gallery navigation with top/down keys
            if (isPhotoViewMode) {
                if (keyTopState == 1) { // Previous photo
                    keyTopState = 0;
                    if (currentPhotoIdx > 1) {
                        --currentPhotoIdx;
                        DisplayTask_ShowGallery(currentPhotoIdx);
                        Serial.printf("[DisplayTask] Gallery: previous photo %d.\n", currentPhotoIdx);
                    }
                }
                if (keyDownState == 1) { // Next photo
                    keyDownState = 0;
                    if (currentPhotoIdx < TfCard_GetNextPhotoIndex() - 1) {
                        ++currentPhotoIdx;
                        DisplayTask_ShowGallery(currentPhotoIdx);
                        Serial.printf("[DisplayTask] Gallery: next photo %d.\n", currentPhotoIdx);
                    }
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Control refresh rate
    }
}