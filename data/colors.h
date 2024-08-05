/// @brief This file contains the definition of color_256 struct and some preset color values
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

const color_t colors[NUM_LEDS] = {
    // Row 0: Dark colors
    {0, 0, 0},       {32, 32, 32},     {64, 64, 64},     {96, 96, 96},     {128, 128, 128}, {160, 160, 160}, {192, 192, 192}, {224, 224, 224},
    // Row 1: Dark reds
    {32, 0, 0},      {64, 0, 0},       {96, 0, 0},       {128, 0, 0},      {160, 0, 0},     {192, 0, 0},     {224, 0, 0},     {255, 0, 0},
    // Row 2: Dark greens
    {0, 32, 0},      {0, 64, 0},       {0, 96, 0},       {0, 128, 0},      {0, 160, 0},     {0, 192, 0},     {0, 224, 0},     {0, 255, 0},
    // Row 3: Dark blues
    {0, 0, 32},      {0, 0, 64},       {0, 0, 96},       {0, 0, 128},      {0, 0, 160},     {0, 0, 192},     {0, 0, 224},     {0, 0, 255},
    // Row 4: Dark yellows
    {32, 32, 0},     {64, 64, 0},      {96, 96, 0},      {128, 128, 0},    {160, 160, 0},   {192, 192, 0},   {224, 224, 0},   {255, 255, 0},
    // Row 5: Dark cyans
    {0, 32, 32},     {0, 64, 64},      {0, 96, 96},      {0, 128, 128},    {0, 160, 160},   {0, 192, 192},   {0, 224, 224},   {0, 255, 255},
    // Row 6: Dark magentas
    {32, 0, 32},     {64, 0, 64},      {96, 0, 96},      {128, 0, 128},    {160, 0, 160},   {192, 0, 192},   {224, 0, 224},   {255, 0, 255},
    // Row 7: Bright colors
    {255, 255, 255}, {224, 224, 224},  {192, 192, 192},  {160, 160, 160},  {128, 128, 128}, {96, 96, 96},    {64, 64, 64},    {32, 32, 32}
};

const uint16_t num_colors = NUM_LEDS;

static inline void set_color(uint8_t led, color_t color) {
    uint8_t divider = 1;
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