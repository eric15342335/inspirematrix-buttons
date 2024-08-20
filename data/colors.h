/// @brief This file contains the definition of color_256 struct and some preset color
/// values
#pragma once
#include "buttons.h"

#include <stdint.h>

typedef struct color_256 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

/// @brief buffer
color_t led_array[NUM_LEDS] = {0};

// clang-format off
const color_t colors[NUM_LEDS] = {
    {3,15,53},{3,29,44},{0,28,14},{57,34,0},{54,19,0},{57,7,0},{34,6,55},{13,13,13},
    {1,42,108},{1,61,95},{3,55,32},{103,73,3},{107,36,4},{107,18,14},{62,16,106},{29,30,35},
    {9,55,166},{14,85,147},{5,81,42},{147,113,5},{161,60,18},{149,39,22},{104,32,168},{54,55,60},
    {18,82,214},{5,122,199},{0,110,57},{200,148,2},{210,86,24},{216,53,48},{135,51,207},{83,83,95},
    {25,107,255},{1,161,237},{2,131,74},{249,183,1},{253,109,39},{255,74,68},{169,76,252},{115,121,135},
    {62,121,251},{42,175,244},{33,160,107},{249,197,44},{255,128,65},{253,100,86},{176,101,254},{146,153,163},
    {104,159,250},{92,188,246},{84,181,138},{246,212,79},{254,157,112},{252,140,129},{199,135,255},{183,184,189},
    {167,191,255},{151,212,255},{144,204,176},{254,222,147},{255,195,158},{255,177,177},{214,178,252},{212,211,217}
};
// clang-format on

const uint16_t num_colors = NUM_LEDS;

static inline color_t color_divide(color_t color, uint8_t divider) {
    return (color_t){color.r / divider, color.g / divider, color.b / divider};
}

static inline void set_color(uint8_t led, color_t color) {
    uint8_t divider = 8;
    led_array[led].r = color.r / divider;
    led_array[led].g = color.g / divider;
    led_array[led].b = color.b / divider;
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