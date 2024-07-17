#define WS2812BSIMPLE_IMPLEMENTATION
/// Required for the WS2812B Simple implementation to work
#define FUNCONF_SYSTICK_USE_HCLK 1
// #define FUNCONF_USE_5V_VDD 1
#include "buttons.h"
#include "colors.h"

#include <stdio.h>

#ifdef _WIN32
#define NOMINMAX 1          // Prevent Windows.h from defining min and max macros
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
void SystemInit(void) {
    // Set the console to UTF-8 mode
    SetConsoleOutputCP(65001);
    // Get the current console mode
    DWORD consoleMode;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &consoleMode);
    // Enable virtual terminal processing
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), consoleMode);
}
#define Delay_Ms(milliseconds) Sleep(milliseconds)
#define Delay_Us(microseconds) Sleep((microseconds) / 1000)
#else
#include <unistd.h>
#include <stdlib.h>
#define Delay_Ms(milliseconds) usleep((milliseconds) * 1000)
#define Delay_Us(microseconds) usleep(microseconds)
#endif

#define SystemInit() // Do nothing

// Prototypes
void adc_cal(void);
void adc_init(void);
uint16_t adc_get(void);
void nextForegroundColor(void);
void nextBackgroundColor(void);
void set_color(uint8_t led, color_t color);
void fill_color(color_t color);
void send(void);
void clear(void);

/// @brief Array of colors for the LED strip, for sending to the LED strip
color_t led_array[NUM_LEDS] = {0};
/// @brief Array of toggles for each LED
uint8_t toggle[NUM_LEDS] = {0};
/// @brief Index of the foreground color
uint8_t foregroundColorIndex = 3;
/// @brief Index of the background color
uint8_t backgroundColorIndex = 24;

/// @brief Main function
int main(void) {
    SystemInit();
    clear();
    // Force the button to be foreground color
    toggle[7] = 1;
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(
            i, toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
    }
    send();
    while (1) {
        uint16_t adc = adc_get();
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
    }
}

void adc_cal(void) {
    // Do nothing
}

void adc_init(void) {
    // Do nothing
}

uint16_t adc_get(void) {
    // Get user input of button number (0-63)
    // and return its ADC reading value
    uint8_t button;
    scanf("%hhd", &button);
    printf("\n");
    return buttons[button];
}

/**
 * @brief Set the color of the LED strip
 * @param led Which LED to set the color of
 * @param color The color to set the LED to, in RGB format `color_t`
 */
void set_color(uint8_t led, color_t color) {
    /// @todo Remove the brightness scaling
    led_array[led].r = color.r;
    led_array[led].g = color.g;
    led_array[led].b = color.b;
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
    // WS2812BSimpleSend(GPIOC, 6, (uint8_t *)led_array, NUM_LEDS * 3);
#define BORDER_X 2
    printf("\\y");
    for (uint8_t i = horizontalButtons; i > 0; i--) {
        printf("%d", i-1);
    }
    printf(" \nx");
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
    for (uint8_t y = verticalButtons; y > 0; y--) {
        printf("%d|", y - 1);
        for (uint8_t x = horizontalButtons; x > 0; x--) {
            uint8_t led = (y - 1) * horizontalButtons + (x - 1);
            if (led < NUM_LEDS &&
                (led_array[led].r || led_array[led].g || led_array[led].b)) {
                printf("\e[38;2;%d;%d;%dm\u2588\x1b[0m", led_array[led].r,
                    led_array[led].g, led_array[led].b);
            }
            else {
                printf(" ");
            }
        }
        printf("|");
        if (y == verticalButtons / 2 ) {
            printf("   foreground: \e[38;2;%d;%d;%dm\x1b[0m",
                colors[foregroundColorIndex].r, colors[foregroundColorIndex].g,
                colors[foregroundColorIndex].b);
            printf(", RGB(%d, %d, %d)", colors[foregroundColorIndex].r,
                colors[foregroundColorIndex].g, colors[foregroundColorIndex].b);
        }
        if (y == verticalButtons / 2 + 1) {
            printf("   background: \e[38;2;%d;%d;%dm\x1b[0m",
                colors[backgroundColorIndex].r, colors[backgroundColorIndex].g,
                colors[backgroundColorIndex].b);
            printf(", RGB(%d, %d, %d)", colors[backgroundColorIndex].r,
                colors[backgroundColorIndex].g, colors[backgroundColorIndex].b);
        }
        if (y == verticalButtons / 2 + 2) {
            printf("   button calculation: y*%d+x", horizontalButtons);
        }
        printf("\n");
    }
    printf(" ");
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
}
