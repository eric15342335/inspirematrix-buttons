#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "fonts.h"
#include "ws2812b_simple.h"

#include <stdio.h>

#define separation 1
#define space_occupied (font_width + separation)
#define last_appear_x (horizontalButtons - space_occupied + 2)
#define INSPIRE_MATRIX_HEIGHT 8
int main(void) {
    SystemInit();
    // Test for each LEDs
    while (0) {
        for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
            clear();
            Delay_Us(1);
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Us(1);
            set_color(ledIndex, colors[ledIndex % num_colors]);
            Delay_Ms(100);
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(100);
        }
    }
    while (1) {
        for (int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
            clear();
            for (int fontIndex = 0; fontIndex < num_fonts; fontIndex++) {
                int verticalOffset = 0;
                const int fontStartPosition = space_occupied * fontIndex;
                int adjustedLedIndex = ledIndex - fontStartPosition;
                while (adjustedLedIndex >= 0) {
                    if (adjustedLedIndex < last_appear_x) {
                        font_draw(font_list[fontIndex],
                            colors[8 * fontIndex % num_colors],
                            adjustedLedIndex % last_appear_x +
                                horizontalButtons * verticalOffset);
                        break;
                    }
                    // verticalOffset += font_height + separation;
                    verticalOffset += INSPIRE_MATRIX_HEIGHT;
                    adjustedLedIndex -= horizontalButtons;
                }
            }
            printf("%d\n", ledIndex);
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(100);
            if (ledIndex > 76) {
                break;
            }
        }
    }
}
