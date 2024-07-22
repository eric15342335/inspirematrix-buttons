#pragma once
#include "ch32v003fun.h"
#include "buttons.h"
#include "ch32v003_GPIO_branchless.h"

#define abs(x) ((x) < 0 ? -(x) : (x))

#ifdef WS2812BSIMPLE_IMPLEMENTATION
#include "funconfig.h"
#if FUNCONF_SYSTICK_USE_HCLK != 1
#error WS2812B Driver Requires FUNCONF_SYSTICK_USE_HCLK
#endif

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
void ADC_init(void) {
    gpio_init_act();
    GPIO_ADCinit();
}

uint16_t ADC_read(void) {
    return GPIO_analogRead(GPIO_Ain7_D4);
}

uint16_t adc_get_pad(void) {
    return GPIO_analogRead(GPIO_Ain2_C4);
}

// Buttons
#define JOY_act_pressed()         (gpio_act_pressed())
#define JOY_act_released()        (!gpio_act_pressed())
#define JOY_pad_pressed()         (adc_get_pad() > 10)
#define JOY_pad_released()        (adc_get_pad() <= 10)
#define JOY_all_released()        (JOY_act_released() && JOY_pad_released())

static inline uint8_t JOY_up_pressed(void) {
 uint16_t val = adc_get_pad();
 return(   ((val > JOY_N  - JOY_DEV) && (val < JOY_N  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV)) );
}

static inline uint8_t JOY_down_pressed(void) {
 uint16_t val = adc_get_pad();
 return(   ((val > JOY_S  - JOY_DEV) && (val < JOY_S  + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_left_pressed(void) {
 uint16_t val = adc_get_pad();
 return(   ((val > JOY_W  - JOY_DEV) && (val < JOY_W  + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_right_pressed(void) {
 uint16_t val = adc_get_pad();
 return(   ((val > JOY_E  - JOY_DEV) && (val < JOY_E  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV)) );
}

int8_t matrix_pressed(void) {
    int adc;
    // average samples
    for (int8_t sample = 0; sample < 16; sample++) {
        adc += ADC_read();
    }
    adc /= 16;
    adc += 6;
    printf("ADC: %d\n", adc);
    int8_t no_button_pressed = -1;
    for (int8_t i = 0; i < 64; i++) {
        int deviation = abs(adc - buttons[i]);
        if (deviation <= BUTTON_DEVIATION) {
            return i;
        }
    }
    return no_button_pressed;
}

#endif
