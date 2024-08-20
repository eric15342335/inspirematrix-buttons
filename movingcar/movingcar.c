#define CH32V003_I2C_IMPLEMENTATION
#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "ch32v003_GPIO_branchless.h"

// Respective pins to control the motors
#define L0 PA1
#define L1 PC5
#define R0 PC7
#define R1 PC6

#define LED_PINS GPIOA, 2

// Save which buttons are toggledy
uint8_t toggle_state[NUM_LEDS] = {0};

uint8_t is_array_all_one(uint8_t *array, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        if (array[i] != 1) {
            return 0;
        }
    }
    return 1;
}

void display_simple_animation(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        clear();
        set_color(i, colors[i]);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(10);
    }
}

int main(void) {
    // Initializations
    SystemInit();
    ADC_init();
    printf("Hello, world!\n");
    funGpioInitAll();
    funPinMode(L0, FUN_OUTPUT);
    funPinMode(L1, FUN_OUTPUT);
    funPinMode(R0, FUN_OUTPUT);
    funPinMode(R1, FUN_OUTPUT);

    display_simple_animation();

    // Clean up
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);


    while (1) {
        if (JOY_Y_pressed()) {
            break;
        }

        int8_t button = matrix_pressed_two();
        if (button != -1) {
            printf("Button: %d\n", button);
            Delay_Ms(150);
            toggle_state[button] = !toggle_state[button];
        }

        // Display toggled LEDs
        for (int i = 0; i < NUM_LEDS; i++) {
            color_t color_tobe_displayed = {255, 255, 0};
            if (i % 8 <= 3) {
                color_tobe_displayed = (color_t){0, 255, 255};
            }
            set_color(i, toggle_state[i] ? color_tobe_displayed : (color_t){0, 0, 0});
        }
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        printf("Loop\n");
    }

    // Start executing the car moving instructions
    // and fill LEDs with color.

    #define RED_COLOR_BUTTON 3
    #define GREEN_COLOR_BUTTON 2
    #define BLUE_COLOR_BUTTON 1
    #define X_COLOR_BUTTON 0

    uint8_t execution_step = 0;
    while (execution_step < verticalButtons) {
        // Debug infos
        printf("Step: %d\n", execution_step);
        printf("Toggle state: ");
        for (int i = 0; i < NUM_LEDS; i++) {
            printf("%d ", toggle_state[i]);
        }
        printf("\n");
        // Stop Car.
        funDigitalWrite(L0, 0);
        funDigitalWrite(L1, 0);
        funDigitalWrite(R0, 0);
        funDigitalWrite(R1, 0);

        const uint8_t starting_index = horizontalButtons*(verticalButtons-execution_step) - horizontalButtons;
        // verify if RESET command is detected.
        if (is_array_all_one(&toggle_state[starting_index], horizontalButtons)) {
            execution_step = 0;
            continue;
        }

        // Fill the LEDs with the color of the current step
        color_t current_step_color = {1,1,1};
        
        current_step_color.r = toggle_state[starting_index + RED_COLOR_BUTTON] ? 255 : 0;
        current_step_color.g = toggle_state[starting_index + GREEN_COLOR_BUTTON] ? 255 : 0;
        current_step_color.b = toggle_state[starting_index + BLUE_COLOR_BUTTON] ? 255 : 0;

        fill_color(current_step_color);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

        // Move the car according to the current step
        #define L0_ON_INDEX 7
        #define L1_ON_INDEX 6
        #define R0_ON_INDEX 5
        #define R1_ON_INDEX 4

        funDigitalWrite(L0, toggle_state[starting_index + L0_ON_INDEX]);
        funDigitalWrite(L1, toggle_state[starting_index + L1_ON_INDEX]);
        funDigitalWrite(R0, toggle_state[starting_index + R0_ON_INDEX]);
        funDigitalWrite(R1, toggle_state[starting_index + R1_ON_INDEX]);

        // Proceed to next step
        execution_step++;

        // Delays
        Delay_Ms(1000);
    }
    NVIC_SystemReset();
}
