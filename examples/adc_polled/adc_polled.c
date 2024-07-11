/**
 * @todo Document this file
 * @details
 * TARGET BOARD: CH32V003F4P6
 * I/O PERIPHERALS: InspireMatrix 8x8 LED Matrix
 * @note
 * Original description (preserved):
 * Example for using ADC with polling
 * 03-27-2023 E. Brombaugh
 */

#define WS2812BSIMPLE_IMPLEMENTATION
/// Required for the WS2812B Simple implementation to work
#define FUNCONF_SYSTICK_USE_HCLK 1
#include "ch32v003fun.h"
#include "ws2812b_simple.h"

#include <stdio.h>
#define abs(x) ((x) < 0 ? -(x) : (x))
#define BUTTON_NONE 10
#define BUTTON_DEVIATION 5
#define BUTTON_0 1019
#define BUTTON_1 995
#define BUTTON_2 970
#define BUTTON_3 946
#define BUTTON_4 923
#define BUTTON_5 900
#define BUTTON_6 875
#define BUTTON_7 856
#define BUTTON_8 835
#define BUTTON_9 814
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
#define BUTTON_23 570
#define BUTTON_24 555
#define BUTTON_25 538
#define BUTTON_26 524
#define BUTTON_27 511
#define BUTTON_28 494
#define BUTTON_29 480
#define BUTTON_30 467
#define BUTTON_31 452
#define BUTTON_32 439
#define BUTTON_33 425
#define BUTTON_34 412
#define BUTTON_35 399
#define BUTTON_36 387
#define BUTTON_37 372
#define BUTTON_38 360
#define BUTTON_39 346
#define BUTTON_40 332
#define BUTTON_41 320
#define BUTTON_42 308
#define BUTTON_43 295
#define BUTTON_44 282
#define BUTTON_45 269
#define BUTTON_46 256
#define BUTTON_47 244
#define BUTTON_48 230
#define BUTTON_49 216
#define BUTTON_50 203
#define BUTTON_51 190
#define BUTTON_52 176
#define BUTTON_53 161
#define BUTTON_54 149
#define BUTTON_55 136
#define BUTTON_56 121
#define BUTTON_57 107
#define BUTTON_58 92
#define BUTTON_59 77
#define BUTTON_60 62
#define BUTTON_61 48
#define BUTTON_62 32
#define BUTTON_63 20

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

    // Enable GPIOD and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;

    // PD2 is analog input chl 3?
    GPIOC->CFGLR &= ~(0xF << (4 * 2)); // CNF = 00: Analog, MODE = 00: Input

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Set up single conversion on chl 3
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 3; // 0-9 for 8 ext inputs and two internals

    // set sampling time for chl 3
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 3));
    ADC1->SAMPTR2 |= 7 << (3 * 3); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // turn on ADC and set rule group to software trigger
    ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;

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

/// @brief  Number of LEDs in the LED strip
#define NUM_LEDS 64
/// @brief Array of buttons corresponding to the ADC values, for linear searching
int buttons[NUM_LEDS] = {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8,
    BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12, BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16, BUTTON_17, BUTTON_18,
    BUTTON_19, BUTTON_20, BUTTON_21, BUTTON_22, BUTTON_23, BUTTON_24, BUTTON_25, BUTTON_26, BUTTON_27, BUTTON_28,
    BUTTON_29, BUTTON_30, BUTTON_31, BUTTON_32, BUTTON_33, BUTTON_34, BUTTON_35, BUTTON_36, BUTTON_37, BUTTON_38,
    BUTTON_39, BUTTON_40, BUTTON_41, BUTTON_42, BUTTON_43, BUTTON_44, BUTTON_45, BUTTON_46, BUTTON_47, BUTTON_48,
    BUTTON_49, BUTTON_50, BUTTON_51, BUTTON_52, BUTTON_53, BUTTON_54, BUTTON_55, BUTTON_56, BUTTON_57, BUTTON_58,
    BUTTON_59, BUTTON_60, BUTTON_61, BUTTON_62, BUTTON_63};

/// @brief  Structure to hold RGB color values
typedef struct color_256 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

/// @brief Array of colors for the LED strip, for sending to the LED strip
color_t led_array[NUM_LEDS] = {0};

/**
 * @brief Set the color of the LED strip
 * @param led Which LED to set the color of
 * @param color The color to set the LED to, in RGB format `color_t`
 */
void set_color(uint8_t led, color_t color) {
    /// @todo Remove the brightness scaling
    led_array[led].r = color.r / 5;
    led_array[led].g = color.g / 5;
    led_array[led].b = color.b / 5;
}

