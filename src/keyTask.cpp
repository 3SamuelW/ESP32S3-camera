// keyTask.cpp - Key input task implementation
// This module implements debounced, interrupt-driven key/button input handling for the ESP32 system.
// It supports single, double, and long press detection for four keys, and controls a flash LED.
// Key events are used for camera shutter, navigation, and UI mode switching.
//
// Key features:
// - Interrupt-based key press detection (fast, low-latency)
// - Debouncing and state machine for reliable input
// - Single, double, and long press event detection
// - Flash LED control for camera
// - Key state variables for UI logic

#include "keyTask.h" // Include header for this module

// Timing constants for key event detection (in milliseconds)
#define DOUBLE_CLICK_MS 400 // Max interval between clicks for double click
#define LONG_PRESS_MS 800   // Min duration for long press
#define DEBOUNCE_MS 100     // Debounce time for key press

// Enumeration for key state machine
enum KeyState {
    KEY_IDLE,         // No press
    KEY_PRESSED,      // Key is pressed
    KEY_WAIT_SECOND,  // Waiting for second press (double click)
    KEY_LONG_PRESSED  // Key is held for long press
};

// Structure to hold key information and state
struct KeyInfo {
    int pin;              // GPIO pin number for the key
    KeyState state;       // Current state of the key
    unsigned long pressStart; // Timestamp when key was pressed
    unsigned long lastPress;  // Timestamp of last release (for double click)
    bool flag;            // Set by ISR when key is pressed (interrupt flag)
    bool skipSingle;      // Skip single click if double/long detected
};

// Array of all keys (camera, top, mid, down)
KeyInfo keyArray[4] = {
    {KEY_CAM_PIN, KEY_IDLE, 0, 0, false, true}, // Camera shutter key
    {KEY_TOP_PIN, KEY_IDLE, 0, 0, false, true}, // Top navigation key
    {KEY_MID_PIN, KEY_IDLE, 0, 0, false, true}, // Middle key
    {KEY_DOWN_PIN, KEY_IDLE, 0, 0, false, true} // Down navigation key
};

// Key state variables for UI logic
int keyMidState = 0;   // 0 = preview mode, 1 = gallery mode
int keyTopState = 0;   // 1 = pressed, 0 = released
int keyDownState = 0;  // 1 = pressed, 0 = released

// Interrupt Service Routines (ISR) for each key
// These are called on falling edge (key press) and set the flag for the main task
void IRAM_ATTR onKeyCam() { keyArray[0].flag = true; }
void IRAM_ATTR onKeyTop() { keyArray[1].flag = true; }
void IRAM_ATTR onKeyMid() { keyArray[2].flag = true; }
void IRAM_ATTR onKeyDown() { keyArray[3].flag = true; }

/**
 * @brief Initialize all keys and the flash LED, set up interrupts.
 * Configures GPIO pins, sets up pull-ups, and attaches ISRs for each key.
 */
void KeyTask_Init() {
    for (int i = 0; i < 4; i++) {
        pinMode(keyArray[i].pin, INPUT_PULLUP); // Set key pin as input with pull-up
    }
    pinMode(LED_FLASH_PIN, OUTPUT); // Set flash LED pin as output
    digitalWrite(LED_FLASH_PIN, HIGH); // Turn off flash LED (active low)
    // Attach interrupts for each key (falling edge = press)
    attachInterrupt(digitalPinToInterrupt(KEY_CAM_PIN), onKeyCam, FALLING);
    attachInterrupt(digitalPinToInterrupt(KEY_TOP_PIN), onKeyTop, FALLING);
    attachInterrupt(digitalPinToInterrupt(KEY_MID_PIN), onKeyMid, FALLING);
    attachInterrupt(digitalPinToInterrupt(KEY_DOWN_PIN), onKeyDown, FALLING);
    Serial.println("[KeyTask] Keys initialized."); // Debug output
}

/**
 * @brief Handle key state machine and event detection for a single key.
 * Detects single, double, and long press events, and calls the appropriate callback.
 * @param key Reference to KeyInfo struct for this key
 * @param name Name of the key (for debug output)
 * @param stateVar Reference to state variable for this key (for UI logic)
 * @param singleClick Callback for single click event
 * @param doubleClick Callback for double click event
 * @param longPress Callback for long press event
 */
