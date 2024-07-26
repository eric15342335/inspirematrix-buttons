#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "ch32v003_GPIO_branchless.h"
#include "music.h"

int deviation = 10;
int button_new[] = {
    1023,997,965,926,895,866,835,803,
    771,740,710,678,647,617,587,555,
    526,494,460,434,400,373,341,308,
    281,246,216,186,156,124,100,69
};

int linear_search(int value, int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (abs(array[i] - value) < deviation) {
            return i;
        }
    }
    return -1;
}

int toggle[64] = {0};

int main(void) {
    SystemInit();
    GPIO_ADCinit();
    clear();
    while(1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            printf("Hello, World! %d\n", i);
            int reading = GPIO_analogRead(GPIO_Ain2_C4);
            printf("PC4: %d\n", reading);
            int button_index = linear_search(reading, button_new, sizeof(button_new) / sizeof(button_new[0]));
            if (button_index!= -1) {
                printf("Button pressed! %d \n", button_index);
                toggle[button_index] =!toggle[button_index];
                set_color(button_index, toggle[button_index]? colors[0] : (color_t){0,0,0});
            }
            /*printf("PD4: %d\n", GPIO_analogRead(GPIO_Ain7_D4));
            printf("PD5: %d\n", GPIO_analogRead(GPIO_Ain5_D5));
            printf("PD6: %d\n", GPIO_analogRead(GPIO_Ain6_D6));
            printf("PA1: %d\n", GPIO_analogRead(GPIO_Ain1_A1));
            printf("PA2: %d\n", GPIO_analogRead(GPIO_Ain0_A2));*/
            //playMusic((noterange_t){i % 13, i % 13 + 1});
            /*
            act: pa2, gpio possible
             */
            //set_color(0, colors[i % num_colors]);
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(200);
        }
    }
}
