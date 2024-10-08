#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"

#include <stdio.h>

#define LED_PINS GPIOA, 2

color_t gridColor = {.r = 0, .g = 0, .b = 150};   // Blue
color_t playerColor = {.r = 0, .g = 150, .b = 0}; // Green
color_t botColor = {.r = 150, .g = 0, .b = 0};

char tictactoe_gameboard[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; // 3x3 gameboard
const int8_t * topleft = (const int8_t[]){0, 1, 8, 9};
const int8_t * topmiddle = (const int8_t[]){3, 4, 11, 12};
const int8_t * topright = (const int8_t[]){6, 7, 14, 15};
const int8_t * middleleft = (const int8_t[]){24, 25, 32, 33};
const int8_t * middlemiddle = (const int8_t[]){27, 28, 35, 36};
const int8_t * middleright = (const int8_t[]){30, 31, 38, 39};
const int8_t * bottomleft = (const int8_t[]){48, 49, 56, 57};
const int8_t * bottommiddle = (const int8_t[]){51, 52, 59, 60};
const int8_t * bottomright = (const int8_t[]){54, 55, 62, 63};
const int8_t initcoord[] = {0, 3, 6, 24, 27, 30, 48, 51, 54};

void fillgrid() {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i % 8 == 2 || i % 8 == 5) {
            set_color(i, gridColor);
            continue;
        }
        else if (i >= 16 && i <= 23) {
            set_color(i, gridColor);
            continue;
        }
        else if (i >= 40 && i <= 47) {
            set_color(i, gridColor);
            continue;
        }
    }
}

void fillcirclecross(uint8_t coord, char circlecross) {
    if (circlecross == 'X') {
        set_color(coord, playerColor);
        set_color(coord + 1, playerColor);
        set_color(coord + 8, playerColor);
        set_color(coord + 9, playerColor);
        return;
    }
    else if (circlecross == 'O') {
        set_color(coord, botColor);
        set_color(coord + 1, botColor);
        set_color(coord + 8, botColor);
        set_color(coord + 9, botColor);
        return;
    }
}

void fillallcirclecross() {
    for (int i = 0; i < 9; i++) {
        fillcirclecross(initcoord[i], tictactoe_gameboard[i]);
    }
}

char checkwinside() {
    char * charptr;
    for (charptr = tictactoe_gameboard; charptr < tictactoe_gameboard + 9; charptr += 3) {
        if (*charptr == *(charptr + 1) && *charptr == *(charptr + 2) && *charptr != ' ') {
            return *charptr;
        }
    }
    // horizontal
    for (charptr = tictactoe_gameboard; charptr < tictactoe_gameboard + 3; charptr++) {
        if (*charptr == *(charptr + 3) && *charptr == *(charptr + 6) && *charptr != ' ') {
            return *charptr;
        }
    }
    // vertical
    if (tictactoe_gameboard[0] == tictactoe_gameboard[4] && tictactoe_gameboard[4] == tictactoe_gameboard[8] &&
        tictactoe_gameboard[0] != ' ') {
        return tictactoe_gameboard[0];
    }
    if (tictactoe_gameboard[2] == tictactoe_gameboard[4] && tictactoe_gameboard[4] == tictactoe_gameboard[6] &&
        tictactoe_gameboard[2] != ' ') {
        return tictactoe_gameboard[2];
    }
    // 2 diagonals
    for (charptr = tictactoe_gameboard; charptr < tictactoe_gameboard + 9; charptr++) {
        if (*charptr == ' ')
            return 0;
    }
    return 'd'; // check draw or not
}

void selectposition() {
    uint8_t i;
    uint16_t seed = 0;
    const int8_t * loopptr;
    int8_t position = -1;
    while (1) {
        while (1) {
            position = matrix_pressed(ADC_read);
            seed++;
            if (position != -1) {
                break;
            }
            if (seed >= 65530) {
                seed = 0;
            }
            Delay_Ms(10);
        }
        JOY_setseed(seed);
        for (loopptr = topleft; loopptr < topleft + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[0] == ' ') {
                tictactoe_gameboard[0] = 'X';
                return;
            }
        }
        for (loopptr = topmiddle; loopptr < topmiddle + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[1] == ' ') {
                tictactoe_gameboard[1] = 'X';
                return;
            }
        }
        for (loopptr = topright; loopptr < topright + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[2] == ' ') {
                tictactoe_gameboard[2] = 'X';
                return;
            }
        }
        for (loopptr = middleleft; loopptr < middleleft + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[3] == ' ') {
                tictactoe_gameboard[3] = 'X';
                return;
            }
        }
        for (loopptr = middlemiddle; loopptr < middlemiddle + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[4] == ' ') {
                tictactoe_gameboard[4] = 'X';
                return;
            }
        }
        for (loopptr = middleright; loopptr < middleright + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[5] == ' ') {
                tictactoe_gameboard[5] = 'X';
                return;
            }
        }
        for (loopptr = bottomleft; loopptr < bottomleft + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[6] == ' ') {
                tictactoe_gameboard[6] = 'X';
                return;
            }
        }
        for (loopptr = bottommiddle; loopptr < bottommiddle + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[7] == ' ') {
                tictactoe_gameboard[7] = 'X';
                return;
            }
        }
        for (loopptr = bottomright; loopptr < bottomright + 4; loopptr++) {
            i = *loopptr;
            if (position == i && tictactoe_gameboard[8] == ' ') {
                tictactoe_gameboard[8] = 'X';
                return;
            }
        }
    }
}

void play() {
    while (1) {
        uint8_t randpos = JOY_random() % 9;
        if (tictactoe_gameboard[randpos] == ' ') {
            tictactoe_gameboard[randpos] = 'O';
            Delay_Ms(300);
            break;
        }
    }
}

int main(void) {
    char winner;
    SystemInit();
    ADC_init();
    fillgrid();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    while (1) {
        selectposition();
        fillallcirclecross();
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        winner = checkwinside();
        Delay_Ms(200);
        if (winner != 0)
            break;
        play();
        fillallcirclecross();
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        winner = checkwinside();
        Delay_Ms(200);
        if (winner != 0)
            break;
    }
}
