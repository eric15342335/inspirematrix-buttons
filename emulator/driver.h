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
#define JOY_init() SystemInit()
#define DLY_ms(milliseconds) Sleep(milliseconds)

static inline bool is_key_pressed(char capitalkey) {
    SHORT result =
        GetAsyncKeyState((int)capitalkey); // windows.h requires capital letters
    return (result & 0x8000) != 0;
}
#define JOY_act_pressed() is_key_pressed('P')
#define JOY_act_released() !is_key_pressed('P')
#define JOY_up_pressed() is_key_pressed('I')
#define JOY_down_pressed() is_key_pressed('K')
#define JOY_left_pressed() is_key_pressed('J')
#define JOY_right_pressed() is_key_pressed('L')
#define JOY_X_pressed() is_key_pressed('U')
#define JOY_Y_pressed() is_key_pressed('O')

uint16_t ADC_read(void) {
    // If pressed A, B, C, D, wait for second input 0-9 and A-F
    // return the value of the button
    // e.g. AF indicates A=0 + F=15 = 15
    // Use non blocking is_key_pressed
    for (char i = 'A'; i <= 'D'; i++) {
        if (is_key_pressed(i)) {
            while (is_key_pressed(i));
            printf("Pressed %c, Press 0-9 or A-F\n", i);
            while (true) {
                for (char j = '0'; j <= '9'; j++) {
                    if (is_key_pressed(j)) {
                        printf("Pressed %c\n", j);
                        return buttons[(i - 'A') * 16 + (j - '0')];
                    }
                }
                for (char j = 'A'; j <= 'F'; j++) {
                    if (is_key_pressed(j)) {
                        printf("Pressed %c\n", j);
                        return buttons[(i - 'A') * 16 + (j - 'A' + 10)];
                    }
                }
            }
        }
    }
    return 0;
}


#elif defined(__APPLE__)
#include "system_mac.h"
#include <unistd.h>

#define SystemInit() pthread_init()
#define DLY_ms(milliseconds) usleep(milliseconds * 1000)
#define Delay_Ms(milliseconds) usleep((milliseconds) * 1000)
#define Delay_Us(microseconds) usleep(microseconds)
#define JOY_act_pressed() is_key_pressed(P_Key)
#define JOY_act_released() !is_key_pressed(P_Key)
#define JOY_up_pressed() is_key_pressed(I_Key)
#define JOY_down_pressed() is_key_pressed(K_Key)
#define JOY_left_pressed() is_key_pressed(J_Key)
#define JOY_right_pressed() is_key_pressed(L_Key)
#define JOY_X_pressed() is_key_pressed(U_Key)
#define JOY_Y_pressed() is_key_pressed(O_Key)

uint16_t ADC_read(void) {
    // If pressed A, B, C, D, wait for second input 0-9 and A-F
    // return the value of the button
    // e.g. AF indicates A=0 + F=15 = 15
    // Use non blocking is_key_pressed
    for (int i = 0; i < 4; i++) {
        if (is_key_pressed(ABCD[i])) {
            while (is_key_pressed(ABCD[i]));
            printf("Pressed %c, Press 0-9 or A-F\n", ABCD[i]);
            while (true) {
                for (int j = 0; j < 16; j++) {
                    if (is_key_pressed(_0123456789ABCDEF[j])) {
                        printf("Pressed %d\n", j);
                        return buttons[i * 16 + j];
                    }
                }
            }
        }
    }
    return 0;
}

#endif

#define JOY_pad_pressed()                                                              \
    (JOY_up_pressed() || JOY_down_pressed() || JOY_left_pressed() || JOY_right_pressed())
#define JOY_pad_released()                                                             \
    (!JOY_up_pressed() && !JOY_down_pressed() && !JOY_left_pressed() && !JOY_right_pressed())
#define JOY_all_released() (JOY_act_released() && !JOY_pad_released())

void ADC_init(void) {
    // Do nothing
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

uint16_t rnval;
uint16_t JOY_random(void) {
  rnval = (rnval >> 0x01) ^ (-(rnval & 0x01) & 0xB400);
  return rnval;
}

void JOY_setseed_default(void){
  rnval = 0x1234;
}

void JOY_setseed(uint16_t seed){
  rnval = seed;
}