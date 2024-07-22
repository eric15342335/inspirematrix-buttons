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
// My custom define, see onBoardLight*() functions;
#define CH32V003F4P6_ENABLE_ONBOARD_LIGHT
#include "buttons.h"
#include "ch32v003fun.h"
#include "colors.h"
#include "ws2812b_simple.h"

#include <stdio.h>

/// @brief Index of the foreground color
uint8_t foregroundColorIndex = 8;
/// @brief Index of the background color
uint8_t backgroundColorIndex = 7;

/// @brief Change the foreground color to the next color in the color array
void nextForegroundColor(void) {
    foregroundColorIndex = (num_colors + foregroundColorIndex + 2) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextForegroundColor();
    }
}

/// @brief Change the background color to the next color in the color array
void nextBackgroundColor(void) {
    backgroundColorIndex = (num_colors + backgroundColorIndex + 2) % num_colors;
    // fore&background cannot be the same
    if (foregroundColorIndex == backgroundColorIndex) {
        nextBackgroundColor();
    }
}

#ifdef CH32V003F4P6_ENABLE_ONBOARD_LIGHT
void onBoardLightInit(void) {
    // Enable PD4 light
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    // GPIO D4 Push-Pull
    GPIOD->CFGLR &= ~(0xf << (4 * 4));
    GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 4);
}
void onBoardLightOn(void) { GPIOD->BSHR = (1 << 4); }
void onBoardLightOff(void) {
    // Turn off the light on the MCU
    GPIOD->BSHR = (1 << (4 + 16));
}
#else
void onBoardLightInit(void) {}
void onBoardLightOn(void) {}
void onBoardLightOff(void) {}
#endif

/// @brief Send the color values to the LED strip
void send(void) {
    Delay_Us(1);
    WS2812BSimpleSend(GPIOC, 6, (uint8_t *)led_array, NUM_LEDS * 3);
}

/// @brief Array of toggles for each LED
uint8_t toggle[NUM_LEDS] = {0};

int main(void) {
    uint32_t count = 0;
    SystemInit();
    onBoardLightInit();
    onBoardLightOn();
    ADC_init();

    clear();
    send();

    clear();
    toggle[7] = 1;
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t) {255,255,255});
    }
    Delay_Ms(1);
    send();
    onBoardLightOff();

    printf("looping...\n\r");
    while (1) {
        onBoardLightOn();
        uint16_t adc;
        while (1) {
            uint16_t adc2;
            adc = ADC_read();
            printf("adc: %d\n", adc);
            Delay_Us(1);
            adc2 = ADC_read();
            printf("adc2: %d\n", adc2);
            if (adc == adc2)
                break;
        }
        // linear search
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
                continue;
            }
        }
        Delay_Ms(20);
        onBoardLightOff();
        printf("Count: %lu adc: %d\n", count++, adc);
        printf(
            "Button pressed: %d, deviation: %d\n\r", button, abs(adc - buttons[button]));
        if (button == 7) {
            nextForegroundColor();
            toggle[button] = 1;
        }
        else if (button == 0) {
            nextBackgroundColor();
            toggle[button] = 0;
        }
        else {
            toggle[button] = !toggle[button];
        }
        clear();
        // Set the color of the LED strip to the button pressed
        for (int i = 0; i < NUM_LEDS; i++) {
            set_color(i,
                toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
        }
        send();
    }
}
