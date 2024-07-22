#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "ws2812b_simple.h"


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

#ifdef _WIN32
#include <windows.h>
#define JOY_init() SystemInit()
#define DLY_ms(milliseconds) Sleep(milliseconds)
#define JOY_sound(freq, dur) Beep(freq, dur)

static inline bool is_key_pressed(char capitalkey) {
    SHORT result =
        GetAsyncKeyState((int)capitalkey); // windows.h requires capital letters
    return (result & 0x8000) != 0;
}
#define JOY_act_pressed() is_key_pressed('F')
#define JOY_act_released() !is_key_pressed('F')
#define JOY_up_pressed() is_key_pressed('W')
#define JOY_down_pressed() is_key_pressed('S')
#define JOY_left_pressed() is_key_pressed('A')
#define JOY_right_pressed() is_key_pressed('D')

#else
#include "system_mac.h"
#include <unistd.h>

#define SystemInit() pthread_init()
#define DLY_ms(milliseconds) usleep(milliseconds * 1000)

#define JOY_act_pressed() is_key_pressed(F_Key)
#define JOY_act_released() !is_key_pressed(F_Key)
#define JOY_up_pressed() is_key_pressed(W_Key)
#define JOY_down_pressed() is_key_pressed(S_Key)
#define JOY_left_pressed() is_key_pressed(A_Key)
#define JOY_right_pressed() is_key_pressed(D_Key)

#endif

#define JOY_pad_pressed()                                                              \
    (JOY_up_pressed() || JOY_down_pressed() || JOY_left_pressed() || JOY_right_pressed())
#define JOY_pad_released()                                                             \
    (!JOY_up_pressed() && !JOY_down_pressed() && !JOY_left_pressed() && !JOY_right_pressed())
#define JOY_all_released() (JOY_act_released() && !JOY_pad_released())

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

uint8_t adc_get_pad(void) {
    return 0;
}