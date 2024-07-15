#pragma once
#include "ch32v003fun.h"

// prototype
void adc_cal(void);
void adc_init(void);
uint16_t adc_get(void);

#ifdef WS2812BSIMPLE_IMPLEMENTATION

#include "funconfig.h"

#if FUNCONF_SYSTICK_USE_HCLK != 1
#error WS2812B Driver Requires FUNCONF_SYSTICK_USE_HCLK
#endif

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

/// @brief initialize adc for polling
void adc_init(void) {
    // ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOC and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;

    // PD2 is analog input chl 3?
    GPIOD->CFGLR &= ~(0xF << (4 * 2)); // CNF = 00: Analog, MODE = 00: Input

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
    //ADC1->CTLR2 = ADC_DataAlign_Left; useless line

    // Set up single conversion on chl 3
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 3; // 0-9 for 8 ext inputs and two internals
    // 8: 370 (Vref)
    // 9: 512 (Vcal)

    // set sampling time for chl 3
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 3));
    ADC1->SAMPTR2 |= 7 << (3 * 3); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // turn on ADC and set rule group to software trigger
    ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;
    //Delay_Us(10);
    adc_cal();
    // should be ready for SW conversion now
}

/// @brief start conversion, wait and return result
uint16_t adc_get(void) {
    /// start sw conversion (auto clears)
    ADC1->CTLR2 |= ADC_SWSTART;

    /// wait for conversion complete
    while (!(ADC1->STATR & ADC_EOC))
        ;

    /// get result
    return ADC1->RDATAR;
}

#endif
