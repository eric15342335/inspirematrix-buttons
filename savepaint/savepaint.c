#define CH32V003_I2C_IMPLEMENTATION
#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include <stdbool.h>
#include "ch32v003_i2c.h"
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"
#include "fonts.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

// initialize file storage structure for 32kb/512pages
// first 8 pages are used for status
void init_storage(void);

// save paint data to eeprom, paint 0 stored in page ?? (out of page 0 to 511)
void save_paint(uint16_t paint_no, color_t *data);

// load paint data from eeprom, paint 0 stored in page ?? (out of page 0 to 511)
void load_paint(uint16_t paint_no, color_t *data, uint8_t is_icon);

// set page status to 0 or 1
void set_page_status(uint16_t page_no, uint8_t status);

// reset to default storage status
void reset_storage(void);

// print storage data to console
void print_status_storage(void);

// check if page[x] is already used
uint8_t is_page_used(uint16_t page_no);

// check if already initialized data, aka init_status_data is set
uint8_t is_storage_initialized(void);

void display_number_centered(uint8_t number);

#define page_size 64
// range of byte that stores status of page[x]
#define init_status_addr_begin 0
#define init_status_addr_end 7
#define init_status_reg_size (init_status_addr_end - init_status_addr_begin + 1)

#define init_status_format " %c "
#define init_status_data (uint8_t*)"IL000001"

#define page_status_addr_begin 8
#define page_status_addr_end 511
#define page_status_reg_size (page_status_addr_end - page_status_addr_begin + 1)

#define page_status_format "%02X "

#define sizeof_paint_data (3 * NUM_LEDS)
#define sizeof_paint_data_aspage (sizeof_paint_data/page_size)
#define matrix_hori 16

#define paint_page_no (8*sizeof_paint_data_aspage)
#define paint_page_no_max (8*sizeof_paint_data_aspage)
#define num_paint_saves (paint_page_no_max / sizeof_paint_data_aspage)

// store icons in EEPROM
#define STORE_ICONS 0
#if STORE_ICONS
#undef paint_page_no
#define paint_page_no (0*sizeof_paint_data_aspage)
#endif

#define app_icon_page_no (0*sizeof_paint_data_aspage)
#define app_icon_page_no_max (8*sizeof_paint_data_aspage)

#define delay 1000

#define LED_PINS GPIOA, 2

const color_t color_savefile_exist = {.r = 0, .g = 0, .b = 100};
const color_t color_savefile_empty = {.r = 0, .g = 100, .b = 0};

color_t foreground = {100, 0, 0};
color_t background = {0, 0, 100};

typedef struct {
    enum { FOREGROUND_LAYER, BACKGROUND_LAYER } layer;
    color_t color;
} canvas_t;

canvas_t canvas[NUM_LEDS] = {0};

/** @brief Numbers are arranged by the order of icons
 * in the EEPROM!!!!! Read app_selection() for more info.
 */
typedef enum _app_selected {
    paint = 0,
    music = 1,
    rec = 2,
    risc_v_code = 3,
    game_tic_tac_toe = 4,
    game_snake = 5,
    robot_car = 6,
} app_selected;

void flushCanvas(void);

void displayColorPalette(void);

void colorPaletteSelection(color_t * selectedColor);

void painting_routine(void);

void setup_unique_pattern(void);

void choose_save_paint_page(void);

void led_display_paint_page_status(void);

void any_paint_exist(uint8_t *paint_exist);

void display_stored_paints(void);

void app_selection(app_selected * app);

void red_screen(void);

void erase_all_paint_saves(void);

// snake game

void drawScore(uint8_t score);

bool collision(int8_t currentDirection);

void snake_game_routine(void);

void moveSnake(int8_t currentDirection, const bool apple);

void generate_apple(void);

bool checkApple(int8_t currentDirection);

int8_t direction(int8_t currentDirection);

void display(void);

void game_init(void);
// end snake game

// tic-tac-toe game

void tic_tac_toe_routine(void);

// end tic-tac-toe game

// moving car

// Respective pins to control the motors
#define L0 PA1
#define L1 PC5
#define R0 PC7
#define R1 PC6

void movingcar_routine(void);

// end moving car

