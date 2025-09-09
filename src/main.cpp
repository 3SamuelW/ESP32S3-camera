// main.cpp - Main entry point
// This is the main entry point for the ESP32 camera project.
// It initializes all hardware modules, starts FreeRTOS tasks, and provides the main loop.
//
// Key features:
// - Initializes serial port for debugging
// - Initializes display, keys, SD card, camera, and web server
// - Starts FreeRTOS tasks for camera, display, web server, and key input
// - Main loop is empty (all logic is in tasks)

#include <SD.h>            // SD card library
#include <Arduino.h>       // Arduino core library
#include <TJpg_Decoder.h>  // JPEG decoder for display
#include "cameraTask.h"    // Camera task module
#include "displayTask.h"   // Display task module
#include "tfCard.h"        // SD card module
#include "webTask.h"       // Web server module
#include "keyTask.h"       // Key input module

// Mutex for camera access (if needed for thread safety)
SemaphoreHandle_t cameraMutex;
// Task handle for camera task (for external access)
TaskHandle_t cameraTaskHandle = NULL;

/**
 * @brief Arduino setup function. Initializes all modules and starts tasks.
 */
void setup() {
    Serial.begin(115200); // Start serial port for debugging
    Serial.println("[Main] System setup started."); // Debug output
    cameraMutex = xSemaphoreCreateMutex(); // Create mutex for camera access
    DisplayTask_Init(); // Initialize TFT display
    KeyTask_Init();     // Initialize keys and flash LED
    TfCard_Init();      // Initialize SD card
    CameraTask_InitPreviewConfig(); // Set camera to preview mode
    CameraTask_Init(); // Initialize camera hardware
    WebTask_Init();    // Initialize web server
    // Start FreeRTOS tasks for camera, display, web server, and key input
    xTaskCreatePinnedToCore(CameraTask, "CameraTask", 4096, NULL, 1, &cameraTaskHandle, 0);
    xTaskCreatePinnedToCore(DisplayTask, "DisplayTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(WebTask, "WebTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(KeyTask, "KeyTask", 4096, NULL, 3, NULL, 1);
    Serial.println("[Main] System setup completed."); // Debug output
}

/**
 * @brief Arduino main loop. Not used (all logic is in FreeRTOS tasks).
 */
void loop() {}

/*
WIFI_Name: ESP32-CAM
WIFI_Password: MyPassword
Web address: http://192.168.4.1
*/