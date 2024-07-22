#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "ch32v003fun.h"
#include "ws2812b_simple.h"
#include <stdio.h>
#include "buttons.h"
#include "colors.h"

void set_color(uint8_t led, color_t color) {
    led_array[led].r = color.r;
    led_array[led].g = color.g;
    led_array[led].b = color.b;
}

color_t onColor = {5, 5, 5};
color_t offColor = {0, 0, 0};
color_t pointerColor = {0, 0, 5};

void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, offColor);
    }
}

void fill(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, color);
    }
}

void send(void) {
    WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
}

int currentposition = 0;
uint8_t toggle[NUM_LEDS] = {0};

int main(void) {
    SystemInit();
    adc_init_pad();
    gpio_init_act();
    fill(onColor);
    send();
    Delay_Ms(100);
    clear();
    send();
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
        // toggle current position
        if (act_pressed)
            toggle[currentposition] = !toggle[currentposition];
        for (int i = 0; i < NUM_LEDS; i++)
            set_color(i, toggle[i] ? onColor : offColor);
        set_color(currentposition, pointerColor);
        send();
        Delay_Ms(10);
    }
}
