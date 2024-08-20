#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"
#include <stdio.h>

#define LED_PINS GPIOA, 2

void display_paint_icon(void) {
    clear();
    const color_t paint_color = {.r = 0, .g = 255, .b = 0};
    set_color(49, paint_color);
    set_color(54, paint_color);
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

int main(void) {
    SystemInit();
    display_paint_icon();
}
