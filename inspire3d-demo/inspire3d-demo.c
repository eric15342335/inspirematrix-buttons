#define WS2812BSIMPLE_IMPLEMENTATION
#include "buttons.h"
#include "ch32v003_GPIO_branchless.h"
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"

#include <stdio.h>

#define LED_PINS GPIOA, 2

#define x_max 5
#define y_max 5
#define z_max 5
#if (x_max * y_max * z_max) != NUM_LEDS
#error "x_max * y_max * z_max must equal NUM_LEDS"
// todo: fix name
#endif
// convert position of LEDs to x, y, z coordinates
void convert_pos_to_xyz(uint16_t pos, uint8_t *x, uint8_t *y, uint8_t *z) {
    *x = pos % x_max;
    *y = (pos / x_max) % y_max;
    *z = pos / (x_max * y_max);
}

#define color_multiplier 20
int main(void) {
    SystemInit();
    while (1) {
        clear();
        for (uint16_t i = 0; i < NUM_LEDS; i++) {
            uint8_t x, y, z;
            convert_pos_to_xyz(i, &x, &y, &z);
            // not using set_color to achieve maximum brightness
            led_array[i].r = x * color_multiplier;
            led_array[i].g = y * color_multiplier;
            led_array[i].b = z * color_multiplier;
            WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
            printf("Hello, World!\n");
            Delay_Ms(20);
        }
    }
}