/**
 * @brief Fill the LED strip with a single color
 * @param color The color to fill the LED strip with, in RGB format `color_t`
 */
void fill_color(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, color);
    }
}

/// @brief Send the color values to the LED strip
void send(void) {
    // WS2812BSimpleSend( GPIOC, 6, led_array, NUM_LEDS*3 );
    WS2812BSimpleSend(GPIOC, 6, (uint8_t *)led_array, NUM_LEDS * 3);
}

/// @brief
static const color_t colors[] = {
    {0x00, 0x1F, 0x5F},
    {0x00, 0x2F, 0x5F},
    {0x5F, 0x2F, 0x00},
    {0x5F, 0x00, 0x00},
    {0x2F, 0x00, 0x5F},
    {0x00, 0x00, 0x5F},
    {0x00, 0x00, 0x2F},
    {0x00, 0x00, 0x00},
    {0x00, 0x4F, 0x7F},
    {0x00, 0x4F, 0x7F},
    {0x7F, 0x4F, 0x00},
    {0x7F, 0x00, 0x00},
    {0x4F, 0x00, 0x7F},
    {0x00, 0x00, 0x7F},
    {0x00, 0x00, 0x4F},
    {0x1F, 0x1F, 0x1F},
    {0x00, 0x6F, 0x9F},
    {0x00, 0x6F, 0x9F},
    {0x9F, 0x6F, 0x00},
    {0x9F, 0x00, 0x00},
    {0x6F, 0x00, 0x9F},
    {0x00, 0x00, 0x9F},
    {0x00, 0x00, 0x6F},
    {0x3F, 0x3F, 0x3F},
    {0x00, 0x8F, 0xBF},
    {0x00, 0x8F, 0xBF},
    {0xBF, 0x8F, 0x00},
    {0xBF, 0x00, 0x00},
    {0x8F, 0x00, 0xBF},
    {0x00, 0x00, 0xBF},
    {0x00, 0x00, 0x8F},
    {0x5F, 0x5F, 0x5F},
    {0x00, 0xAF, 0xDF},
    {0x00, 0xAF, 0xDF},
    {0xDF, 0xAF, 0x00},
    {0xDF, 0x00, 0x00},
    {0xAF, 0x00, 0xDF},
    {0x00, 0x00, 0xDF},
    {0x00, 0x00, 0xAF},
    {0x7F, 0x7F, 0x7F},
    {0x00, 0xCF, 0xFF},
    {0x00, 0xCF, 0xFF},
    {0xFF, 0xCF, 0x00},
    {0xFF, 0x00, 0x00},
    {0xCF, 0x00, 0xFF},
    {0x00, 0x00, 0xFF},
    {0x00, 0x00, 0xCF},
    {0x9F, 0x9F, 0x9F},
    {0x20, 0xDF, 0xFF},
    {0x20, 0xDF, 0xFF},
    {0xFF, 0xDF, 0x20},
    {0xFF, 0x20, 0x20},
    {0xDF, 0x20, 0xFF},
    {0x20, 0x20, 0xFF},
    {0x20, 0x20, 0xDF},
    {0xBF, 0xBF, 0xBF},
    {0x40, 0xEF, 0xFF},
    {0x40, 0xEF, 0xFF},
    {0xFF, 0xEF, 0x40},
    {0xFF, 0x40, 0x40},
    {0xEF, 0x40, 0xFF},
    {0x40, 0x40, 0xFF},
    {0x40, 0x40, 0xEF},
    {0xDF, 0xDF, 0xDF},
};
/// @brief Index of the foreground color
static uint8_t foregroundColorIndex = 0;
/// @brief Index of the background color
static uint8_t backgroundColorIndex = 1;
/// @brief Number of colors in the color array
static const uint16_t num_colors = sizeof(colors) / sizeof(colors[0]);

/// @brief Change the foreground color to the next color in the color array
void nextForegroundColor(void) {
    foregroundColorIndex = (num_colors + foregroundColorIndex + 1) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextForegroundColor();
    }
}

/// @brief Change the background color to the next color in the color array
void nextBackgroundColor(void) {
    backgroundColorIndex = (num_colors + backgroundColorIndex + 1) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextBackgroundColor();
    }
}

/// @brief Clear the LED strip to No Color
void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t){0, 0, 0});
    }
}

/// @brief Array of toggles for each LED
uint8_t toggle[NUM_LEDS] = {0};

