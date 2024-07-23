#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"

color_t gridColor = {0, 0, 150}; // Blue
color_t playerColor = {0, 150, 0}; // Green

char gameboard[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; // 3x3 gameboard
const int8_t *topleft = (const int8_t[]){0,1,8,9};
const int8_t *topmiddle = (const int8_t[]){3,4,11,12};
const int8_t *topright = (const int8_t[]){6,7,14,15};
const int8_t *middleleft = (const int8_t[]){24,25,32,33};
const int8_t *middlemiddle = (const int8_t[]){27,28,35,36};
const int8_t *middleright = (const int8_t[]){30,31,38,39};
const int8_t *bottomleft = (const int8_t[]){48,49,56,57};
const int8_t *bottommiddle = (const int8_t[]){51,52,59,60};
const int8_t *bottomright = (const int8_t[]){54,55,62,63};


void fillgrid() {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (i % 8 == 2 || i % 8 == 5){
            set_color(i, gridColor);
            continue;
        }
        else if (i >= 16 && i <= 23){
            set_color(i, gridColor);
            continue;
        }
        else if (i >= 40 && i <= 47){
            set_color(i, gridColor);
            continue;
        }
    }
}

void selectposition(){
    uint8_t i;
    const int8_t * loopptr;
    while (1){
        for (loopptr = topleft; loopptr < topleft + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[0] == ' '){
                gameboard[0] = 'X';
                return;
            }
        }
        for (loopptr = topmiddle; loopptr < topmiddle + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[1] == ' '){
            gameboard[1] = 'X';
            return;
            }
        }
        for (loopptr = topright; loopptr < topright + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[2] == ' '){
            gameboard[2] = 'X';
            return;
            }
        }
        for (loopptr = middleleft; loopptr < middleleft + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[3] == ' '){
            gameboard[3] = 'X';
            return;
            }
        }
        for (loopptr = middlemiddle; loopptr < middlemiddle + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[4] == ' '){
            gameboard[4] = 'X';
            return;
            }
        }
        for (loopptr = middleright; loopptr < middleright + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[5] == ' '){
            gameboard[5] = 'X';
            return;
            }
        }
        for (loopptr = bottomleft; loopptr < bottomleft + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[6] == ' '){
            gameboard[6] = 'X';
            return;
            }
        }
        for (loopptr = bottommiddle; loopptr < bottommiddle + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[7] == ' '){
            gameboard[7] = 'X';
            return;
            }
        }
        for (loopptr = bottomright; loopptr < bottomright + 4; loopptr++){
            i = *loopptr;
            if (matrix_pressed() == i && gameboard[8] == ' '){
            gameboard[8] = 'X';
            return;
            }
        }
    }
}

int main(void) {
    //char winner;
    SystemInit();
    ADC_init();
    fillgrid();
    WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
    while (1) {
        selectposition();
        WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
        //winner = checkwinside();
        //if (winner != 0) break;
        //play();
        //winner = checkwinside();
        //if (winner != 0) break;
        DLY_ms(100);
    }
}
