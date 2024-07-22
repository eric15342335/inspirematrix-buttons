#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "ch32v003fun.h"
#include "ws2812b_simple.h"
#include <stdio.h>
#include "buttons.h"
#include "colors.h"

//#define NUM_LEDS 64
#define PAD_NONE 1
#define PAD_UP 199
#define PAD_DOWN 362
#define PAD_LEFT 514
#define PAD_RIGHT 90
#define ACT_RELEASED // random values
#define ACT_PRESSED 0
#define DEVIATION 20

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

enum {none,up,down,left,right};
#define abs(x) ((x) < 0 ? -(x) : (x))
uint8_t adc_get_pad_button(void) {
    uint16_t val = adc_get_pad();
    // determine which button is pressed
    if (val == PAD_NONE) return none;
    if (abs(val - PAD_UP) < DEVIATION) return up;
    if (abs(val - PAD_DOWN) < DEVIATION) return down;
    if (abs(val - PAD_LEFT) < DEVIATION) return left;
    if (abs(val - PAD_RIGHT) < DEVIATION) return right;
    return none;
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
        printf("ACT pressed?: %ld\r\n", gpio_act_pressed());
        int pad = adc_get_pad_button();
        printf("PAD which?: %d\r\n", pad);
        // move current position
        switch (pad) {
            case up:
                currentposition = (NUM_LEDS + currentposition + 8) % NUM_LEDS;
                break;
            case down:
                currentposition = (NUM_LEDS + currentposition - 8) % NUM_LEDS;
                break;
            case left:
                currentposition = (NUM_LEDS + currentposition + 1) % NUM_LEDS;
                break;
            case right:
                currentposition = (NUM_LEDS + currentposition - 1) % NUM_LEDS;
                break;
            default:
                break;
        }
        // toggle current position
        if (gpio_act_pressed())
            toggle[currentposition] = !toggle[currentposition];
        for (int i = 0; i < NUM_LEDS; i++)
            set_color(i, toggle[i] ? onColor : offColor);
        set_color(currentposition, pointerColor);
        send();
        Delay_Ms(21);
    }
}
