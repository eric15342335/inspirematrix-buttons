#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "colors.h"
#include "buttons.h"

#define GPIOC                                   ((GPIO_TypeDef *)0)
#define GPIOD                                   ((GPIO_TypeDef *)0)
#define __IO volatile
typedef struct
{
	__IO uint32_t CFGLR;
	__IO uint32_t CFGHR;
	__IO uint32_t INDR;
	__IO uint32_t OUTDR;
	__IO uint32_t BSHR;
	__IO uint32_t BCR;
	__IO uint32_t LCKR;
} GPIO_TypeDef;

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

void ADC_calibrate(void) {
    // Do nothing
}

void ADC_init(void) {
    // Do nothing
}

uint16_t ADC_read(void) {
    // Get user input of button number (0-63)
    // and return its ADC reading value
    uint8_t button;
    scanf("%hhd", &button);
    printf("\n");
    return buttons[button];
}

int8_t matrix_pressed(void) {
    uint16_t adc_value = ADC_read();
    for (uint8_t i = 0; i < 64; i++) {
        if (buttons[i] == adc_value) {
            return i;
        }
    }
    return -1;
}
