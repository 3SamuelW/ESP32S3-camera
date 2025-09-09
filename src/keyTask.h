// keyTask.h - Key input task module
// This header declares all functions and variables related to the key/button input task.
// It provides interfaces for initializing keys, handling key events, controlling the flash LED,
// and managing key states for camera/photo navigation and UI interaction.
//
// Key features:
// - Debounced, interrupt-driven key input
// - Single, double, and long press detection
// - Flash LED control
// - Key state variables for UI logic

#pragma once // Prevent multiple inclusion of this header
#include <Arduino.h> // Arduino core library
#include "displayTask.h" // For display feedback

// Pin definitions for keys and flash LED (change according to your hardware)
#define KEY_CAM_PIN 10      // Camera shutter key (take photo)
#define KEY_TOP_PIN 9       // Top navigation key (mode up/prev)
#define KEY_MID_PIN 8       // Middle key (toggle gallery/preview)
#define KEY_DOWN_PIN 7      // Down navigation key (mode down/next)
#define LED_FLASH_PIN 6     // Flash LED control pin

// Key state variables (used by display task and others)
extern int keyTopState;     // State of the top key (1 = pressed, 0 = released)
extern int keyMidState;     // State of the middle key (toggle: 0 = preview, 1 = gallery)
extern int keyDownState;    // State of the down key (1 = pressed, 0 = released)

/**
 * @brief Initialize all keys and the flash LED, set up interrupts.
 */
void KeyTask_Init();

/**
 * @brief Control the flash LED (on/off).
 * @param on true to turn on, false to turn off
 */
void KeyTask_SetLED(bool on);

/**
 * @brief Main key input task loop. Handles debouncing and event detection.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void KeyTask(void *pvParameters);