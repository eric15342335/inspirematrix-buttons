#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include <stdio.h>
#include "driver.h"

color_t onColor = {5, 5, 5};
color_t offColor = {0, 0, 0};
color_t pointerColor = {0, 0, 5};

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
        printf("ADC reading: %d; ", adc_get_pad());
        int act_pressed = JOY_act_pressed();
        printf("ACT pressed?: %d\r\n",act_pressed);
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
        if (act_pressed)
            toggle[currentposition] = !toggle[currentposition];
        for (int i = 0; i < NUM_LEDS; i++)
            set_color(i, toggle[i] ? onColor : offColor);
        set_color(currentposition, pointerColor);
        WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(10);
    }
}
