#ifndef DRIVER_H
#define DRIVER_H

// Display driver configuration for Seeed XIAO ESP32-C3 with 7.5" ePaper
// This configuration is required by the Seeed_GFX library

// Define the board and screen combination
// 502 = 7.5 inch monochrome ePaper (800x480)
#define BOARD_SCREEN_COMBO 502

// Enable XIAO ePaper driver board support
#define USE_XIAO_EPAPER_DRIVER_BOARD

// Enable ePaper functionality
#define EPAPER_ENABLE

#endif  // DRIVER_H
