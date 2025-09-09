// cameraTask.h - Camera task module
// This header declares all functions and variables related to the camera task.
// It provides interfaces for initializing the camera, configuring preview and photo modes,
// managing the camera sensor, and handling the camera frame queue.
//
// Key features:
// - Camera hardware configuration (OV2640/OV5640)
// - Preview mode (low-res, fast, RGB565)
// - Photo mode (high-res, JPEG)
// - Camera sensor parameter adjustment (effects, brightness, etc.)
// - Frame queue for real-time preview

#pragma once // Prevent multiple inclusion of this header
#include <TFT_eSPI.h> // TFT display library (for preview integration)
#include <esp_camera.h> // ESP32 camera driver
#include <freertos/queue.h> // FreeRTOS queue for frame buffer

// Pin definitions for camera module (change according to your hardware)
#define CAM_PWDN_PIN 46   // Power down pin
#define CAM_RESET_PIN -1  // Reset pin (not used)
#define CAM_XCLK_PIN -1   // External clock pin (not used)
#define CAM_SIOD_PIN 17   // SCCB data
#define CAM_SIOC_PIN 18   // SCCB clock
#define CAM_Y9_PIN 21     // Data pins
#define CAM_Y8_PIN 42
#define CAM_Y7_PIN 40
#define CAM_Y6_PIN 41
#define CAM_Y5_PIN 39
#define CAM_Y4_PIN 15
#define CAM_Y3_PIN 38
#define CAM_Y2_PIN 16
#define CAM_VSYNC_PIN 48  // Vertical sync
#define CAM_HREF_PIN 47   // Horizontal reference
#define CAM_PCLK_PIN 45   // Pixel clock

// External references to display and camera configuration
extern TFT_eSPI tftDisplay;           // TFT display object (for preview)
extern camera_config_t cameraConfig;  // Camera configuration struct
extern QueueHandle_t cameraFrameQueue;// Queue for camera frame buffers
extern sensor_t *cameraSensor;        // Pointer to camera sensor struct
extern int cameraEffectMode;          // Camera special effect mode (0 = none)
extern int cameraParamLevel;          // Camera parameter level (brightness, etc.)

/**
 * @brief Initialize camera configuration for preview mode (low-res, RGB565).
 */
void CameraTask_InitPreviewConfig();

/**
 * @brief Initialize camera configuration for photo mode (high-res, JPEG).
 */
void CameraTask_InitPhotoConfig();

/**
 * @brief Main camera task loop. Continuously captures frames and sends to queue.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void CameraTask(void *pvParameters);

/**
 * @brief Initialize the camera hardware, create frame queue, and set sensor parameters.
 */
void CameraTask_Init();

/**
 * @brief Initialize camera sensor software parameters (effects, brightness, etc.).
 */
void CameraTask_InitSensorConfig();