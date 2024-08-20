#define WS2812BSIMPLE_IMPLEMENTATION
#define CH32V003_I2C_IMPLEMENTATION
#include "buttons.h"
#include "ch32v003_GPIO_branchless.h"
#include "colors.h"
#include "driver.h"
#include "music.h"
#include "ws2812b_simple.h"
#include "ch32v003_i2c.h"

#include <stdio.h>

#define LED_PINS GPIOA, 2

int linear_search(int value, const int array[], int size) {
    for (int i = 0; i < size; i++) {
        if (abs(array[i] - value) < BUTTON_DEVIATION) {
            return i;
        }
    }
    return -1;
}

int toggle[64] = {0};
uint16_t act_func(void) { return GPIO_analogRead(GPIO_Ain0_A2); }
uint16_t external_matrix_button(void) { return GPIO_analogRead(GPIO_Ain1_A1); }
int main(void) {
    SystemInit();
    while(1) {
        i2c_init();
        i2c_scan();
        Delay_Ms(1000);
    }
    while(0) {
        playAllMusic();
    }
    JOY_setseed_default();
    while (0) {
        fill_color((color_t){JOY_random() % 10, JOY_random() % 10, JOY_random() % 10});
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        printf("Hello, World!\n");
        Delay_Ms(300);
    }
    GPIO_ADCinit();
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    while (1) {
        clear();
        for (int i = 0; i < NUM_LEDS; i++) {
            printf("Hello, World! %d\n", i);
            /*uint16_t reading = multiple_ADC_reads(external_matrix_button, 5);
            printf("PA1 (average of 5 readings): %u\n", reading);
            int button_index =
                linear_search(reading, buttons, sizeof(buttons) / sizeof(buttons[0]));
            if (button_index != -1) {
                // printf("Button pressed! %d \n", button_index);
                toggle[button_index] = !toggle[button_index];
                set_color(
                    button_index, toggle[button_index] ? colors[0] : (color_t){0, 0, 0});
            }*/
            
            printf("PC4: %u\n", GPIO_analogRead(GPIO_Ain2_C4));
            //printf("PD4: %d\n", GPIO_analogRead(GPIO_Ain7_D4));
            //printf("PD5: %d\n", GPIO_analogRead(GPIO_Ain5_D5));
            //printf("PD6: %d\n", GPIO_analogRead(GPIO_Ain6_D6));
            //printf("PA1: %d\n", GPIO_analogRead(GPIO_Ain1_A1));
            //printf("PA2: %d\n", GPIO_analogRead(GPIO_Ain0_A2));
            printf("PD3: %d\n", GPIO_analogRead(GPIO_Ain4_D3));
            printf("PD2: %d\n", GPIO_analogRead(GPIO_Ain3_D2));
            // playMusic((noterange_t){i % notes, i % notes + 1});
            // playAllMusic();
            /*
            act: pa2, gpio possible
             */
            //set_color(i-1, colors[i % num_colors]);
            //WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
            Delay_Ms(200);
        }
    }
}
