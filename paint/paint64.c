#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "buttons.h"
#include "ch32v003fun.h"
#include "colors.h"
#include "ws2812b_simple.h"

#include <stdio.h>

uint8_t foregroundColorIndex = 8;
uint8_t backgroundColorIndex = 7;
void nextForegroundColor(void) {
    foregroundColorIndex = (num_colors + foregroundColorIndex + 2) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextForegroundColor();
    }
}
void nextBackgroundColor(void) {
    backgroundColorIndex = (num_colors + backgroundColorIndex + 2) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextBackgroundColor();
    }
}

void send(void) {
    Delay_Us(1);
    WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
}

uint8_t toggle[NUM_LEDS] = {0};

int main(void) {
    SystemInit();
    ADC_init();
    clear();
    send();
    toggle[7] = 1;
    fill_color(colors[foregroundColorIndex]);
    Delay_Ms(1);
    send();
    while (1) {
        int8_t button = matrix_pressed();
        if (button == 7) {
            nextForegroundColor();
            toggle[button] = 1;
        }
        else if (button == 0) {
            nextBackgroundColor();
            toggle[button] = 0;
        }
        else {
            toggle[button] = !toggle[button];
        }
        clear();
        for (int i = 0; i < NUM_LEDS; i++) {
            set_color(i,
                toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
        }
        send();
    }
}
