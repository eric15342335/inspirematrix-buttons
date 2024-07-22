#pragma once
#include "colors.h"
#include "buttons.h"
#include "ch32v003fun.h"

void WS2812BSimpleSend(GPIO_TypeDef *port, int pin, uint8_t *data, int len_in_bytes) {
    (void) port;
    (void) pin;
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
            if (led < len_in_bytes && (data[led] & 0x07)) {
                printf("\e[48;2;%d;%d;%dm\u2588\x1b[0m", (data[led] & 0xE0) >> 5, (data[led] & 0x1C) >> 2, (data[led] & 0x03));
            } else {
                printf(" ");
            }
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
