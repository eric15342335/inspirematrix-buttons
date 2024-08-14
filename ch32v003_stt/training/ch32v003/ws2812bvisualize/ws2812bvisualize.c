/**
 * Visualize audio input from a microphone on a WS2812B LED strip.
 * Analog input from a microphone is sampled at 6400 samples/sec.
 * Reading is proportional to the brightness and number of LEDs turned on.
 */

#define WS2812BSIMPLE_IMPLEMENTATION
#include "ch32v003fun.h"
#include "colors.h"
#include "ws2812b_simple.h"

#include <stdio.h>

static int result = 0, total = 0, count = 0, c = 0;
#define range_low -385
#define range_high 638
#define range_diff (range_high - range_low)
static const uint8_t _range_ratio(int val) {
    return (val - range_low) * UINT8_MAX / range_diff;
}
// initialize timer for 6400 samples/sec
void init_timer(void);

// initialize ADC for analog input from PD4 (a microphone)
void init_adc(void);

// timer interrupt handler
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));

int main() {
    SystemInit();

    init_adc();
    init_timer();

    while (1) {
        clear();
        c = _range_ratio(c);
        for (uint8_t i = 0; i < (NUM_LEDS * (c / UINT8_MAX)); i++) {
            set_color(i, (color_t){c, c, c});
        }
        WS2812BSimpleSend(GPIOC, 5, (uint8_t *)led_array, NUM_LEDS * 3);
    }
}

void init_timer(void) {
    // TIMER
    printf("Initializing timer...\r\n");
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    TIM1->ATRLR = 80; // lower = higher sample rate. 800 for 6400sam/sec
    TIM1->PSC = 10 - 1;
    TIM1->RPTCR = 0;
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->INTFR = ~TIM_FLAG_Update;
    TIM1->DMAINTENR |= TIM_IT_Update;
    TIM1->CTLR1 |= TIM_CEN;
}

void init_adc(void) {
    printf("Initializing ADC... (bjs pin PD4 I think...)\r\n");

    // ADCCLK = 24 MHz => RCC_ADCPRE divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOD and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;

    // PD4 is analog input chl 7
    GPIOD->CFGLR &= ~(0xf << (4 * 4)); // pin D4 analog-in  now

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Set sequencer to channel 7 only
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 7;

    // set sampling time for chl 7
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 7));
    ADC1->SAMPTR2 |= 7 << (3 * 7); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // Keep CALVOL register with initial value
    ADC1->CTLR2 = ADC_ADON | ADC_EXTSEL;

    // Reset calibrate
    printf("Calibrating ADC...\r\n");
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL)
        ;
    // Calibrate
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL)
        ;

    printf("Calibrating done...\r\n");
}
void TIM1_UP_IRQHandler(void) {
    if (TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
        result = ADC1->RDATAR;
        result -= 512;
        total += result;
        count++;
        if (count == 8) {
            total >>= 3;
            total += 127;
            c = total;
            total = 0;
            count = 0;
        }
        ADC1->CTLR2 |= ADC_SWSTART; // start next ADC conversion
    }
}
