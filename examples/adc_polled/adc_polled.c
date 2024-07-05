/*
 * Example for using ADC with polling
 * 03-27-2023 E. Brombaugh
 */

#include "ch32v003fun.h"

#include <stdio.h>

/*
 * initialize adc for polling
 */
void adc_init(void) {
    // ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOD and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;

    // PD4 is analog input chl 7
    GPIOD->CFGLR &= ~(0xf << (4 * 4)); // CNF = 00: Analog, MODE = 00: Input

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Set up single conversion on chl 7
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 7; // 0-9 for 8 ext inputs and two internals

    // set sampling time for chl 7
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 7));
    ADC1->SAMPTR2 |= 7 << (3 * 7); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // turn on ADC and set rule group to sw trig
    ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;

    // Reset calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL)
        ;

    // Calibrate
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL)
        ;

    // should be ready for SW conversion now
}

/*
 * start conversion, wait and return result
 */
uint16_t adc_get(void) {
    // start sw conversion (auto clears)
    ADC1->CTLR2 |= ADC_SWSTART;

    // wait for conversion complete
    while (!(ADC1->STATR & ADC_EOC))
        ;

    // get result
    return ADC1->RDATAR;
}

#define BUTTON_NONE 503
#define BUTTON_DEVIATION 2
#define BUTTON_DEVIATION_MINIMUM 1
// real deviation = BUTTON_DEVIATION * abs( BUTTON_NUM - BUTTON_NONE ) / BUTTON_NONE
#define BUTTON_0 1018
#define BUTTON_1 993
#define BUTTON_2 969
#define BUTTON_3 944
#define BUTTON_4 921
#define BUTTON_5 899
#define BUTTON_6 877
#define BUTTON_7 856
#define BUTTON_8 835
#define BUTTON_9 815
#define BUTTON_10 794
#define BUTTON_11 775
#define BUTTON_12 756
#define BUTTON_13 737
#define BUTTON_14 719
#define BUTTON_15 701
#define BUTTON_16 684
#define BUTTON_17 666
#define BUTTON_18 650
#define BUTTON_19 633
#define BUTTON_20 617
#define BUTTON_21 601
#define BUTTON_22 586
#define BUTTON_23 575
#define BUTTON_24 563
#define BUTTON_25 553
#define BUTTON_26 545
#define BUTTON_27 538
#define BUTTON_28 533
#define BUTTON_29 529
#define BUTTON_30 524
#define BUTTON_31 520
#define BUTTON_32 516
#define BUTTON_33 512
#define BUTTON_34 508
#define BUTTON_35 504 // Special value. This button should have no deviation.
#define BUTTON_36 500
#define BUTTON_37 497
#define BUTTON_38 494
#define BUTTON_39 490
#define BUTTON_40 487
#define BUTTON_41 484
#define BUTTON_42 481
#define BUTTON_43 478
#define BUTTON_44 474
#define BUTTON_45 470
#define BUTTON_46 467
#define BUTTON_47 463
#define BUTTON_48 459
#define BUTTON_49 455
#define BUTTON_50 451
#define BUTTON_51 446
#define BUTTON_52 442
#define BUTTON_53 437
#define BUTTON_54 431
#define BUTTON_55 425
#define BUTTON_56 419
#define BUTTON_57 412
#define BUTTON_58 405
#define BUTTON_59 397
#define BUTTON_60 388
#define BUTTON_61 377
#define BUTTON_62 366
#define BUTTON_63 352

// Declare a list which contains all the buttons
int buttons[] = {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6,
    BUTTON_7, BUTTON_8, BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12, BUTTON_13, BUTTON_14,
    BUTTON_15, BUTTON_16, BUTTON_17, BUTTON_18, BUTTON_19, BUTTON_20, BUTTON_21,
    BUTTON_22, BUTTON_23, BUTTON_24, BUTTON_25, BUTTON_26, BUTTON_27, BUTTON_28,
    BUTTON_29, BUTTON_30, BUTTON_31, BUTTON_32, BUTTON_33, BUTTON_34, BUTTON_35,
    BUTTON_36, BUTTON_37, BUTTON_38, BUTTON_39, BUTTON_40, BUTTON_41, BUTTON_42,
    BUTTON_43, BUTTON_44, BUTTON_45, BUTTON_46, BUTTON_47, BUTTON_48, BUTTON_49,
    BUTTON_50, BUTTON_51, BUTTON_52, BUTTON_53, BUTTON_54, BUTTON_55, BUTTON_56,
    BUTTON_57, BUTTON_58, BUTTON_59, BUTTON_60, BUTTON_61, BUTTON_62, BUTTON_63};

/*
 * entry
 */
int main() {
    uint32_t count = 0;

    SystemInit();

    printf("\r\r\n\nadc_polled example\n\r");

    // init systick @ 1ms rate
    printf("initializing adc...");
    adc_init();
    printf("done.\n\r");

    // Enable GPIOs
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

    // GPIO D0 Push-Pull
    GPIOD->CFGLR &= ~(0xf << (4 * 0));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 0);
    printf("looping...\n\r");
    while (1) {
        GPIOD->BSHR = 1; // Turn on GPIOs
        Delay_Ms(300);
        GPIOD->BSHR = (1 << 16); // Turn off GPIODs
        Delay_Ms(300);
        int adc = adc_get();
        printf("Count: %lu adc: %d\n\r", count++, adc);
#define abs(x) ((x) < 0 ? -(x) : (x))
        // Find out which button is pressed via the ADC value
        // perform binary search
        int left = 0;
        int closestButton = left;
        int closestDeviation = abs(adc - closestButton);
        // Perform linear search
        if (adc == BUTTON_NONE) {
            closestButton = -1;
            closestDeviation = 0;
        }
        else if (adc == BUTTON_35) {
            closestButton = 35;
            closestDeviation = 0;
        }
        else {
            for (int i = 0; i < 64; i++) {

                int deviation = abs(adc - buttons[i]);
                if (deviation < closestDeviation) {
                    closestButton = i;
                    closestDeviation = deviation;
                }
            }
        }
        // Print the closest button pressed
        printf("Button pressed: %d\n\r", closestButton);
    }
}
