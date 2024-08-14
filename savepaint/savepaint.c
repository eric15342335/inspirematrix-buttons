#define CH32V003_I2C_IMPLEMENTATION
#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
#include "colors.h"
#include "ws2812b_simple.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

// initialize file storage structure for 32kb/512pages
// first 8 pages are used for status
void init_storage(void);

// save paint data to eeprom, paint 0 stored in page 8 (out of page 0 to 511)
void save_paint(uint16_t paint_no, color_t *data);

// load paint data from eeprom, paint 0 stored in page 8 (out of page 0 to 511)
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
#define delay 1000

void setup_unique_pattern(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_color(i, colors[i]);
    }
}

int main(void) {
    SystemInit();
    clear();
    WS2812BSimpleSend(GPIOC, 5, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(delay);

    i2c_init();
    printf("I2C Initialized\n");
    init_storage();
    // reset_storage();
    Delay_Ms(delay);
    print_status_storage();

    if (is_page_used(paint_page_no + page_status_addr_begin)) {
        load_paint(paint_page_no, led_array);
    }
    
    // setup_unique_pattern();
    WS2812BSimpleSend(GPIOC, 5, (uint8_t *)led_array, NUM_LEDS * 3);

    if (!is_page_used(paint_page_no + page_status_addr_begin)) {
        save_paint(paint_page_no, led_array);
    }

    Delay_Ms(delay);
    while(1);
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
    uint16_t page_no = paint_no + page_status_addr_begin;
    if (is_page_used(page_no)) {
        printf("Paint %d already used, overwriting\n", paint_no);
    }
    set_page_status(page_no, 1);
    i2c_result_e err =
        i2c_write_pages(EEPROM_ADDR, page_no * page_size, I2C_REGADDR_2B, (uint8_t*)data, sizeof_paint_data);
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
    uint16_t page_no = paint_no + page_status_addr_begin;
    if (!is_page_used(page_no)) {
        printf("Paint %d not found\n", paint_no);
        printf("DEBUG: %d\n", __LINE__);
        while(1);
    }
    i2c_result_e err =
        i2c_read_pages(EEPROM_ADDR, page_no * page_size, I2C_REGADDR_2B, (uint8_t*)data, sizeof_paint_data);
    printf("Load paint result: %d\n", err);
    Delay_Ms(3);
    printf("Paint %d loaded\n", paint_no);
}