/** @brief Get the number of toggles for the LED strip
 * @param adc The ADC value
 * @return The number of toggled on LEDs
 * @note This function is not used in the main loop
 */
int get_num_toggle(int adc) {
    // calculate num of '1' in toggle[]
    int count = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        if (toggle[i] == 1)
            count++;
    }
    if (adc > 500)
        count = 0 - count;
    // return count;
    return 0; // Disabled for now
}
#define CH32V003F4P6_ENABLE_ONBOARD_LIGHT
void onBoardLightInit(void) {
#ifdef CH32V003F4P6_ENABLE_ONBOARD_LIGHT
    // Enable PD4 light
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    // GPIO D4 Push-Pull
    GPIOD->CFGLR &= ~(0xf << (4 * 4));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 4);
#endif
}
void onBoardLightOn(void) {
#ifdef CH32V003F4P6_ENABLE_ONBOARD_LIGHT
    GPIOD->BSHR = (1 << 4);
#endif
}
void onBoardLightOff(void) {
#ifdef CH32V003F4P6_ENABLE_ONBOARD_LIGHT
    // Turn off the light on the MCU
    GPIOD->BSHR = (1 << (4 + 16));
#endif
}

/// @brief Main function
int main(void) {
    // Number of times the loop has run
    uint32_t count = 0;
    SystemInit();
    onBoardLightInit();
    onBoardLightOn();

    printf("\r\r\n\nadc_polled example\n\r");
    // init systick @ 1ms rate
    printf("initializing adc...");
    adc_init();
    printf("done.\n\r");

    clear();
    fill_color(colors[backgroundColorIndex]);
    send();
    Delay_Ms(200);
    onBoardLightOff();

    clear();
    send();

    printf("looping...\n\r");
    while (1) {
        onBoardLightOn();
        // Delay_Ms(1);
        uint16_t adc;
        while (1) {
            uint16_t adc2;
            adc = adc_get();
            // printf("adc: %d\n", adc);
            Delay_Us(1);
            adc2 = adc_get();
            // Check if the ADC value is the same
            if (adc == adc2)
                break;
        }
        // Apply offset to the ADC value
        // adc += 4;
        // printf("Count: %lu adc: %d, new adc:%d\n", count++, adc, adc-get_num_toggle(adc));
        // apply some hysteresis to the ADC value
        // adc -= get_num_toggle(adc);
        // Find out which button is pressed via the ADC value
        // Perform linear search
        int button = -1;
        if (abs(adc - BUTTON_NONE) <= BUTTON_DEVIATION) {
            continue;
        }
        else {
            for (int i = 0; i < 64; i++) {
                int deviation = abs(adc - buttons[i]);
                if (deviation <= BUTTON_DEVIATION) {
                    button = i;
                    break;
                }
            }
            if (button == -1) {
                // printf("No button pressed\n\r");
                continue;
            }
        }
        Delay_Ms(20);
        onBoardLightOff();
        // Print the closest button pressed
        printf("Count: %lu adc: %d\n", count++, adc);
        printf("Button pressed: %d, deviation: %d\n\r", button, abs(adc - buttons[button]));
        // Paint color selection for background and foreground.
        if (button == 7) {
            nextForegroundColor();
            // Force the button to be foreground color
            toggle[button] = 1;
        }
        else if (button == 0) {
            nextBackgroundColor();
            // Force the button to be background color
            toggle[button] = 0;
        }
        else {
            toggle[button] = !toggle[button];
        }
        clear();
        // Set the color of the LED strip to the button pressed
        for (int i = 0; i < NUM_LEDS; i++) {
            set_color(i, toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
        }
        send();
        /*
        // Print the 8x8 LED strip
        printf("----------------\n\r");
        for (int button = 63; button >= 0; button--) {
            printf("%d ", toggle[button]);
            if (button % 8 == 0) printf("\n\r");
        }
        printf("----------------\n\r");
        // Upscale the 8x8 LED strip to a 16x16 LED strip and print it
        printf("----------------\n\r");
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                printf("%d ", toggle[i / 2 * 8 + j / 2]);
            }
            printf("\n\r");
        }
        printf("----------------\n\r");*/
        // uint8_t image[256] = {0};
        //  upscale the 8x8 LED strip to a 16x16 LED strip by enlarging each LED to a
        //  2x2 LED
        /*for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                image[i * 16 + j] = toggle[i / 2 * 8 + j / 2];
            }
        }
        for (int i = 255; i >= 0; i--) {
            printf(image[i] ? "120 " : "-20 ");
            Delay_Ms(1);
        }
        printf("\n");*/
    }
}
