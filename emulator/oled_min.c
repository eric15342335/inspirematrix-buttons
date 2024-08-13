#include "oled_min.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t line, column, scroll;

uint8_t BUFFER[BUFFER_SIZE];

void OLED_init(void) {
    _OLED_refresh_display();
    return;
}

void OLED_data_start(void) { _OLED_doNothing(); }

void _OLED_doNothing(void) {
    // Do nothing
}

void OLED_setpos(uint8_t x, uint8_t y) {
    column = x;
    line = y;
    scroll = 0;
}

void OLED_fill(uint8_t p) {
    for (uint8_t y = 0; y < SCREEN_Y / AXIS_Y_STORAGE; y++) {
        OLED_data_start();
        for (uint8_t x = 0; x < SCREEN_X; x++) {
            BUFFER[y * SCREEN_X + x] = p;
        }
    }
}

void OLED_clear(void) {
    // Actually OLED_fill(0x00) works the same
    for (int i = 0; i < BUFFER_SIZE; i++) {
        BUFFER[i] = 0;
    }
    column = 0;
    line = 0;
    scroll = 0;
}

// OLED plot a single character
void OLED_plotChar(char c) {
    uint16_t ptr = c - 32; // character pointer
    ptr += ptr << 2;       // -> ptr = (ch - 32) * 5;
    OLED_data_start();
    for (uint8_t i = 5; i; i--) {
        I2C_write(OLED_FONT[ptr++]);
    }
    I2C_write(0x00); // write space between characters
}

// OLED write a character or handle control characters
void OLED_write(char c) {
    c = c & 0x7F; // ignore top bit
    // normal character
    if (c >= 32) {
        OLED_plotChar(c);
        if (++column > SCREEN_X - 6) {
            column = 0;
            if (line == 7)
                OLED_scrollDisplay();
            else
                line++;
            OLED_setline((line + scroll) & 0x07);
        }
    }
    // new line
    else if (c == '\n') {
        column = 0;
        if (line == 7)
            OLED_scrollDisplay();
        else
            line++;
        OLED_setline((line + scroll) & 0x07);
    }
    // carriage return
    else if (c == '\r') {
        column = 0;
        OLED_setline((line + scroll) & 0x07);
    }
}

// OLED print string
void OLED_print(char * str) {
    while (*str)
        OLED_write(*str++);
}

// OLED print string with newline
void OLED_println(char * str) {
    OLED_print(str);
    OLED_write('\n');
}

// Convert byte nibble into hex character and print it
void OLED_printN(uint8_t nibble) {
  OLED_write((nibble <= 9) ? ('0' + nibble) : ('A' - 10 + nibble));
}

// Convert byte into hex characters and print it
void OLED_printB(uint8_t value) {
  OLED_printN(value >> 4);
  OLED_printN(value & 0x0f);
}

// Convert word into hex characters and print it
void OLED_printW(uint16_t value) {
  OLED_printB(value >> 8);
  OLED_printB(value);
}

void OLED_printD(uint32_t value) {
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%u", value);
    OLED_print(buffer);
}

void OLED_setline(uint8_t new_line) {
    OLED_data_start();
    line = new_line;
    column = 0;
}

void OLED_scrollDisplay(void) {
    const int numVertical = SCREEN_Y / AXIS_Y_STORAGE;
    for (int xCoord = 0; xCoord < SCREEN_X; xCoord++) {
        // Copy the next line to the current line
        for (int yCoord = 0; yCoord < numVertical - 1; yCoord++) {
            int byteIndex = yCoord * SCREEN_X + xCoord;
            int nextByteIndex = (yCoord + 1) * SCREEN_X + xCoord;
            BUFFER[byteIndex] = BUFFER[nextByteIndex];
        }
        // Clear the last line
        int byteIndex = (numVertical - 1) * SCREEN_X + xCoord;
        BUFFER[byteIndex] = 0;
    }
    scroll = (scroll + 1) & 0x07;
}

void _OLED_setBuffer(uint8_t data) {
    // Set the buffer at the current line and column
    int byteIndex = line * SCREEN_X + column;
    BUFFER[byteIndex] = data;
    column++;
    if (column >= SCREEN_X) {
        column = 0;
        if (line > 7)
            OLED_scrollDisplay();
        else
            line++;
        OLED_setline((line + scroll) & 0x07);
    }
}

void _OLED_refresh_display() {
    // Clear the screen
    printf("\033[2J\033[H");
#define BORDER_X 2
    // upper border
    for (int x = 0; x < SCREEN_X + BORDER_X; x++) {
        printf("-");
    }
    printf("\n");
    for (int y = 0; y < SCREEN_Y; y++) {
        printf("|");
        for (int x = 0; x < SCREEN_X; x++) {
            int byteIndex = (y / 8) * SCREEN_X + x;
            int bitIndex = y % 8;
            int b = BUFFER[byteIndex];
            printf((b >> bitIndex) & 1 ? "#" : " ");
        }
        printf("|");

        printf("\n");
    }
    // lower border
    for (int x = 0; x < SCREEN_X + BORDER_X; x++) {
        printf("-");
    }
    printf("\n");
}
