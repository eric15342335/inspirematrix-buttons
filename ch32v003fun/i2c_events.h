#pragma once
#include "ch32v003fun.h"
#include <stdio.h>

uint8_t check_i2c_event(uint32_t event) {
    uint16_t STAR1, STAR2 __attribute__((unused));
    STAR1 = I2C1->STAR1;
    STAR2 = I2C1->STAR2;
    uint32_t status = (STAR1 | (STAR2 << 16)) & 0x00FFFFFF;
    // Check if the event matches the status
    return (status & event) == event;
}

const uint32_t timeout_default = 5000000;
void wait_for_event(uint32_t event) {
    uint32_t timeout = timeout_default;
    while (!check_i2c_event(event)) {
        if (--timeout == 0) {
            printf("Resetted due to 0x%08lX timeout!\n", event);
            NVIC_SystemReset();
        }
    }
}
