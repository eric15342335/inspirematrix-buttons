#include <stdio.h>
#include <stdlib.h>
#include "colors.h"
#include "buttons.h"

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
