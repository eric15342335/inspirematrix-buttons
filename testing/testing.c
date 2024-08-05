#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "buttons.h"
#include "ws2812b_simple.h"
#include "ch32v003_GPIO_branchless.h"
#include "music.h"

int linear_search(int value, const int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (abs(array[i] - value) < BUTTON_DEVIATION) {
            return i;
        }
    }
    return -1;
}

int toggle[64] = {0};
uint16_t act_func(void) {
    return GPIO_analogRead(GPIO_Ain0_A2);
}
int main(void) {
    SystemInit();
    GPIO_ADCinit();
    while(1) {
        clear();
        for (int i = 0; i < NUM_LEDS; i++) {
            printf("Hello, World! %d\n", i);
            uint16_t reading = multiple_ADC_reads(ADC_read_pad, 5);
            printf("PC4 (average of 5 readings): %u\n", reading);
            int button_index = linear_search(reading, buttons, sizeof(buttons) / sizeof(buttons[0]));
            if (button_index!= -1) {
                //printf("Button pressed! %d \n", button_index);
                toggle[button_index] =!toggle[button_index];
                set_color(button_index, toggle[button_index]? colors[0] : (color_t){0,0,0});
            }
            printf("PD4: %d\n", GPIO_analogRead(GPIO_Ain7_D4));
            printf("PD5: %d\n", GPIO_analogRead(GPIO_Ain5_D5));
            printf("PD6: %d\n", GPIO_analogRead(GPIO_Ain6_D6));
            printf("PA1: %d\n", GPIO_analogRead(GPIO_Ain1_A1));
            printf("PA2: %d\n", multiple_ADC_reads(act_func, 5));
            // playMusic((noterange_t){i % notes, i % notes + 1});
            playAllMusic();
            /*
            act: pa2, gpio possible
             */
            //set_color(i-1, colors[i % num_colors]);
            WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(100);
        }
    }
}
