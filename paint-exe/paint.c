#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"

int currentposition = 0;
uint8_t toggle[NUM_LEDS] = {0};

int main(void) {
    SystemInit();
    ADC_init();
    fill_color((color_t){20,20,20});
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
}
