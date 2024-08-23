/// @brief This file contains the definition of color_256 struct and some preset color
/// values
#pragma once
#include "buttons.h"

#include <stdint.h>

/// @brief NOT FOLLOWING THE R,G,B ORDER!!!
/// See here: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
typedef struct color_256 {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} color_t;

/// @brief buffer
color_t led_array[NUM_LEDS] = {0};

// clang-format off
const color_t colors[NUM_LEDS] = {
    {15, 3, 53}, {29, 3, 44}, {28, 0, 14}, {34, 57, 0}, {19, 54, 0}, {7, 57, 0}, {6, 34, 55}, {0, 0, 0},
    {42, 1, 108}, {61, 1, 95}, {55, 3, 32}, {73, 103, 3}, {36, 107, 4}, {18, 107, 14}, {16, 62, 106}, {30, 29, 35},
    {55, 9, 166}, {85, 14, 147}, {81, 5, 42}, {113, 147, 5}, {60, 161, 18}, {39, 149, 22}, {32, 104, 168}, {55, 54, 60},
    {82, 18, 214}, {122, 5, 199}, {110, 0, 57}, {148, 200, 2}, {86, 210, 24}, {53, 216, 48}, {51, 135, 207}, {83, 83, 95},
    {107, 25, 255}, {161, 1, 237}, {131, 2, 74}, {183, 249, 1}, {109, 253, 39}, {74, 255, 68}, {76, 169, 252}, {121, 115, 135},
    {121, 62, 251}, {175, 42, 244}, {160, 33, 107}, {197, 249, 44}, {128, 255, 65}, {100, 253, 86}, {101, 176, 254}, {153, 146, 163},
    {159, 104, 250}, {188, 92, 246}, {181, 84, 138}, {212, 246, 79}, {157, 254, 112}, {140, 252, 129}, {135, 199, 255}, {184, 183, 189},
    {191, 167, 255}, {212, 151, 255}, {204, 144, 176}, {222, 254, 147}, {195, 255, 158}, {177, 255, 177}, {178, 214, 252}, {211, 212, 217}

};
// clang-format on

const uint16_t num_colors = NUM_LEDS;

color_t color_divide(color_t color, uint8_t divider) {
    return (color_t){color.r / divider, color.g / divider, color.b / divider};
}

#define smaller(x, y) ((x) < (y) ? (x) : (y))
void set_color(uint8_t led, color_t color) {
    uint8_t divider = 8;
    led_array[led].r = smaller(color.r, color.r / divider);
    led_array[led].g = smaller(color.g, color.g / divider);
    led_array[led].b = smaller(color.b, color.b / divider);
}

static inline void fill_color(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, color);
    }
}

static inline void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t){0, 0, 0});
    }
}