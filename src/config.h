// config.h - Global configuration
// This header defines global configuration options for the ESP32 camera project.
// It allows selection of camera model and display driver.
//
// Key features:
// - Camera model selection (OV2640 or OV5640)
// - Display driver selection (ST7789, ILI9341, etc.)
// - Used by camera and display modules for hardware compatibility

#pragma once // Prevent multiple inclusion of this header

// Camera model selection (uncomment the one you use)
// #define OV5640 // Uncomment for OV5640 camera module
#define OV2640   // Uncomment for OV2640 camera module

// Display driver selection (see TFT_eSPI User_Setup.h for details)
// file at ".pio/libdeps/esp32-s3-devkitc-1/TFT_eSPI/User_Setup.h"
// #define ST7789_DRIVER // Uncomment for ST7789 display
// #define ILI9341_DRIVER // Uncomment for ILI9341 display