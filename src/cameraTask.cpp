// cameraTask.cpp - Camera task implementation
// This module implements all camera-related operations for the ESP32 system.
// It handles camera hardware configuration, preview and photo modes, sensor parameter adjustment,
// and real-time frame capture for display or storage.
//
// Key features:
// - Camera hardware configuration (OV2640/OV5640)
// - Preview mode (low-res, fast, RGB565)
// - Photo mode (high-res, JPEG)
// - Camera sensor parameter adjustment (effects, brightness, etc.)
// - Frame queue for real-time preview

#include "cameraTask.h" // Include header for this module
#include "config.h"     // Include global configuration
#include "displayTask.h"// For error display and preview integration

// Camera effect mode (0 = none, others = special effects)
int cameraEffectMode = 0;
// Camera parameter level (brightness, contrast, etc.)
int cameraParamLevel = 0;
// Camera configuration struct (hardware pins, format, etc.)
camera_config_t cameraConfig;
// Queue for camera frame buffers (for real-time preview)
QueueHandle_t cameraFrameQueue;
// Pointer to camera sensor struct (for parameter adjustment)
sensor_t *cameraSensor;

/**
 * @brief Initialize camera configuration for preview mode (low-res, RGB565).
 * Sets all hardware pins and parameters for fast, low-latency preview.
 */
void CameraTask_InitPreviewConfig() {
    cameraConfig.ledc_channel = LEDC_CHANNEL_0; // LEDC channel for XCLK
    cameraConfig.ledc_timer = LEDC_TIMER_0;     // LEDC timer for XCLK
    cameraConfig.pin_d0 = CAM_Y2_PIN;           // Data pins
    cameraConfig.pin_d1 = CAM_Y3_PIN;
    cameraConfig.pin_d2 = CAM_Y4_PIN;
    cameraConfig.pin_d3 = CAM_Y5_PIN;
    cameraConfig.pin_d4 = CAM_Y6_PIN;
    cameraConfig.pin_d5 = CAM_Y7_PIN;
    cameraConfig.pin_d6 = CAM_Y8_PIN;
    cameraConfig.pin_d7 = CAM_Y9_PIN;
    cameraConfig.pin_xclk = CAM_XCLK_PIN;       // XCLK pin (not used)
    cameraConfig.pin_pclk = CAM_PCLK_PIN;       // Pixel clock
    cameraConfig.pin_vsync = CAM_VSYNC_PIN;     // Vertical sync
    cameraConfig.pin_href = CAM_HREF_PIN;       // Horizontal reference
    cameraConfig.pin_sccb_sda = CAM_SIOD_PIN;   // SCCB data
    cameraConfig.pin_sccb_scl = CAM_SIOC_PIN;   // SCCB clock
    cameraConfig.pin_pwdn = CAM_PWDN_PIN;       // Power down
    cameraConfig.pin_reset = CAM_RESET_PIN;     // Reset (not used)
    cameraConfig.xclk_freq_hz = 20000000;       // XCLK frequency
    cameraConfig.pixel_format = PIXFORMAT_RGB565;// RGB565 for fast preview
    cameraConfig.frame_size = FRAMESIZE_QVGA;   // QVGA resolution (320x240)
    cameraConfig.jpeg_quality = 20;             // JPEG quality (not used in preview)
    cameraConfig.fb_count = 2;                  // Double buffering
    cameraConfig.fb_location = CAMERA_FB_IN_PSRAM; // Use PSRAM for frame buffer
}

/**
 * @brief Initialize camera configuration for photo mode (high-res, JPEG).
 * Sets all hardware pins and parameters for high-quality photo capture.
 */
