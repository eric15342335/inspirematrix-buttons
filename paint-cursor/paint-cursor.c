#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"

color_t onColor = {100, 255, 100}; // Green
color_t offColor = {0, 0, 0}; // Black
color_t pointerColor = {100, 100, 255}; // Blue

int currentposition = 0;
uint8_t toggle[NUM_LEDS] = {0};

int main(void) {
    SystemInit();
    ADC_init();
    fill_color(onColor);
    WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(100);
    clear();
    WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
    while (1) {
        clear();
        int act_pressed = JOY_act_pressed();
        // move current position
        if (JOY_up_pressed()) {
            currentposition = (NUM_LEDS + currentposition + 8) % NUM_LEDS;
        }
        if (JOY_down_pressed()) {
            currentposition = (NUM_LEDS + currentposition - 8) % NUM_LEDS;
        }
        if (JOY_left_pressed()){
            currentposition = (NUM_LEDS + currentposition + 1) % NUM_LEDS;
        }
        if (JOY_right_pressed()) {
            currentposition = (NUM_LEDS + currentposition - 1) % NUM_LEDS;
        }
        int8_t button = matrix_pressed();
        if (button != -1) {
            printf("Button: %d\n", button);
            Delay_Ms(100);
            toggle[button] = !toggle[button];
        }
        else if (JOY_pad_released()) {
            continue;
        }
        if (act_pressed)
            toggle[currentposition] = !toggle[currentposition];
        for (int i = 0; i < NUM_LEDS; i++)
            set_color(i, toggle[i] ? onColor : offColor);
        set_color(currentposition, pointerColor);
        WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(200);
    }
}
