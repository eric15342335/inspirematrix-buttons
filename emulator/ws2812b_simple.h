#pragma once
#include <stdio.h>
#include "colors.h"
#include "buttons.h"
#include "ch32v003fun.h"

void WS2812BSimpleSend(GPIO_TypeDef *port, int pin, uint8_t *data, int len_in_bytes) {
    (void) port;
    (void) pin;
    if (len_in_bytes != NUM_LEDS * 3) {
        printf("Error: len_in_bytes != NUM_LEDS * 3\n");
        exit(1);
    }
    #define BORDER_X 2
    printf("\\x");
    for (uint8_t i = horizontalButtons; i > 0; i--) {
        printf("%d", i - 1);
    }
    printf(" \ny");
    for (uint8_t i = 0; i < horizontalButtons + BORDER_X; i++) {
        printf("-");
    }
    printf("\n");
    for (uint8_t y = verticalButtons; y > 0; y--) {
        printf("%d|", y - 1);
        for (uint8_t x = horizontalButtons; x > 0; x--) {
            uint8_t led = (y - 1) * horizontalButtons + (x - 1);
            printf("\e[38;2;%d;%d;%dm\u2588\x1b[0m", data[led * 3], data[led * 3 + 1], data[led * 3 + 2]);
            //printf("r:%d g:%d b:%d ", data[led * 3], data[led * 3 + 1], data[led * 3 + 2]);
        }
        printf("|");
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