void handleKey(KeyInfo &key, const char *name, int &stateVar, void (*singleClick)(), void (*doubleClick)(), void (*longPress)()) {
    unsigned long now = millis(); // Current time
    if (key.flag) { // If ISR set the flag (key pressed)
        key.flag = false; // Clear flag
        if (key.state == KEY_IDLE) {
            key.pressStart = now; // Record press time
            key.state = KEY_PRESSED; // Go to pressed state
            key.skipSingle = false; // Reset skip flag
        } else if (key.state == KEY_WAIT_SECOND && (now - key.lastPress) < DOUBLE_CLICK_MS) {
            if (doubleClick) doubleClick(); // Call double click callback
            Serial.printf("[KeyTask] %s double click.\n", name);
            key.state = KEY_IDLE; // Reset state
            key.skipSingle = true; // Skip single click
        }
    }
    if (key.state == KEY_PRESSED && (now - key.pressStart > LONG_PRESS_MS)) {
        if (longPress) longPress(); // Call long press callback
        Serial.printf("[KeyTask] %s long press.\n", name);
        key.state = KEY_LONG_PRESSED; // Go to long pressed state
        key.skipSingle = true; // Skip single click
    }
    if ((key.state == KEY_PRESSED || key.state == KEY_LONG_PRESSED) && digitalRead(key.pin) == HIGH) {
        if (key.state == KEY_PRESSED) {
            key.lastPress = now; // Record release time
            key.state = KEY_WAIT_SECOND; // Wait for possible double click
        } else {
            key.state = KEY_IDLE; // Reset state
        }
    }
    if (key.state == KEY_WAIT_SECOND && (now - key.lastPress > DOUBLE_CLICK_MS)) {
        if (!key.skipSingle && singleClick) {
            singleClick(); // Call single click callback
            Serial.printf("[KeyTask] %s single click.\n", name);
        }
        key.state = KEY_IDLE; // Reset state
    }
}

// Callback functions for each key event
// Camera key: single = take photo, double/long = take photo with flash
void camSingleClick() { Serial.println("[KeyTask] Photo taken (single)."); DisplayTask_SavePhoto(); }
void camDoubleClick() { Serial.println("[KeyTask] Photo taken with flash (double)."); KeyTask_SetLED(true); DisplayTask_SavePhoto(); KeyTask_SetLED(false); }
void camLongPress() { Serial.println("[KeyTask] Photo taken with flash (long)."); KeyTask_SetLED(true); DisplayTask_SavePhoto(); KeyTask_SetLED(false); }
// Top key: single = set state for mode up
void topSingleClick() { keyTopState = 1; }
void topDoubleClick() {}
void topLongPress() {}
// Middle key: single = toggle gallery/preview
void midSingleClick() { keyMidState = !keyMidState; }
void midDoubleClick() {}
void midLongPress() {}
// Down key: single = set state for mode down
void downSingleClick() { keyDownState = 1; }
void downDoubleClick() {}
void downLongPress() {}

/**
 * @brief Main key input task loop. Handles debouncing and event detection for all keys.
 * Should be run as a FreeRTOS task. Continuously checks key states and calls event handlers.
 * @param pvParameters Not used (for FreeRTOS compatibility)
 */
void KeyTask(void *pvParameters) {
    while (1) {
        handleKey(keyArray[0], "Cam", keyMidState, camSingleClick, camDoubleClick, camLongPress); // Camera key
        handleKey(keyArray[1], "Top", keyTopState, topSingleClick, topDoubleClick, topLongPress); // Top key
        handleKey(keyArray[2], "Mid", keyMidState, midSingleClick, midDoubleClick, midLongPress); // Middle key
        handleKey(keyArray[3], "Down", keyDownState, downSingleClick, downDoubleClick, downLongPress); // Down key
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to yield CPU
    }
}

/**
 * @brief Control the flash LED (on/off).
 * @param on true to turn on (active low), false to turn off
 */
void KeyTask_SetLED(bool on) {
    digitalWrite(LED_FLASH_PIN, on ? LOW : HIGH); // Active low: LOW = on, HIGH = off
}