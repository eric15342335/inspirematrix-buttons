#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"

typedef struct snakePartDir{
    char part;
    int8_t direction;
} snakePartDir;

color_t appleColor = {255, 0, 0};   // red
color_t snakeHeadColor = {0, 255, 255}; // Green
color_t snakeBodyColor = {51, 255, 51}; // Green
color_t snakeTailColor = {255, 255, 51}; // Yellow

snakePartDir gameboard[64]; // 8x8 gameboard
int8_t snakeHead = 45;
int8_t snakeTail = 47;

void game_init(){
    char * ptr;
    for (ptr = (char*)gameboard; ptr < (char*)(gameboard + 64); ptr += sizeof(snakePartDir)) {
        *(snakePartDir*)ptr = (snakePartDir){'0',0};
    }
    gameboard[47] = (snakePartDir){'t', -1};
    gameboard[46] = (snakePartDir){'b', -1};
    gameboard[45] = (snakePartDir){'h', 0};
    gameboard[42] = (snakePartDir){'a', 0};
}
void display(){
    for (int i = 0; i < 64; i++) {
        switch (gameboard[i].part) {
            case 't':
                set_color(i,snakeTailColor);
                break;
            case 'b':
                set_color(i,snakeBodyColor);
                break;
            case 'h':
                set_color(i,snakeHeadColor);
                break;
            case 'a':
                set_color(i,appleColor);
                break;
            default:
                set_color(i,(color_t) {0, 0, 0});
                break;
        }
        WS2812BSimpleSend(GPIOC, 2, (uint8_t *)led_array, NUM_LEDS * 3);
    }
}

int8_t direction(const int8_t currentDirection) {
    if (JOY_up_pressed() && gameboard[snakeHead + 8].part != 'b') {
        return 8; // go up
    }
    if (JOY_down_pressed() && gameboard[snakeHead - 8].part != 'b') {
        return -8; // go down
    }
    if (JOY_left_pressed() && gameboard[snakeHead + 1].part != 'b') {
        return 1; // go left
    }
    if (JOY_right_pressed() && gameboard[snakeHead - 1].part != 'b') {
        return -1; // go right
    }
    // check if the snake is running to itself
    // if not then set the direction
    return currentDirection;
}

bool checkApple(int8_t currentDirection){
    if (gameboard[snakeHead + currentDirection].part == 'a'){
        return true;
    }
    return false;
}

void generate_apple(void) {
    uint8_t applePos;
    do {
        applePos = JOY_random() % 64;
    } while (gameboard[applePos].part != '0');
    gameboard[applePos] = (snakePartDir){'a',0};
    // random a location to put down an apple
    // random till that is a empty space
}

void moveSnake(const int8_t currentDirection, const bool apple){
    gameboard[snakeHead].part = 'b';
    gameboard[snakeHead].direction = currentDirection;
    snakeHead += currentDirection;
    gameboard[snakeHead].part = 'h';
    if (apple) {
        return;
    }
    // if the snake eats the apple, the tail will not move
    // move the tail when the snake does not eat the apple
    gameboard[snakeTail].part = '0';
    snakeTail += gameboard[snakeTail].direction;
    gameboard[snakeTail].part = 't';
}

bool collision(int8_t currentDirection){
    int8_t nextSnakeHead = snakeHead + currentDirection;
    if (gameboard[nextSnakeHead].part == 'b' || gameboard[nextSnakeHead].part == 't'){
        return true;
    }
    if (nextSnakeHead % 8 == 0 && currentDirection == 1){
        return true;
    }
    if (nextSnakeHead % 8 == 7 && currentDirection == -1){
        return true;
    }
    if (nextSnakeHead < 0 || nextSnakeHead > 63){
        return true;
    }
    return false;
}

int main(void) {
    SystemInit();
    ADC_init();
    game_init();
    display();
    uint16_t seed = 0;
    bool apple;
    while (!JOY_pad_pressed()) {
        seed++;
        if (seed > 0xFFF0) {
            seed = 0;
        } // boundary check of uint16_t rnval
        // generate a seed according to the time between boot and button pressed
        Delay_Ms(1);
        // wait for the button to be pressed
    }
    JOY_setseed(seed);
    int8_t currentDirection = 1;
    while (1){
        for (uint8_t i = 0; i < 7; i++) {
            currentDirection = direction(currentDirection);
            Delay_Ms(100);
        }
        if (collision(currentDirection)){
            break;
        }
        apple = checkApple(currentDirection);
        moveSnake(currentDirection,apple);
        if (apple){
            generate_apple();
        }
        display();
    }
}