int main(void) {
    SystemInit();
    ADC_init();
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(delay);

    i2c_init();
    printf("I2C Initialized\n");
    init_storage();

    // Hold button Y at startup to reset all paints
    uint16_t delay_countdown = 50;
    while (delay_countdown-- > 0) {
        if (JOY_Y_pressed()) {
            erase_all_paint_saves();
            // Visual indication of paint save reset
            red_screen();
            printf("Paint reset\n");
            printf("DEBUG: %d\n", __LINE__);
            Delay_Ms(1000);
        }
        Delay_Ms(1);
    }

    print_status_storage();

#if STORE_ICONS == 1
    app_selected app = paint;
#else
    app_selected app = paint;
    app_selection(&app);
#endif
    switch (app) {
        case paint:
            display_stored_paints();
            painting_routine();
            choose_save_paint_page();
            break;
        case music:
            red_screen();
            break;
        case rec:
            red_screen();
            break;
        case risc_v_code:
            red_screen();
            break;
        case game_tic_tac_toe:
            tic_tac_toe_routine();
            break;
        case game_snake:
            snake_game_routine();
            break;
        case robot_car:
            movingcar_routine();
            break;
        default:
            red_screen();
            break;
    }

    Delay_Ms(delay);
    while(1) {
        if (JOY_Y_pressed()) {
            NVIC_SystemReset();
        }
        Delay_Ms(200);
    }
}

void erase_all_paint_saves(void) {
    // Set status of paint pages to 0
    for (uint16_t _paint_page_no = paint_page_no; _paint_page_no < paint_page_no_max + paint_page_no; _paint_page_no++) {
        for (uint16_t i = _paint_page_no + page_status_addr_begin; i < _paint_page_no + page_status_addr_begin + sizeof_paint_data_aspage; i++) {
            set_page_status(i, 0);
            Delay_Ms(3);
        }
    }
    printf("All paint saves status erased\n");
    // Erase existing data to 0
    for (uint16_t _paint_page_no = paint_page_no; _paint_page_no < paint_page_no_max + paint_page_no; _paint_page_no+=sizeof(uint8_t)) {
        i2c_result_e err =
            i2c_write_pages(EEPROM_ADDR, _paint_page_no * page_size, I2C_REGADDR_2B, (uint8_t[]){0}, sizeof(uint8_t));
        printf("Erase paint result: %d\n", err);
        Delay_Ms(3);
    }
}

