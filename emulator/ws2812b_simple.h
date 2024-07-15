#pragma once
#include "colors.h"
#include "buttons.h"

/**
 * @brief Set the color of the LED strip
 * @param led Which LED to set the color of
 * @param color The color to set the LED to, in RGB format `color_t`
 */
void set_color(uint8_t led, color_t color) {
    /// @todo Remove the brightness scaling
    led_array[led].r = color.r;
    led_array[led].g = color.g;
    led_array[led].b = color.b;
}

/// @brief Clear the LED strip to No Color
void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t){0, 0, 0});
    }
}

/**
 * @brief Fill the LED strip with a single color
 * @param color The color to fill the LED strip with, in RGB format `color_t`
 */
void fill_color(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, color);
    }
}

extern uint8_t foregroundColorIndex;
extern uint8_t backgroundColorIndex;

/// @brief Send the color values to the LED strip
void send(void) {
    // WS2812BSimpleSend(GPIOC, 6, (uint8_t *)led_array, NUM_LEDS * 3);
#define BORDER_X 2
    printf("\\y");
    for (uint8_t i = horizontalButtons; i > 0; i--) {
        printf("%d", i-1);
    }
    printf(" \nx");
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
    for (uint8_t y = verticalButtons; y > 0; y--) {
        printf("%d|", y - 1);
        for (uint8_t x = horizontalButtons; x > 0; x--) {
            uint8_t led = (y - 1) * horizontalButtons + (x - 1);
            if (led < NUM_LEDS &&
                (led_array[led].r || led_array[led].g || led_array[led].b)) {
                printf("\033[38;2;%d;%d;%dm\u2588\033[0m", led_array[led].r,
                    led_array[led].g, led_array[led].b);
            }
            else {
                printf(" ");
            }
        }
        printf("|");
        if (y == verticalButtons / 2 ) {
            printf("   foreground: \033[38;2;%d;%d;%dm\u2588\033[0m",
                colors[foregroundColorIndex].r, colors[foregroundColorIndex].g,
                colors[foregroundColorIndex].b);
            printf(", RGB(%d, %d, %d)", colors[foregroundColorIndex].r,
                colors[foregroundColorIndex].g, colors[foregroundColorIndex].b);
        }
        if (y == verticalButtons / 2 + 1) {
            printf("   background: \033[38;2;%d;%d;%dm\u2588\033[0m",
                colors[backgroundColorIndex].r, colors[backgroundColorIndex].g,
                colors[backgroundColorIndex].b);
            printf(", RGB(%d, %d, %d)", colors[backgroundColorIndex].r,
                colors[backgroundColorIndex].g, colors[backgroundColorIndex].b);
        }
        if (y == verticalButtons / 2 + 2) {
            printf("   button calculation: y*%d+x", horizontalButtons);
        }
        printf("\n");
    }
    printf(" ");
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
}
