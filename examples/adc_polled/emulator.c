
#define WS2812BSIMPLE_IMPLEMENTATION
/// Required for the WS2812B Simple implementation to work
#define FUNCONF_SYSTICK_USE_HCLK 1
// #define FUNCONF_USE_5V_VDD 1

#include <stdint.h>
#include <stdio.h>

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
#define SystemInit() // Do nothing
#endif

#define horizontalButtons 8
#define verticalButtons 8
#define NUM_LEDS horizontalButtons * verticalButtons
/// @brief Array of buttons corresponding to the ADC values, for linear searching
int buttons[NUM_LEDS] = {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8,
    BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12, BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16, BUTTON_17, BUTTON_18,
    BUTTON_19, BUTTON_20, BUTTON_21, BUTTON_22, BUTTON_23, BUTTON_24, BUTTON_25, BUTTON_26, BUTTON_27, BUTTON_28,
    BUTTON_29, BUTTON_30, BUTTON_31, BUTTON_32, BUTTON_33, BUTTON_34, BUTTON_35, BUTTON_36, BUTTON_37, BUTTON_38,
    BUTTON_39, BUTTON_40, BUTTON_41, BUTTON_42, BUTTON_43, BUTTON_44, BUTTON_45, BUTTON_46, BUTTON_47, BUTTON_48,
    BUTTON_49, BUTTON_50, BUTTON_51, BUTTON_52, BUTTON_53, BUTTON_54, BUTTON_55, BUTTON_56, BUTTON_57, BUTTON_58,
    BUTTON_59, BUTTON_60, BUTTON_61, BUTTON_62, BUTTON_63};

void adc_cal(void) {
    // Do nothing
}

void adc_init(void) {
    // Do nothing
}

uint16_t adc_get(void) {
    // Get user input of button number (0-63)
    // and return its ADC reading value
    int button;
    scanf("%d", &button);
    return buttons[button];
}

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
    led_array[led].r = color.r;
    led_array[led].g = color.g;
    led_array[led].b = color.b;
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
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
    for (uint8_t y = verticalButtons; y > 0; y--) {
        printf("|");
        for (uint8_t x = horizontalButtons; x > 0; x--) {
            uint8_t led = (y - 1) * horizontalButtons + (x - 1);
            if (led < NUM_LEDS && (led_array[led].r || led_array[led].g || led_array[led].b)) {
                printf("\033[38;2;%d;%d;%dm\u2588\033[0m", led_array[led].r, led_array[led].g, led_array[led].b);
            }
            else {
                printf(" ");
            }
        }
        printf("|\n");
    }
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
}

/// @brief
static const color_t colors[] = {
    // Red (8 shades)
    {255, 0, 0}, {223, 0, 0}, {191, 0, 0}, {159, 0, 0}, {127, 0, 0}, {95, 0, 0}, {63, 0, 0}, {31, 0, 0},

    // Green (8 shades)
    {0, 255, 0}, {0, 223, 0}, {0, 191, 0}, {0, 159, 0}, {0, 127, 0}, {0, 95, 0}, {0, 63, 0}, {0, 31, 0},

    // Blue (8 shades)
    {0, 0, 255}, {0, 0, 223}, {0, 0, 191}, {0, 0, 159}, {0, 0, 127}, {0, 0, 95}, {0, 0, 63}, {0, 0, 31},

    // Yellow (8 shades)
    {255, 255, 0}, {223, 223, 0}, {191, 191, 0}, {159, 159, 0}, {127, 127, 0}, {95, 95, 0}, {63, 63, 0}, {31, 31, 0},

    // Cyan (8 shades)
    {0, 255, 255}, {0, 223, 223}, {0, 191, 191}, {0, 159, 159}, {0, 127, 127}, {0, 95, 95}, {0, 63, 63}, {0, 31, 31},

    // Magenta (8 shades)
    {255, 0, 255}, {223, 0, 223}, {191, 0, 191}, {159, 0, 159}, {127, 0, 127}, {95, 0, 95}, {63, 0, 63}, {31, 0, 31},

    // White (8 shades)
    {255, 255, 255}, {223, 223, 223}, {191, 191, 191}, {159, 159, 159}, {127, 127, 127}, {95, 95, 95}, {63, 63, 63},
    {31, 31, 31},

    // Orange (8 shades)
    {255, 127, 0}, {223, 111, 0}, {191, 95, 0}, {159, 79, 0}, {127, 63, 0}, {95, 47, 0}, {63, 31, 0}, {31, 15, 0}};

/// @brief Index of the foreground color
static uint8_t foregroundColorIndex = 3;
/// @brief Index of the background color
static uint8_t backgroundColorIndex = 24;
/// @brief Number of colors in the color array
static const uint16_t num_colors = sizeof(colors) / sizeof(colors[0]);

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

/// @brief Array of toggles for each LED
uint8_t toggle[NUM_LEDS] = {0};

/// @brief Main function
int main(void) {
    // Number of times the loop has run
    uint32_t count = 0;
    SystemInit();
    clear();
    // Force the button to be foreground color
    toggle[7] = 1;
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
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
            set_color(i, toggle[i] ? colors[foregroundColorIndex] : colors[backgroundColorIndex]);
        }
        send();
    }
}
