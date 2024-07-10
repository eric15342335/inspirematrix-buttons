#define WS2812BSIMPLE_IMPLEMENTATION
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "ch32v003fun.h"
#include "ws2812b_simple.h"
#include <stdio.h>

#define NUM_LEDS 64
#define PAD_NONE 1
#define PAD_UP 199
#define PAD_DOWN 362
#define PAD_LEFT 514
#define PAD_RIGHT 90
#define ACT_RELEASED // random values
#define ACT_PRESSED 0
#define DEVIATION 20

typedef struct color_256 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

color_t led_array[NUM_LEDS] = {0};

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
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
}

void adc_cal(void) {
    // Reset calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL)
        ;

    // Calibrate
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL)
        ;
}

// initialize adc for polling
void adc_init_pad(void) {
    // ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOC and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1;

    // PC4 is analog input chl 2
    GPIOC->CFGLR &= ~(0xF << (4 * 4)); // CNF = 00: Analog, MODE = 00: Input

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    uint8_t channel = 2;
    // Set up single conversion on chl 3
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = channel; // 0-9 for 8 ext inputs and two internals

    // set sampling time for chl 3
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * channel));
    ADC1->SAMPTR2 |= 7 << (3 * channel); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // turn on ADC and set rule group to sw trig
    ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;

    adc_cal();

    // should be ready for SW conversion now
}

// start conversion, wait and return result
uint16_t adc_get_pad(void) {
    // start sw conversion (auto clears)
    ADC1->CTLR2 |= ADC_SWSTART;

    // wait for conversion complete
    while (!(ADC1->STATR & ADC_EOC))
        ;

    // get result
    return ADC1->RDATAR;
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

// README: ACT Button is connected to PA2 !!!!!
void gpio_init_act(void) {
    // Enable GPIOA
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;

    // PA2 is input
    GPIOA->CFGLR &= ~(0xF << (4 * 2)); // CNF = 01: Floating input, MODE = 00: Input
	GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_PUPD)<<(4*2);
    GPIOA->BSHR = ((uint32_t)1<<2);
}
// CHANGE THIS TOO IF YOU DONT USE PA2 FOR ACT BUTTON
uint32_t gpio_act_pressed(void) {
    // check the value of pa2 is low
    //return ((GPIOA->INDR & (1 << 2)) == 0);
    return !(GPIOA->INDR >> 2);
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
        printf("PAD which?: %d\r\n", adc_get_pad_button());
        // move current position
        switch (adc_get_pad_button()) {
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
