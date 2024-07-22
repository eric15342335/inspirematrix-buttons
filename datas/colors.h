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

/// @brief buffer
color_t led_array[NUM_LEDS] = {0};

const color_t colors[] = {
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
const uint16_t num_colors = sizeof(colors) / sizeof(colors[0]);

/**
 * @brief Set the color of the LED strip
 * @param led Which LED to set the color of
 * @param color The color to set the LED to, in RGB format `color_t`
 */
static inline void set_color(uint8_t led, color_t color) {
    /// @todo Remove the brightness scaling
    led_array[led].r = color.r / 4;
    led_array[led].g = color.g / 4;
    led_array[led].b = color.b / 4;
}

/**
 * @brief Fill the LED strip with a single color
 * @param color The color to fill the LED strip with, in RGB format `color_t`
 */
static inline void fill_color(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, color);
    }
}

/// @brief Clear the LED strip to No Color
static inline void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t){0, 0, 0});
    }
}