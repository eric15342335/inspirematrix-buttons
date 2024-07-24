#define WS2812BSIMPLE_IMPLEMENTATION
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "fonts.h"

int main(void) {
    SystemInit();
    while (1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            clear();
            if (i < 6) {
                font_draw(font_1, colors[0], i % 6 + 8);
            }
            if (i > 3 && i < 10) {
                font_draw(font_2, colors[8], (i-4) % 6 + 8);
            }
            if (i > 8 && i < 15) {
                font_draw(font_3, colors[16], (i-9) % 6 + 8);
            }
            if (i > 13 && i < 20) {
                font_draw(font_4, colors[24], (i-14) % 6 + 8);
            }
            if (i > 18 && i < 25) {
                font_draw(font_5, colors[32], (i-19) % 6 + 8);
            }
            if (i > 23 && i < 30) {
                font_draw(font_6, colors[40], (i-24) % 6 + 8);
            }
            if (i > 28 && i < 35) {
                font_draw(font_7, colors[48], (i-29) % 6 + 8);
            }
            if (i > 33 && i < 40) {
                font_draw(font_8, colors[54], (i-34) % 6 + 8);
            }
            if (i > 38 && i < 45) {
                font_draw(font_9, colors[62], (i-39) % 6 + 8);
            }
            if (i > 43 && i < 50) {
                font_draw(font_0, colors[0], (i-44) % 6 + 8);
            }
            if (i > 48 && i < 55) {
                font_draw(font_X, colors[8], (i-49) % 6 + 8);
            }
            if (i > 53 && i < 60) {
                font_draw(font_D, colors[16], (i-54) % 6 + 8);
            }
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(200);
        }
    }
}