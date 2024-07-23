#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "ch32v003_GPIO_branchless.h"
#include "music.h"

int main(void) {
    SystemInit();
    GPIO_ADCinit();
    while(1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            clear();
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(300);
            printf("Hello, World! %d\n", i);
            printf("PC4: %d\n", GPIO_analogRead(GPIO_Ain2_C4));
            printf("PD4: %d\n", GPIO_analogRead(GPIO_Ain7_D4));
            printf("PD5: %d\n", GPIO_analogRead(GPIO_Ain5_D5));
            printf("PD6: %d\n", GPIO_analogRead(GPIO_Ain6_D6));
            printf("PA1: %d\n", GPIO_analogRead(GPIO_Ain1_A1));
            printf("PA2: %d\n", GPIO_analogRead(GPIO_Ain0_A2));
            playMusic((noterange_t){i % 13, i % 13 + 1});
            /*
            act: pa2, gpio possible
            */
            fill_color((color_t){30, 0, 0});
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(300);
        }
    }
}
