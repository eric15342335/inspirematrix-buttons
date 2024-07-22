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
// #define FUNCONF_USE_5V_VDD 1
// My custom define, see onBoardLight*() functions;
#define CH32V003F4P6_ENABLE_ONBOARD_LIGHT
#include "buttons.h"
#include "ch32v003fun.h"
#include "colors.h"
#include "ws2812b_simple.h"

#include <stdio.h>

// Prototypes
#define abs(x) ((x) < 0 ? -(x) : (x))
void nextForegroundColor(void);
void nextBackgroundColor(void);
void set_color(uint8_t led, color_t color);
void fill_color(color_t color);
void send(void);
void clear(void);
void onBoardLightInit(void);
void onBoardLightOn(void);
void onBoardLightOff(void);
int get_brightness_offset(void);
uint16_t get_num_toggle(void);

/// @brief Index of the foreground color
uint8_t foregroundColorIndex = 8;
/// @brief Index of the background color
uint8_t backgroundColorIndex = 7;

/// @brief Array of toggles for each LED
uint8_t toggle[NUM_LEDS] = {0};

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
    send();

    clear();
    // Force the button to be foreground color
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
        // Delay_Ms(1);
        uint16_t adc;
        while (1) {
            uint16_t adc2;
            adc = adc_get();
            printf("adc: %d\n", adc);
            Delay_Us(1);
            adc2 = adc_get_pad();
            printf("adc2: %d\n", adc2);
            // Check if the ADC value is the same
            //if (adc == adc2)
            //    break;
        }
        // Apply offset to the ADC value
        adc += get_brightness_offset();
        // printf("Count: %lu adc: %d, new adc:%d\n", count++, adc,
        // adc-get_num_toggle(adc)); apply some hysteresis to the ADC value adc -=
        // get_num_toggle(adc); Find out which button is pressed via the ADC value Perform
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
                // printf("No button pressed\n\r");
                continue;
            }
        }
        Delay_Ms(20);
        onBoardLightOff();
        // Print the closest button pressed
        printf("Count: %lu adc: %d\n", count++, adc);
        printf(
            "Button pressed: %d, deviation: %d\n\r", button, abs(adc - buttons[button]));
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
            set_color(i,
                toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
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

/**
 * @brief Set the color of the LED strip
 * @param led Which LED to set the color of
 * @param color The color to set the LED to, in RGB format `color_t`
 */
void set_color(uint8_t led, color_t color) {
    /// @todo Remove the brightness scaling
    led_array[led].r = color.r / 4;
    led_array[led].g = color.g / 4;
    led_array[led].b = color.b / 4;
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
    Delay_Us(1);
    WS2812BSimpleSend(GPIOC, 6, (uint8_t *)led_array, NUM_LEDS * 3);
}

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

/// @brief Clear the LED strip to No Color
void clear(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, (color_t){0, 0, 0});
    }
}

/// @brief Biggest value in provided color argument
uint8_t max_color_value(color_t color) {
    uint8_t max = 0;
    if (color.r > max) {
        max = color.r;
    }
    if (color.g > max) {
        max = color.g;
    }
    if (color.b > max) {
        max = color.b;
    }
    return max;
}

/// @brief Get the brightness offset for the LED strip
int get_brightness_offset(void) {
    // Get the brightness offset from the ADC value

    return get_num_toggle() / 32 * max_color_value(colors[foregroundColorIndex]) / 0xFF +
           (NUM_LEDS - get_num_toggle()) / 32 *
               max_color_value(colors[backgroundColorIndex]) / 0xFF;
}

/** @brief Get the number of toggles for the LED strip
 * @return The number of toggled on LED
 */
uint16_t get_num_toggle(void) {
    // calculate num of '1' in toggle[]
    uint16_t count = 0;
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (toggle[i] == 1)
            count++;
    }
    return count;
    // return 0; // Disabled for now
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
