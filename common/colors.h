/// @brief Hardcoded colors for the LED strip
#pragma once
#include "buttons.h"
#include <stdint.h>

/// @brief Structure to hold RGB color values
typedef struct color_256 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;


/// @brief Array of colors for the LED strip, for sending to the LED strip
color_t led_array[NUM_LEDS] = {0};

/// @brief Array of colors for the LED strip, for sending to the LED strip
static const color_t colors[] = {
    // Red (8 shades)
    {255, 0, 0}, {223, 0, 0}, {191, 0, 0}, {159, 0, 0}, {127, 0, 0}, {95, 0, 0},
    {63, 0, 0}, {31, 0, 0},

    // Green (8 shades)
    {0, 255, 0}, {0, 223, 0}, {0, 191, 0}, {0, 159, 0}, {0, 127, 0}, {0, 95, 0},
    {0, 63, 0}, {0, 31, 0},

    // Blue (8 shades)
    {0, 0, 255}, {0, 0, 223}, {0, 0, 191}, {0, 0, 159}, {0, 0, 127}, {0, 0, 95},
    {0, 0, 63}, {0, 0, 31},

    // Yellow (8 shades)
    {255, 255, 0}, {223, 223, 0}, {191, 191, 0}, {159, 159, 0}, {127, 127, 0},
    {95, 95, 0}, {63, 63, 0}, {31, 31, 0},

    // Cyan (8 shades)
    {0, 255, 255}, {0, 223, 223}, {0, 191, 191}, {0, 159, 159}, {0, 127, 127},
    {0, 95, 95}, {0, 63, 63}, {0, 31, 31},

    // Magenta (8 shades)
    {255, 0, 255}, {223, 0, 223}, {191, 0, 191}, {159, 0, 159}, {127, 0, 127},
    {95, 0, 95}, {63, 0, 63}, {31, 0, 31},

    // White (8 shades)
    {255, 255, 255}, {223, 223, 223}, {191, 191, 191}, {159, 159, 159}, {127, 127, 127},
    {95, 95, 95}, {63, 63, 63}, {31, 31, 31},

    // Orange (8 shades)
    {255, 127, 0}, {223, 111, 0}, {191, 95, 0}, {159, 79, 0}, {127, 63, 0}, {95, 47, 0},
    {63, 31, 0}, {31, 15, 0}};

/// @brief Number of colors in the color array
static const uint16_t num_colors = sizeof(colors) / sizeof(colors[0]);
