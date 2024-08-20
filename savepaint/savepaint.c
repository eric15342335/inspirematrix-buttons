#define CH32V003_I2C_IMPLEMENTATION
#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
#include "colors.h"
#include "driver.h"
#include "ws2812b_simple.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

// initialize file storage structure for 32kb/512pages
// first 8 pages are used for status
void init_storage(void);

// save paint data to eeprom, paint 0 stored in page 8-10 (out of page 0 to 511)
void save_paint(uint16_t paint_no, color_t *data);

// load paint data from eeprom, paint 0 stored in page 8-10 (out of page 0 to 511)
void load_paint(uint16_t paint_no, color_t *data);

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

#define paint_page_no 0
#define paint_page_no_max (8*sizeof_paint_data_aspage)
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

void flushCanvas(void);

void displayColorPalette(void);

void colorPaletteSelection(color_t * selectedColor);

void painting_routine(void);

void setup_unique_pattern(void);

void choose_save_paint_page(void);

void led_display_paint_page_status(void);


void display_stored_paints(void);

int main(void) {
    SystemInit();
    ADC_init();
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(delay);

    i2c_init();
    printf("I2C Initialized\n");
    init_storage();

    // Hold button Y at startup to reset storage
    uint16_t delay_countdown = 50;
    while (delay_countdown-- > 0) {
        if (JOY_Y_pressed()) {
            reset_storage();
            // Visual indication of storage reset
            fill_color((color_t){.r = 100, .g = 0, .b = 0});
            WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
            printf("Storage reset\n");
            printf("DEBUG: %d\n", __LINE__);
            Delay_Ms(1000);
        }
        Delay_Ms(1);
    }

    print_status_storage();

    display_stored_paints();

    painting_routine();

    choose_save_paint_page();

    Delay_Ms(delay);
    while(1) {
        if (JOY_up_pressed()) {
            NVIC_SystemReset();
        }
        Delay_Ms(200);
    }
}

void display_stored_paints(void) {
    for (uint16_t _paint_page_no = paint_page_no; _paint_page_no < paint_page_no_max; _paint_page_no+=sizeof_paint_data_aspage) {
        clear();
        if (!is_page_used(_paint_page_no + page_status_addr_begin) ||
            !is_page_used(_paint_page_no + page_status_addr_begin + 1) ||
            !is_page_used(_paint_page_no + page_status_addr_begin + 2)) {
            printf("Paint %d not found\n", _paint_page_no / 3);
            printf("DEBUG: %d\n", __LINE__);
            break;
        }
        printf("Displaying paint %d\n", _paint_page_no);
        load_paint(_paint_page_no / sizeof_paint_data_aspage, led_array);
        // setup_unique_pattern();
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        Delay_Ms(delay);
    }
    printf("All stored paints displayed\n");
}

void led_display_paint_page_status(void) {
    clear();
    for (uint16_t _paint_page_no = 0; _paint_page_no < paint_page_no_max - paint_page_no; _paint_page_no+=sizeof_paint_data_aspage) {
        if (is_page_used(_paint_page_no + page_status_addr_begin) &&
            is_page_used(_paint_page_no + page_status_addr_begin + 1) &&
            is_page_used(_paint_page_no + page_status_addr_begin + 2)) {
            set_color(_paint_page_no / sizeof_paint_data_aspage, color_savefile_exist);
        } else {
            set_color(_paint_page_no / sizeof_paint_data_aspage, color_savefile_empty);
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
                continue;
            }
            printf("Selected page %d\n", button);
            // Put canvas to led_array
            for (int i = 0; i < NUM_LEDS; i++) {
                set_color(i, canvas[i].color);
            }
            save_paint(button, led_array);
            printf("Paint saved\n");
            break;
        }
        Delay_Ms(200);
    }
    // Display saved paint
    display_stored_paints();
    Delay_Ms(delay);
    printf("Saved paint %d displayed\n", button);
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
    uint16_t page_no_start = paint_no * sizeof_paint_data_aspage + page_status_addr_begin;
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

void load_paint(uint16_t paint_no, color_t *data) {
    if (paint_no < 0 || paint_no > page_status_addr_end) {
        printf("Invalid paint number %d\n", paint_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    uint16_t page_no_start = paint_no * sizeof_paint_data_aspage + page_status_addr_begin;
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
