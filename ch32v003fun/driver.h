#pragma once
#include "funconfig.h"
#include "ch32v003fun.h"
#include "buttons.h"
#include "ch32v003_GPIO_branchless.h"

#define abs(x) ((x) < 0 ? -(x) : (x))

#ifdef WS2812BSIMPLE_IMPLEMENTATION
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

uint16_t ADC_read_pad(void) {
    return GPIO_analogRead(GPIO_Ain2_C4);
}

uint16_t ADC_read_smallboard(void) {
    return GPIO_analogRead(GPIO_Ain2_C4);
}

// Buttons
#define JOY_act_pressed()         (gpio_act_pressed())
#define JOY_act_released()        (!gpio_act_pressed())
#define JOY_pad_pressed()         (ADC_read_pad() > 10)
#define JOY_pad_released()        (ADC_read_pad() <= 10)
#define JOY_all_released()        (JOY_act_released() && JOY_pad_released())

static inline uint8_t JOY_up_pressed(void) {
 uint16_t val = ADC_read_pad();
 return(   ((val > JOY_N  - JOY_DEV) && (val < JOY_N  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV)) );
}

static inline uint8_t JOY_down_pressed(void) {
 uint16_t val = ADC_read_pad();
 return(   ((val > JOY_S  - JOY_DEV) && (val < JOY_S  + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_left_pressed(void) {
 uint16_t val = ADC_read_pad();
 return(   ((val > JOY_W  - JOY_DEV) && (val < JOY_W  + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_right_pressed(void) {
 uint16_t val = ADC_read_pad();
 return(   ((val > JOY_E  - JOY_DEV) && (val < JOY_E  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV)) );
}

#define no_buttons_pressed -1
int8_t matrix_pressed(uint16_t (*matrix)(void)) {
    int64_t adc = 0;
    const int8_t samples = 5;
    int16_t readings[samples];
    int8_t valid_samples = 1;
    for (int8_t i = 0; i < samples; i++) {
        readings[i] = matrix();
        Delay_Ms(1);
    }
    for (int8_t i = 1; i < samples; i++) {
        int16_t deviation = abs(readings[i] - readings[i-1]);
        if (deviation > BUTTON_DEVIATION) {
            valid_samples = 0;
            break;
        }
    }
    if (valid_samples) {
        for (int8_t i = 0; i < samples; i++) {
            adc += readings[i];
        }
        adc /= samples;
        adc += 10;
    }
    for (int8_t i = 0; i < NUM_BUTTONS; i++) {
        int deviation = abs(adc - buttons[i]);
        if (deviation <= BUTTON_DEVIATION) {
            return i;
        }
    }
    return no_button_pressed;
}

uint16_t rnval;
uint16_t JOY_random(void) {
  rnval = (rnval >> 0x01) ^ (-(rnval & 0x01) & 0xB400);
  return rnval;
}

void JOY_setseed_default(void){
  rnval = 0x1234;
}

void JOY_setseed(uint16_t seed){
  rnval = seed;
}

#endif
