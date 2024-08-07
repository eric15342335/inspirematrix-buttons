#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"

#include <stdio.h>

color_t foreground = {100, 0, 0};
color_t background = {0, 0, 100};
typedef struct {
    enum { FOREGROUND_LAYER, BACKGROUND_LAYER } layer;
    color_t color;
} canvas_t;
canvas_t canvas[NUM_LEDS] = {0};
void flushCanvas(void) {
    for (int i = 1; i < NUM_LEDS; i++) {
        set_color(i - 1, canvas[i].color);
    }
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
}

void displayColorPalette(void) {
    for (int i = 1; i < NUM_LEDS; i++) {
        set_color(i - 1, colors[i]);
    }
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
    printf("Color palette displayed\n");
}

void colorPaletteSelection(color_t * selectedColor) {
    displayColorPalette();
    while (1) {
        int8_t button = matrix_pressed(ADC_read_smallboard);
        if (button != no_button_pressed) {
            *selectedColor = colors[button];
            break;
        }
        Delay_Ms(200);
    }
    printf("Selected color: R:%d G:%d B:%d\n", selectedColor->r, selectedColor->g,
        selectedColor->b);
    flushCanvas();
}

int main(void) {
    SystemInit();
    ADC_init();
    clear();
    while (1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            set_color(i, color_divide(colors[i], 10));
        }
        WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
    }
    Delay_Ms(500);
    printf("\nBackground Initialized\n");
    for (int i = 1; i < NUM_LEDS; i++) {
        canvas[i].layer = BACKGROUND_LAYER;
        canvas[i].color = (color_t){0, 0, 0};
    }
    flushCanvas();
    while (1) {
        Delay_Ms(200);
        // printf("Foreground color: R:%d G:%d B:%d\n", foreground.r, foreground.g,
        // foreground.b); printf("Background color: R:%d G:%d B:%d\n", background.r,
        // background.g, background.b);
        int8_t user_input = matrix_pressed(ADC_read_smallboard);
        if (user_input == no_button_pressed) {
            if (JOY_Y_pressed()) {
                colorPaletteSelection(&foreground);
            }
            else if (JOY_X_pressed()) {
                colorPaletteSelection(&background);
            }
            else if (JOY_down_pressed()) {
                printf("System reset\n");
                NVIC_SystemReset();
            }
            continue;
        }
        // user sets canvas color
        if (canvas[user_input].layer == BACKGROUND_LAYER) {
            canvas[user_input].layer = FOREGROUND_LAYER;
            canvas[user_input].color = foreground;
        }
        else {
            canvas[user_input].layer = BACKGROUND_LAYER;
            canvas[user_input].color = background;
        }
        printf("Canvas[%d] set to %s layer\n", user_input,
            canvas[user_input].layer == FOREGROUND_LAYER ? "FOREGROUND" : "BACKGROUND");
        printf("Canvas color set to R:%d G:%d B:%d\n", canvas[user_input].color.r,
            canvas[user_input].color.g, canvas[user_input].color.b);
        flushCanvas();
    }
}