void CameraTask_InitPhotoConfig() {
    cameraConfig.ledc_channel = LEDC_CHANNEL_0;
    cameraConfig.ledc_timer = LEDC_TIMER_0;
    cameraConfig.pin_d0 = CAM_Y2_PIN;
    cameraConfig.pin_d1 = CAM_Y3_PIN;
    cameraConfig.pin_d2 = CAM_Y4_PIN;
    cameraConfig.pin_d3 = CAM_Y5_PIN;
    cameraConfig.pin_d4 = CAM_Y6_PIN;
    cameraConfig.pin_d5 = CAM_Y7_PIN;
    cameraConfig.pin_d6 = CAM_Y8_PIN;
    cameraConfig.pin_d7 = CAM_Y9_PIN;
    cameraConfig.pin_xclk = CAM_XCLK_PIN;
    cameraConfig.pin_pclk = CAM_PCLK_PIN;
    cameraConfig.pin_vsync = CAM_VSYNC_PIN;
    cameraConfig.pin_href = CAM_HREF_PIN;
    cameraConfig.pin_sccb_sda = CAM_SIOD_PIN;
    cameraConfig.pin_sccb_scl = CAM_SIOC_PIN;
    cameraConfig.pin_pwdn = CAM_PWDN_PIN;
    cameraConfig.pin_reset = CAM_RESET_PIN;
    cameraConfig.xclk_freq_hz = 20000000;
    cameraConfig.pixel_format = PIXFORMAT_JPEG; // JPEG for high-quality photo
#if defined(OV2640)
    cameraConfig.frame_size = FRAMESIZE_SXGA; // 1280x1024 for OV2640
#elif defined(OV5640)
    cameraConfig.frame_size = FRAMESIZE_QSXGA; // 2560x1920 for OV5640
#else
    cameraConfig.frame_size = FRAMESIZE_UXGA; // 1600x1200 (default)
#endif
    cameraConfig.jpeg_quality = 12;           // Higher quality for photos
    cameraConfig.fb_count = 1;                // Single buffer for photo
    cameraConfig.fb_location = CAMERA_FB_IN_PSRAM;
}

/**
 * @brief Main camera task loop. Continuously captures frames and sends to queue.
 * Should be run as a FreeRTOS task. Used for real-time preview.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void CameraTask(void *pvParameters) {
    while (1) {
        camera_fb_t *fb = esp_camera_fb_get(); // Capture a frame from camera
        if (!fb) { // If capture failed
            Serial.println("[CameraTask] Camera capture failed!");
            vTaskDelay(10 / portTICK_PERIOD_MS); // Wait and retry
            continue;
        }
        if (xQueueSend(cameraFrameQueue, &fb, 0) != pdTRUE) { // Send frame to queue
            esp_camera_fb_return(fb); // If queue full, return frame buffer
        }
        vTaskDelay(30 / portTICK_PERIOD_MS); // Control frame rate
    }
}

/**
 * @brief Initialize camera sensor software parameters (effects, brightness, etc.).
 * Sets special effect mode, brightness, contrast, and other sensor parameters.
 */
void CameraTask_InitSensorConfig() {
    cameraSensor = esp_camera_sensor_get(); // Get pointer to sensor struct
    cameraSensor->set_contrast(cameraSensor, cameraParamLevel);   // Set contrast
    cameraSensor->set_brightness(cameraSensor, cameraParamLevel); // Set brightness
    cameraSensor->set_saturation(cameraSensor, cameraParamLevel); // Set saturation
#if defined(OV2640)
    cameraSensor->set_vflip(cameraSensor, 0); // Vertical flip for OV2640
#elif defined(OV5640)
    cameraSensor->set_vflip(cameraSensor, 1); // Vertical flip for OV5640
#else
    cameraSensor->set_vflip(cameraSensor, 1); // Default vertical flip
#endif
    cameraSensor->set_special_effect(cameraSensor, cameraEffectMode); // Set special effect
}

/**
 * @brief Initialize the camera hardware, create frame queue, and set sensor parameters.
 * Handles hardware errors and retries initialization if needed.
 */
void CameraTask_Init() {
    CameraTask_InitPreviewConfig(); // Set preview mode config
    if (esp_camera_init(&cameraConfig) != ESP_OK) { // Try to initialize camera
        Serial.println("[CameraTask] Camera init failed!");
        while (1) {
            Serial.println("[CameraTask] Retrying camera init...");
            DisplayTask_ShowError("Camera NOT Found.\n\nPlease check the camera connection and reboot.");
            esp_camera_deinit();
            CameraTask_InitPreviewConfig();
            if (esp_camera_init(&cameraConfig) == ESP_OK) {
                tftDisplay.fillScreen(TFT_BLACK);
                break;
            }
        }
        delay(1000);
    }
    Serial.println("[CameraTask] Camera init done.");
    cameraFrameQueue = xQueueCreate(1, sizeof(camera_fb_t *)); // Create frame queue
    if (!cameraFrameQueue) {
        Serial.println("[CameraTask] Failed to create frame queue!");
        while (1) {}
    }
    Serial.println("[CameraTask] Frame queue created.");
    CameraTask_InitSensorConfig(); // Set sensor parameters
}