void red_screen(void) {
    fill_color((color_t){.r = 100, .g = 0, .b = 0});
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void app_selection(app_selected * app) {
    clear();
    printf("App selection\n");

    // Display app icon menu just like paint saves
    
    // todo: add missing icon check
    
    int8_t current_display_icon = 0;
    display_number_centered(current_display_icon);
    Delay_Ms(1000);

    typedef enum direction_pressed {
        up_pressed = -1,
        down_pressed = 1,
    } direction_pressed;
    direction_pressed last_direction_pressed = down_pressed;

    uint8_t leave = 0;

    while (1) {
        if (leave) {
            break;
        }

        if (current_display_icon >= paint_page_no_max / sizeof_paint_data_aspage) {
            current_display_icon = 0;
        }
        if (current_display_icon < 0) {
            current_display_icon = paint_page_no_max / sizeof_paint_data_aspage - 1;
        }

        uint16_t _icon_page_no = current_display_icon * sizeof_paint_data_aspage + app_icon_page_no;
        clear();
        if (!is_page_used(_icon_page_no + page_status_addr_begin) ||
            !is_page_used(_icon_page_no + page_status_addr_begin + 1) ||
            !is_page_used(_icon_page_no + page_status_addr_begin + 2)) {
            printf("Icon %d not found\n", _icon_page_no / 3);
            printf("DEBUG: %d\n", __LINE__);
            current_display_icon += last_direction_pressed;
            continue;
        }
        display_number_centered(current_display_icon);
        Delay_Ms(500);
        printf("Displaying icon %d\n", _icon_page_no);
        load_paint(_icon_page_no / sizeof_paint_data_aspage, led_array, 1);
        // setup_unique_pattern();
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

        while (1) {
            if (JOY_left_pressed()) {
                leave = 1;
                break;
            }
            if (JOY_down_pressed()) {
                current_display_icon++;
                printf("Down pressed\n");
                last_direction_pressed = down_pressed;
                break;
            }
            else if (JOY_up_pressed()) {
                current_display_icon--;
                printf("Up pressed\n");
                last_direction_pressed = up_pressed;
                break;
            }
            Delay_Ms(100);
        }
    }
    printf("All stored icons displayed\n");
    clear();
    red_screen();
    Delay_Ms(delay);
    *app = current_display_icon;
}

void any_paint_exist(uint8_t *paint_exist) {
    for (uint16_t _paint_page_no = paint_page_no; _paint_page_no < paint_page_no_max + paint_page_no; _paint_page_no+=sizeof_paint_data_aspage) {
        if (is_page_used(_paint_page_no + page_status_addr_begin) &&
            is_page_used(_paint_page_no + page_status_addr_begin + 1) &&
            is_page_used(_paint_page_no + page_status_addr_begin + 2)) {
            *paint_exist = 1;
            return;
        }
    }
    *paint_exist = 0;
}

void display_number_centered(uint8_t number) {
    clear();
    printf("Displaying number %d\n", number);
    font_draw(font_list[number], color_savefile_empty, horizontalButtons * 1 + verticalButtons / font_width);
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void display_stored_paints(void) {
    // Check if any paint to display
    uint8_t paint_exist = 0;
    any_paint_exist(&paint_exist);
    if (!paint_exist) {
        printf("No paint to display\n");
        fill_color((color_t){.r = 100, .g = 0, .b = 0});
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(1000);
        return;
    }
    
    int8_t current_display_paint = 0;
    display_number_centered(current_display_paint);
    Delay_Ms(1000);

    typedef enum direction_pressed {
        up_pressed = -1,
        down_pressed = 1,
    } direction_pressed;
    direction_pressed last_direction_pressed = down_pressed;

    uint8_t leave = 0;

    while (1) {
        if (leave) {
            break;
        }

        if (current_display_paint >= paint_page_no_max / sizeof_paint_data_aspage) {
            current_display_paint = 0;
        }
        if (current_display_paint < 0) {
            current_display_paint = paint_page_no_max / sizeof_paint_data_aspage - 1;
        }

        uint16_t _paint_page_no = current_display_paint * sizeof_paint_data_aspage + paint_page_no;
        clear();
        if (!is_page_used(_paint_page_no + page_status_addr_begin) ||
            !is_page_used(_paint_page_no + page_status_addr_begin + 1) ||
            !is_page_used(_paint_page_no + page_status_addr_begin + 2)) {
            printf("Paint %d not found\n", _paint_page_no / sizeof_paint_data_aspage);
            printf("DEBUG: %d\n", __LINE__);
            current_display_paint += last_direction_pressed;
            continue;
        }
        display_number_centered(current_display_paint);
        Delay_Ms(500);
        printf("Displaying paint %d\n", _paint_page_no / sizeof_paint_data_aspage);
        load_paint(_paint_page_no / sizeof_paint_data_aspage, led_array, 0);
        // setup_unique_pattern();
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

        while (1) {
            if (JOY_left_pressed()) {
                leave = 1;
                break;
            }
            if (JOY_down_pressed()) {
                current_display_paint++;
                printf("Down pressed\n");
                last_direction_pressed = down_pressed;
                break;
            }
            else if (JOY_up_pressed()) {
                current_display_paint--;
                printf("Up pressed\n");
                last_direction_pressed = up_pressed;
                break;
            }
            Delay_Ms(100);
        }
    }
    printf("All stored paints displayed\n");
    clear();
    fill_color((color_t){.r = 100, .g = 0, .b = 0});
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(delay);
}

void led_display_paint_page_status(void) {
    clear();
    for (uint16_t _paint_page_no = paint_page_no; _paint_page_no < paint_page_no_max + paint_page_no; _paint_page_no+=sizeof_paint_data_aspage) {
        if (is_page_used(_paint_page_no + paint_page_no + page_status_addr_begin) &&
            is_page_used(_paint_page_no + paint_page_no + page_status_addr_begin + 1) &&
            is_page_used(_paint_page_no + paint_page_no + page_status_addr_begin + 2)) {
            set_color((_paint_page_no - paint_page_no) / sizeof_paint_data_aspage, color_savefile_exist);
        } else {
            set_color((_paint_page_no - paint_page_no) / sizeof_paint_data_aspage, color_savefile_empty);
        }
    }
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void choose_save_paint_page(void) {
    led_display_paint_page_status();
    int8_t button = no_button_pressed;
    while (1) {
        button = matrix_pressed_two();
        if (button != no_button_pressed) {
            if (is_page_used(button * sizeof_paint_data_aspage + page_status_addr_begin)) {
                printf("Page %d already used\n", button);
                // Fill the screen with red to indicate error
                fill_color((color_t){.r = 100, .g = 0, .b = 0});
                WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
                Delay_Ms(1000);
                led_display_paint_page_status();
                #if STORE_ICONS == 1
                #else
                continue;
                #endif
            }
            printf("Selected page %d\n", button);
            // Put canvas to led_array
            for (int i = 0; i < NUM_LEDS; i++) {
                set_color(i, canvas[i].color);
            }
            save_paint(button, led_array);
            printf("Paint saved\n");
            Delay_Ms(1000);
            break;
        }
        Delay_Ms(200);
    }
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void setup_unique_pattern(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, colors[i]);
    }
}

void init_storage(void) {
    if (!is_storage_initialized()) {
        reset_storage();
        printf("Storage initialized\n");
    } else {
        printf("Storage already initialized\n");
    }
}

uint8_t is_storage_initialized(void) {
    uint8_t data[init_status_reg_size];
    i2c_read(EEPROM_ADDR, init_status_addr_begin, I2C_REGADDR_2B, data, init_status_reg_size);
    for (uint8_t i = 0; i < init_status_reg_size; i++) {
        if (data[i] != *(init_status_data+i)) {
            return 0;
        }
    }
    return 1;
}

void reset_storage(void) {
    i2c_write(EEPROM_ADDR, init_status_addr_begin, I2C_REGADDR_2B, init_status_data, init_status_reg_size);
    Delay_Ms(3);
    for (uint16_t addr = page_status_addr_begin; addr < page_status_addr_begin + page_status_reg_size; addr++) {
        i2c_write(EEPROM_ADDR, addr, I2C_REGADDR_2B, (uint8_t[]){0}, sizeof(uint8_t));
        Delay_Ms(3);
    }
    printf("Storage reset\n");
}

void print_status_storage(void) {
    printf("Status storage data:\n");
    for (uint16_t addr = init_status_addr_begin; addr < init_status_addr_begin + init_status_reg_size; addr++) {
        uint8_t data = 0;
        i2c_read(EEPROM_ADDR, addr, I2C_REGADDR_2B, &data, sizeof(data));
        printf(init_status_format, data);
    }
    for (uint16_t addr = page_status_addr_begin; addr < page_status_addr_begin + page_status_reg_size; addr++) {
        uint8_t data = 0;
        i2c_read(EEPROM_ADDR, addr, I2C_REGADDR_2B, &data, sizeof(data));
        printf(page_status_format, data);
        if ((addr + 1) % matrix_hori == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void set_page_status(uint16_t page_no, uint8_t status) {
    if (status > 1) {
        printf("Invalid status %d\n", status);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    if (page_no < page_status_addr_begin || page_no > page_status_addr_end) {
        printf("Invalid page number %d\n", page_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    i2c_write(EEPROM_ADDR, page_no, I2C_REGADDR_2B, &status, sizeof(status));
    Delay_Ms(3);
    printf("Page %d status set to %d\n", page_no, status);
}

uint8_t is_page_used(uint16_t page_no) {
    if (page_no < page_status_addr_begin || page_no > page_status_addr_end) {
        printf("Invalid page number %d\n", page_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    uint8_t data = 0;
    i2c_read(EEPROM_ADDR, page_no, I2C_REGADDR_2B, &data, sizeof(data));
    printf("Page %d is %s\n", page_no, data ? "used" : "empty");
    return data;
}

void save_paint(uint16_t paint_no, color_t *data) {
    if (paint_no < 0 || paint_no > page_status_addr_end) {
        printf("Invalid paint number %d\n", paint_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    uint16_t page_no_start = paint_no * sizeof_paint_data_aspage + paint_page_no + page_status_addr_begin;
    for (uint16_t i = page_no_start; i < page_no_start + sizeof_paint_data_aspage; i++) {
        if (is_page_used(i)) {
            printf("Paint %d already used, overwriting\n", paint_no);
            Delay_Ms(500);
        }
        set_page_status(i, 1);
    }
    i2c_result_e err =
        i2c_write_pages(EEPROM_ADDR, page_no_start * page_size, I2C_REGADDR_2B, (uint8_t*)data, sizeof_paint_data);
    printf("Save paint result: %d\n", err);
    Delay_Ms(3);
    printf("Paint %d saved\n", paint_no);
}

void load_paint(uint16_t paint_no, color_t *data, uint8_t is_icon) {
    if (paint_no < 0 || paint_no > page_status_addr_end) {
        printf("Invalid paint number %d\n", paint_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    uint16_t page_no_start = 0;
    if (is_icon) {
        page_no_start = (paint_no + app_icon_page_no) * sizeof_paint_data_aspage + page_status_addr_begin;
    }
    else {
        page_no_start = paint_no * sizeof_paint_data_aspage + paint_page_no + page_status_addr_begin;
    }
    printf("Loading paint_no %d from page %d, is_icon: %d\n", paint_no, page_no_start, is_icon);
    if (!is_page_used(page_no_start)) {
        printf("Paint %d not found\n", paint_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    i2c_result_e err =
        i2c_read_pages(EEPROM_ADDR, page_no_start * page_size, I2C_REGADDR_2B, (uint8_t*)data, sizeof_paint_data);
    printf("Load paint result: %d\n", err);
    Delay_Ms(3);
    printf("Paint %d loaded\n", paint_no);
}

void flushCanvas(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, canvas[i].color);
    }
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void displayColorPalette(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, colors[i]);
    }
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    printf("Color palette displayed\n");
}

void colorPaletteSelection(color_t * selectedColor) {
    displayColorPalette();
    while (1) {
        int8_t button = matrix_pressed_two();
        if (button != no_button_pressed) {
            *selectedColor = colors[button];
            break;
        }
        Delay_Ms(200);
    }
    printf("Selected color: R:%d G:%d B:%d\n", selectedColor->r, selectedColor->g,
        selectedColor->b);
    flushCanvas();
}

void painting_routine(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        canvas[i].layer = BACKGROUND_LAYER;
        canvas[i].color = (color_t){0, 0, 0};
    }
    flushCanvas();
    while (1) {
        Delay_Ms(200);
        // printf("Foreground color: R:%d G:%d B:%d\n", foreground.r, foreground.g,
        // foreground.b); printf("Background color: R:%d G:%d B:%d\n", background.r,
        // background.g, background.b);
        printf("test\n");
        int8_t user_input = matrix_pressed_two();
        if (user_input == no_button_pressed) {
            if (JOY_Y_pressed()) {
                colorPaletteSelection(&foreground);
            }
            else if (JOY_X_pressed()) {
                colorPaletteSelection(&background);
            }
            else if (JOY_down_pressed()) {
                printf("System reset\n");
                NVIC_SystemReset();
            }
            else if (JOY_up_pressed()) {
                // save paint
                printf("Exit paint mode, entering save\n");
                break;
            }
            continue;
        }
        // user sets canvas color
        if (canvas[user_input].layer == BACKGROUND_LAYER) {
            canvas[user_input].layer = FOREGROUND_LAYER;
            canvas[user_input].color = foreground;
        }
        else {
            canvas[user_input].layer = BACKGROUND_LAYER;
            canvas[user_input].color = background;
        }
        printf("Canvas[%d] set to %s layer\n", user_input,
            canvas[user_input].layer == FOREGROUND_LAYER ? "FOREGROUND" : "BACKGROUND");
        printf("Canvas color set to R:%d G:%d B:%d\n", canvas[user_input].color.r,
            canvas[user_input].color.g, canvas[user_input].color.b);
        flushCanvas();
    }
}

// snake game

typedef struct snakePartDir {
    char part;
    int8_t direction;
} snakePartDir;

color_t appleColor = {.r = 255, .g = 0, .b = 0};        // red
color_t snakeHeadColor = {.r = 0, .g = 255, .b = 255};  // Green
color_t snakeBodyColor = {.r = 51, .g = 255, .b = 51};  // Green
color_t snakeTailColor = {.r = 255, .g = 255, .b = 51}; // Yellow

snakePartDir snake_game_gameboard[64]; // 8x8 gameboard
int8_t snakeHead = 45;
int8_t snakeTail = 47;

void game_init(void) {
    char * ptr;
    for (ptr = (char *)snake_game_gameboard; ptr < (char *)(snake_game_gameboard + 64);
         ptr += sizeof(snakePartDir)) {
        *(snakePartDir *)ptr = (snakePartDir){'0', 0};
    }
    snake_game_gameboard[47] = (snakePartDir){'t', -1};
    snake_game_gameboard[46] = (snakePartDir){'b', -1};
    snake_game_gameboard[45] = (snakePartDir){'h', 0};
    snake_game_gameboard[42] = (snakePartDir){'a', 0};
}
void display(void) {
    clear();
    for (int i = 0; i < 64; i++) {
        switch (snake_game_gameboard[i].part) {
            case 't':
                set_color(i, snakeTailColor);
                break;
            case 'b':
                set_color(i, snakeBodyColor);
                break;
            case 'h':
                set_color(i, snakeHeadColor);
                break;
            case 'a':
                set_color(i, appleColor);
                break;
            default:
                set_color(i, (color_t){0, 0, 0});
                break;
        }
    }
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

int8_t direction(int8_t currentDirection) {
    if (JOY_up_pressed() && snake_game_gameboard[snakeHead + 8].part != 'b') {
        return 8; // go up
    }
    if (JOY_down_pressed() && snake_game_gameboard[snakeHead - 8].part != 'b') {
        return -8; // go down
    }
    if (JOY_left_pressed() && snake_game_gameboard[snakeHead + 1].part != 'b') {
        return 1; // go left
    }
    if (JOY_right_pressed() && snake_game_gameboard[snakeHead - 1].part != 'b') {
        printf("right\n");
        return -1; // go right
    }
    // check if the snake is running to itself
    // if not then set the direction
    return currentDirection;
}

bool checkApple(int8_t currentDirection) {
    if (snake_game_gameboard[snakeHead + currentDirection].part == 'a') {
        return true;
    }
    return false;
}

void generate_apple(void) {
    uint8_t applePos;
    do {
        applePos = JOY_random() % 64;
    } while (snake_game_gameboard[applePos].part != '0');
    snake_game_gameboard[applePos] = (snakePartDir){'a', 0};
    // random a location to put down an apple
    // random till that is a empty space
}

void moveSnake(int8_t currentDirection, const bool apple) {
    snake_game_gameboard[snakeHead].part = 'b';
    snake_game_gameboard[snakeHead].direction = currentDirection;
    snakeHead += currentDirection;
    snake_game_gameboard[snakeHead].part = 'h';
    if (apple) {
        return;
    }
    // if the snake eats the apple, the tail will not move
    // move the tail when the snake does not eat the apple
    snake_game_gameboard[snakeTail].part = '0';
    snakeTail += snake_game_gameboard[snakeTail].direction;
    snake_game_gameboard[snakeTail].part = 't';
}

bool collision(int8_t currentDirection) {
    int8_t nextSnakeHead = snakeHead + currentDirection;
    if (snake_game_gameboard[nextSnakeHead].part == 'b' || snake_game_gameboard[nextSnakeHead].part == 't') {
        return true;
    }
    if (nextSnakeHead % 8 == 0 && currentDirection == 1) {
        return true;
    }
    if (nextSnakeHead % 8 == 7 && currentDirection == -1) {
        return true;
    }
    if (nextSnakeHead < 0 || nextSnakeHead > 63) {
        return true;
    }
    return false;
}


void snake_game_routine(void) {
    game_init();
    display();
    uint16_t seed = 0;
    uint8_t score = 0;
    bool apple;
    while (!JOY_Y_pressed()) {
        seed++;
        if (seed > 0xFFF0) {
            seed = 0;
        } // boundary check of uint16_t rnval
        // generate a seed according to the time between boot and button pressed
        Delay_Ms(10);
        // wait for the button to be pressed
    }
    JOY_setseed(seed);
    int8_t currentDirection = -1;
    while (1) {
        currentDirection = direction(currentDirection);
        if (collision(currentDirection)) {
            break;
        }
        apple = checkApple(currentDirection);
        moveSnake(currentDirection, apple);
        if (apple) {
            generate_apple();
            score++;
        }
        display();
        Delay_Ms(700);
    }
    drawScore(score);
    while (!JOY_Y_pressed()) {
        Delay_Ms(1);
        // wait for the button to be pressed
    }
    NVIC_SystemReset();
}

void drawScore(uint8_t score) {
    clear();
    const uint8_t tenth_digit = score / 10;
    const uint8_t unit_digit = score % 10;
    font_draw(font_list[tenth_digit], colors[8 * tenth_digit % num_colors], 4);
    font_draw(font_list[unit_digit], colors[8 * unit_digit % num_colors], 0);
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

// tic-tac-toe game

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
            position = matrix_pressed_two();
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

void tic_tac_toe_routine(void) {
    char winner;
    clear();
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

// end of tic-tac-toe game

// moving car

// Save which buttons are toggledy
uint8_t toggle_state[NUM_LEDS] = {0};

uint8_t is_array_all_one(uint8_t *array, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        if (array[i] != 1) {
            return 0;
        }
    }
    return 1;
}

void display_simple_animation(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        clear();
        set_color(i, colors[i]);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(10);
    }
}

void movingcar_routine(void) {
    funGpioInitAll();
    funPinMode(L0, FUN_OUTPUT);
    funPinMode(L1, FUN_OUTPUT);
    funPinMode(R0, FUN_OUTPUT);
    funPinMode(R1, FUN_OUTPUT);

    display_simple_animation();

    // Clean up
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);


    while (1) {
        if (JOY_Y_pressed()) {
            break;
        }

        int8_t button = matrix_pressed_two();
        if (button != -1) {
            printf("Button: %d\n", button);
            Delay_Ms(150);
            toggle_state[button] = !toggle_state[button];
        }

        // Display toggled LEDs
        for (int i = 0; i < NUM_LEDS; i++) {
            color_t color_tobe_displayed = {255, 255, 0};
            if (i % 8 <= 3) {
                color_tobe_displayed = (color_t){0, 255, 255};
            }
            set_color(i, toggle_state[i] ? color_tobe_displayed : (color_t){0, 0, 0});
        }
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        printf("Loop\n");
    }

    // Start executing the car moving instructions
    // and fill LEDs with color.

    #define RED_COLOR_BUTTON 3
    #define GREEN_COLOR_BUTTON 2
    #define BLUE_COLOR_BUTTON 1
    #define X_COLOR_BUTTON 0

    uint8_t execution_step = 0;
    while (execution_step < verticalButtons) {
        // Debug infos
        printf("Step: %d\n", execution_step);
        printf("Toggle state: ");
        for (int i = 0; i < NUM_LEDS; i++) {
            printf("%d ", toggle_state[i]);
        }
        printf("\n");
        // Stop Car.
        funDigitalWrite(L0, 0);
        funDigitalWrite(L1, 0);
        funDigitalWrite(R0, 0);
        funDigitalWrite(R1, 0);

        const uint8_t starting_index = horizontalButtons*(verticalButtons-execution_step) - horizontalButtons;
        // verify if RESET command is detected.
        if (is_array_all_one(&toggle_state[starting_index], horizontalButtons)) {
            execution_step = 0;
            continue;
        }

        // Fill the LEDs with the color of the current step
        color_t current_step_color = {1,1,1};
        
        current_step_color.r = toggle_state[starting_index + RED_COLOR_BUTTON] ? 255 : 0;
        current_step_color.g = toggle_state[starting_index + GREEN_COLOR_BUTTON] ? 255 : 0;
        current_step_color.b = toggle_state[starting_index + BLUE_COLOR_BUTTON] ? 255 : 0;

        fill_color(current_step_color);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

        // Move the car according to the current step
        #define L0_ON_INDEX 7
        #define L1_ON_INDEX 6
        #define R0_ON_INDEX 5
        #define R1_ON_INDEX 4

        funDigitalWrite(L0, toggle_state[starting_index + L0_ON_INDEX]);
        funDigitalWrite(L1, toggle_state[starting_index + L1_ON_INDEX]);
        funDigitalWrite(R0, toggle_state[starting_index + R0_ON_INDEX]);
        funDigitalWrite(R1, toggle_state[starting_index + R1_ON_INDEX]);

        // Proceed to next step
        execution_step++;

        // Delays
        Delay_Ms(1000);
    }
    NVIC_SystemReset();
}

// end of moving car
