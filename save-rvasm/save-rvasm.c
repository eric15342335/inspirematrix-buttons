#define CH32V003_I2C_IMPLEMENTATION
#define WS2812BSIMPLE_IMPLEMENTATION
#include "colors.h"
#include "driver.h"
#include "rv.h"
#include "ws2812b_simple.h"
#include "ch32v003_i2c.h"

#include <stdio.h>
#include <string.h>

// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

#define RAM_BASE 0x80000000
#define RAM_SIZE 0x400

#define LED_PINS GPIOA, 2

rv_res bus_cb(void * user, rv_u32 addr, rv_u8 * data, rv_u32 is_store, rv_u32 width);

// Just a printf wrapper
void display_all_registers(rv * cpu);

#define num_of_instructions 32
#define led_page_size_show 2
#define num_of_led_page_show (num_of_instructions / led_page_size_show)

// rv16 bit
#define size_of_one_saveprogram (num_of_instructions * sizeof(rv_u16))
#define size_of_one_saveprogram_in_page (size_of_one_saveprogram / page_size)
#define page_size 64
// range of byte that stores status of page[x]
#define init_status_addr_begin 0
#define init_status_addr_end 7
#define init_status_reg_size (init_status_addr_end - init_status_addr_begin + 1)

#define init_status_format "  %c "
#define init_status_data (uint8_t *)"IL000001"

#define page_status_addr_begin 8
#define page_status_addr_end 511
#define page_status_reg_size (page_status_addr_end - page_status_addr_begin + 1)

#define delay 1000
#define matrix_hori 16

/*
Given:
#define paint_page_no (8 * sizeof_paint_data_aspage)
#define paint_page_no_max (8 * sizeof_paint_data_aspage)
It is recommended to separate paint storage and
program storage to different pages with gap.
*/
#define program_store_page_no ((8+64) * size_of_one_saveprogram_in_page)
// the size of all program pages
#define program_store_page_no_max (64 * size_of_one_saveprogram_in_page)

void rv_asm_routine(void);
rv_res bus_cb(void * user, rv_u32 addr, rv_u8 * data, rv_u32 is_store, rv_u32 width);
void display_all_registers(rv * cpu);
void init_storage(void);
void reset_storage(void);
uint8_t is_storage_initialized(void);
uint8_t is_page_used(uint16_t page_no);
void red_screen(void);

void choose_save_program_page(void);
void choose_load_program_page(void);
void led_display_program_page_status(void);
void save_program(uint16_t program_no, rv_u16 * _program);
void load_program(uint16_t program_no, rv_u16 * _program);
void erase_all_program_saves(void);

const color_t color_savefile_exist = {.r = 0, .g = 0, .b = 100};
const color_t color_savefile_empty = {.r = 0, .g = 100, .b = 0};

// print storage data to console
void print_status_storage(void);
void set_page_status(uint16_t page_no, uint8_t status);

