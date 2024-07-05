
#include "ch32v003fun.h"
#include "color_utilities.h"
#include "funconfig.h"
#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "ws2812b_simple.h"

#define NUM_LEDS 64 // Define the number of LEDs in your strip

typedef struct color_256 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

color_t led_array[NUM_LEDS];

void set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    led_array[led].r = r;
    led_array[led].g = g;
    led_array[led].b = b;
}

void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, 0, 0, 0);
    }
}

void send(void) {
    WS2812BSimpleSend( GPIOC, 6, led_array, NUM_LEDS*3 );
}

int main() {
	SystemInit();

	// Enable GPIOD (for debugging)
	/*RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	GPIOD->CFGLR &= ~(0xf<<(4*0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	GPIOD->BSHR = 1;	 // Turn on GPIOD0*/

    while (1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            set_color(i, 255, 0, 0);
            send();
            Delay_Ms(100);
            set_color(i, 0, 255, 0);
            send();
            Delay_Ms(100);
            set_color(i, 0, 0, 255);
            send();
            Delay_Ms(100);
        }
        clear();
    }
    return 0;
}
