#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "fonts.h"

int main(void) {
    SystemInit();
    while (1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            clear();
#define separation 1
#define space_occupied (font_width+separation)
            for (int index = 0; index < num_fonts; index++) {
                if (i > -1+space_occupied*index && i < 6+space_occupied*index) {
                    font_draw(font_list[index], colors[8*index%num_colors], (i-(space_occupied*index)) % 6 + horizontalButtons);
                }
            }
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(400);
            if (i > 49) break;
        }
    }
}