void set_page_status(uint16_t page_no, uint8_t status) {
    if (status > 1) {
        printf("Invalid status %d\n", status);
        printf("DEBUG: %d\n", __LINE__);
        while (1)
            ;
    }
    if (page_no < page_status_addr_begin || page_no > page_status_addr_end) {
        printf("Invalid page number %d\n", page_no);
        printf("DEBUG: %d\n", __LINE__);
        while (1)
            ;
    }
    i2c_write(EEPROM_ADDR, page_no, I2C_REGADDR_2B, &status, sizeof(status));
    Delay_Ms(3);
    printf("Page %d status set to %d\n", page_no, status);
}
void print_status_storage(void) {
    printf("Status storage data:\n");
    for (uint16_t addr = init_status_addr_begin;
         addr < init_status_addr_begin + init_status_reg_size; addr++) {
        uint8_t data = 0;
        i2c_read(EEPROM_ADDR, addr, I2C_REGADDR_2B, &data, sizeof(data));
        printf(init_status_format, data);
    }
    for (uint16_t addr = page_status_addr_begin;
         addr < page_status_addr_begin + page_status_reg_size; addr++) {
        uint8_t data = 0;
        i2c_read(EEPROM_ADDR, addr, I2C_REGADDR_2B, &data, sizeof(data));
        if (data) {
            printf("%d ", addr);
        }
        else {
            printf("    ");
        }
        if ((addr + 1) % matrix_hori == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void add_no_page_ontop(uint16_t current_no_page) {
    if (current_no_page < 0 || current_no_page > num_of_led_page_show) {
        return;
    }
    for (uint16_t i = NUM_LEDS - 1; i >= NUM_LEDS - 1 - current_no_page; i--) {
        set_color(i, (color_t){30, 30, 30});
    }
}

rv_u16 program[num_of_instructions] = {
    // smile mouth
    // c.addi x5, 4
    0x0291,
    // c.slli x5, 4
    0x0292,
    // c.addi x5, 2
    0x0289,
    // c.slli x5, 4
    0x0292,
    // c.addi x5, 7
    0x029d,
    // c.slli x5, 4
    0x0292,
    // c.addi x5, 14
    0x02b9,
    // eyes
    // c.addi x6, 4
    0x0311,
    // c.slli x6, 4
    0x0312,
    // c.addi x6, 2
    0x0309,
    // c.slli x6, 8
    0x0322,
    // ecall
    0x0073,
    // 13-20: filler with zeros.
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    // 21-30:
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    // 31-32
    0x0000,
    0x0000,
};

void save_program(uint16_t program_no, rv_u16 * _program) {
    uint16_t addr = page_status_addr_begin + program_store_page_no + program_no;
    if (is_page_used(addr)) {
        printf("Paint %d already used, overwriting\n", addr);
        Delay_Ms(500);
    }
    set_page_status(addr, 1);
    addr *= page_size;
    i2c_write_pages(EEPROM_ADDR, addr, I2C_REGADDR_2B, (uint8_t *)_program, size_of_one_saveprogram);
    printf("Program %d saved at page %d\n", program_no, addr/page_size);
}

void load_program(uint16_t program_no, rv_u16 * _program) {
    uint16_t addr = page_status_addr_begin + program_store_page_no + program_no;
    if (!is_page_used(addr)) {
        printf("Paint %d not used, aborting\n", addr);
        return;
    }
    addr *= page_size;
    i2c_read_pages(EEPROM_ADDR, addr, I2C_REGADDR_2B, (uint8_t *)_program, size_of_one_saveprogram);
    printf("Program %d loaded.\n", program_no);
}

void erase_all_program_saves(void) {
    for (uint16_t addr = page_status_addr_begin + program_store_page_no;
            addr < page_status_addr_begin + program_store_page_no + program_store_page_no_max;
            addr+=size_of_one_saveprogram_in_page) {
        set_page_status(addr, 0);
        i2c_write(EEPROM_ADDR, addr, I2C_REGADDR_2B, (uint8_t[]){0}, sizeof(uint8_t));
        Delay_Ms(3);
    }
    printf("All program saves erased.\n");
}

void choose_save_program_page(void) {
    led_display_program_page_status();

    int8_t button = no_button_pressed;
    while (1) {
        button = matrix_pressed_two();
        if (button != no_button_pressed) {
            if (is_page_used(button * size_of_one_saveprogram_in_page + program_store_page_no +
                             page_status_addr_begin)) {
                printf("Page %d already used\n", button);
                // Overwrite save
            }
            printf("Selected page %d\n", button);
            save_program(button, program);
            printf("Program saved\n");
            Delay_Ms(1000);
            break;
        }
        Delay_Ms(200);
    }
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void choose_load_program_page(void) {
    led_display_program_page_status();
    
    int8_t button = no_button_pressed;
    while (1) {
        button = matrix_pressed_two();
        if (button != no_button_pressed) {
            if (!is_page_used(button * size_of_one_saveprogram_in_page + program_store_page_no +
                             page_status_addr_begin)) {
                printf("Page %d is not used\n", button);
                // Fill the screen with red to indicate error
                fill_color((color_t){.r = 100, .g = 0, .b = 0});
                WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
                Delay_Ms(1000);
                led_display_program_page_status();
                continue;
            }
            printf("Selected page %d\n", button);
            load_program(button, program);
            printf("Program loaded\n");
            Delay_Ms(1000);
            break;
        }
        if (JOY_Y_pressed()) {
            // Use default program[]
            printf("Use default program[]\n");
            break;
        }
        Delay_Ms(200);
    }
    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

}

void led_display_program_page_status(void) {
    clear();
    for (uint16_t addr = page_status_addr_begin + program_store_page_no;
            addr < page_status_addr_begin + program_store_page_no + program_store_page_no_max;
            addr+=size_of_one_saveprogram_in_page) {
        const uint16_t _led_no = ((addr - page_status_addr_begin - program_store_page_no) / size_of_one_saveprogram_in_page);
        printf("LED %d: ", _led_no);
        if (is_page_used(addr)) {
            set_color((_led_no),
                color_savefile_exist);
        }
        else {
            set_color(_led_no,
                color_savefile_empty);
        }
    }
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

#define instruction_size 16
typedef enum _which_focus {
    focus_instruction_0 = 0,
    focus_instruction_1 = 1,
    none = 2,
} which_focus;

uint8_t is_page_used(uint16_t page_no) {
    if (page_no < page_status_addr_begin || page_no > page_status_addr_end) {
        printf("Invalid page number %d\n", page_no);
        printf("DEBUG: %d\n", __LINE__);
        while (1)
            ;
    }
    uint8_t data = 0;
    i2c_read(EEPROM_ADDR, page_no, I2C_REGADDR_2B, &data, sizeof(data));
    printf("Page %d is %s\n", page_no, data ? "used" : "empty");
    return data;
}

void red_screen(void) {
    fill_color((color_t){.r = 100, .g = 0, .b = 0});
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

int main(void) {
    SystemInit();
    ADC_init();
    printf("Hello, World!\n");

    clear();
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(delay);
    i2c_init();
    printf("I2C Initialized\n");
    init_storage();

    print_status_storage();
    // Hold button X at startup to reset all paints
    uint16_t delay_countdown = 50;
    while (delay_countdown-- > 0) {
        if (JOY_X_pressed()) {
            erase_all_program_saves();
            // Visual indication of paint save reset
            red_screen();
            printf("Paint reset\n");
            printf("DEBUG: %d\n", __LINE__);
            Delay_Ms(1000);
        }
        Delay_Ms(1);
    }
    // choose which save to load

    choose_load_program_page();

    rv_asm_routine();

    choose_save_program_page();

    while (!JOY_Y_pressed()) {
        Delay_Ms(100);
    }
    NVIC_SystemReset();
}

void instructionDisplay(int16_t index, which_focus focus) {
    if (index < 0) {
        return;
    }
    const uint16_t starting_index = index * led_page_size_show;

#define default_code_color (color_t){.g = 255, .r = 0, .b = 0}
    color_t code1_color = default_code_color;
    if (focus == focus_instruction_0) {
        code1_color = (color_t){.b = 255, .r = 0, .g = 0};
    }
    color_t code2_color = default_code_color;
    if (focus == focus_instruction_1) {
        code2_color = (color_t){.b = 255, .r = 0, .g = 0};
    }

    // display 2 16bit instructions in led 0-15, 16-31
    for (int8_t bit = instruction_size - 1; bit >= 0; bit--) {
        if (program[starting_index] & (1 << bit)) {
            set_color(bit, code1_color);
        }
    }
    for (int8_t bit = instruction_size - 1; bit >= 0; bit--) {
        if (program[starting_index + 1] & (1 << bit)) {
            set_color(bit + instruction_size, code2_color);
        }
    }
}

const color_t white = {255, 255, 255};

void inputProgram(void) {
    // input 2 16bit instructions in led 0-15, 16-31
    int16_t page_of_instructions = 0;
    clear();
    instructionDisplay(page_of_instructions, none);
    add_no_page_ontop(page_of_instructions);
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
    while (1) {
        int8_t input = matrix_pressed_two();
        Delay_Ms(120);
        if (input == no_button_pressed) {
            if (JOY_down_pressed()) {
                page_of_instructions++;
                if (page_of_instructions > num_of_led_page_show) {
                    page_of_instructions = 0;
                }
                printf("page: %d\n", page_of_instructions);
            }
            else if (JOY_up_pressed()) {
                page_of_instructions--;
                if (page_of_instructions < 0) {
                    page_of_instructions = num_of_led_page_show;
                }
                printf("page: %d\n", page_of_instructions);
            }
            else if (JOY_Y_pressed()) {
                break;
            }
            else {
                continue;
            }
        }
        else {
            int16_t instruction_index = (led_page_size_show * page_of_instructions)
                + (input / instruction_size);
            int8_t bit_index = input % instruction_size;
            printf("instruction: %d, ", instruction_index);
            printf("bit: %d, ", bit_index);
            printf("program: %04x, ", program[instruction_index]);
            program[instruction_index] ^= (1 << bit_index);
            printf("new program: %04X\n", program[instruction_index]);
        }


        clear();
        instructionDisplay(page_of_instructions, none);
        add_no_page_ontop(page_of_instructions);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);

    }
}

#define register_r5_cpu_r_index 5
#define register_r6_cpu_r_index 6

#define register_r5_color (color_t){0, 255, 0}
#define register_r6_color (color_t){0, 0, 255}

void rv_asm_routine(void) {
    inputProgram();

    printf("Matrix Pressed\n");

    rv_u8 mem[RAM_SIZE];
    rv cpu;
    rv_init(&cpu, (void *)mem, &bus_cb);
    memcpy((void *)mem, (void *)program, sizeof(program));

    while (1) {
        rv_u32 trap = rv_step(&cpu);

        clear();
        for (int8_t bit = instruction_size - 1; bit >= 0; bit--) {
            // display register r10, r11 value at led 32-47, 48-63
            if (cpu.r[register_r5_cpu_r_index] & (1 << bit)) {
                set_color(bit + 32, register_r5_color);
            }
            if (cpu.r[register_r6_cpu_r_index] & (1 << bit)) {
                set_color(bit + 48, register_r6_color);
            }
        }
        if (trap == RV_EMECALL) {
            break;
        }
        uint8_t next_pos = (cpu.pc - RAM_BASE) / sizeof(program[0]);

        instructionDisplay((next_pos - 1) / led_page_size_show,
            ((next_pos - 1) % led_page_size_show) == 0 ? focus_instruction_0 : focus_instruction_1);
        WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
        
        printf("PC: %lX, ", cpu.pc);
        printf("Trap: %lX\n", trap);
        printf("Opcode: %X, ", program[next_pos]);
        printf("Next_pos: %d\n", next_pos);
        
        display_all_registers(&cpu);

        Delay_Ms(900);

        while (JOY_Y_pressed()) {
            Delay_Ms(100);
            printf("Waiting for Y to be released\n");
        }

        if (JOY_X_pressed()) {
            printf("Execution stopped\n");
            break;
        }
    }

    printf("Environment call @ %lX\n", cpu.csr.mepc);
    display_all_registers(&cpu);
}

rv_res bus_cb(void * user, rv_u32 addr, rv_u8 * data, rv_u32 is_store, rv_u32 width) {
    rv_u8 * mem = (rv_u8 *)user + addr - RAM_BASE;
    if (addr < RAM_BASE || addr + width >= RAM_BASE + RAM_SIZE)
        return RV_BAD;
    memcpy(is_store ? mem : data, is_store ? data : mem, width);
    return RV_OK;
}

void display_all_registers(rv * cpu) {
    for (int i = 0; i < 32; i++) {
        if (cpu->r[i] != 0)
            printf("r%d: %ld ", i, cpu->r[i]);
    }
    printf("\n");
}

void init_storage(void) {
    if (!is_storage_initialized()) {
        reset_storage();
        printf("Storage initialized\n");
    }
    else {
        printf("Storage already initialized\n");
    }
}

uint8_t is_storage_initialized(void) {
    uint8_t data[init_status_reg_size];
    i2c_read(
        EEPROM_ADDR, init_status_addr_begin, I2C_REGADDR_2B, data, init_status_reg_size);
    for (uint8_t i = 0; i < init_status_reg_size; i++) {
        if (data[i] != *(init_status_data + i)) {
            return 0;
        }
    }
    return 1;
}

void reset_storage(void) {
    i2c_write(EEPROM_ADDR, init_status_addr_begin, I2C_REGADDR_2B, init_status_data,
        init_status_reg_size);
    Delay_Ms(3);
    for (uint16_t addr = page_status_addr_begin;
         addr < page_status_addr_begin + page_status_reg_size; addr++) {
        i2c_write(EEPROM_ADDR, addr, I2C_REGADDR_2B, (uint8_t[]){0}, sizeof(uint8_t));
        Delay_Ms(3);
    }
    printf("Storage reset\n");
}
