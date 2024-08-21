#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "riscv-disas.h"
#include "driver.h"
#include "oled_min.h"
#include "ws2812b_simple.h"
#include "colors.h"

#define array_size(arr) (sizeof(arr) / sizeof(arr[0]))
#define instruction_size 16
#define num_of_instructions_display (NUM_LEDS / instruction_size)
#define LED_PINS GPIOA, 2

const color_t instructions_color = {255, 0, 0};
const color_t highlight_color = {0, 255, 0};

void display_four_instructions(size_t offset);

void highlight_instruction(size_t offset);

void dissassemble(uint64_t pc, const uint8_t *data, size_t data_len)
{
    size_t offset = 0, inst_len;
    rv_inst inst;
    while (offset < data_len)
    {   
        display_four_instructions(offset / inst_len);

        char buf[128] = {'\0'};
        inst_fetch(data + offset, &inst, &inst_len);
        disasm_inst(buf, sizeof(buf), rv32, pc + offset, inst);

        OLED_clear();

        OLED_print("PC   : 0x");
        OLED_printW(pc + offset);
        printf("PC: 0x%lx", pc + offset);

        OLED_print("\nBytes: 0x");
        OLED_printW(inst);
        printf("\nBytes: 0x%04x", inst);

        OLED_print("\nDisassembly:\n");
        OLED_println(buf);
        printf("\nDisassembly:\n%s\n", buf);
        _OLED_refresh_display();
        
        offset += inst_len;
        Delay_Ms(700);
    }
}

const uint16_t program[] = {
    // 0: c.li a0, 5
    0x4515,
    // 2: c.li a1, 4
    0x4591,
    // 4: c.or a2, a0
    0x8e49,
    // 6: c.addi a1, -1
    0x15fd,
    // 8: c.beqz a1, 8
    0xc581,
    // 10: c.addi a1, -1
    0x15fd,
    // 12: c.add a0, a2
    0x9532,
    // 14: c.j -6
    0xbfed,
    // 16: ecall
    0x0073,
};

void t1()
{
    uint8_t inst_arr[array_size(program) * 2];
    // reverse the program[], chop each 16bit into 2 8 bits,
    // put least significant 8 bits into the first byte,
    // put most significant 8 bits into the second byte
    // and put them into inst_arr[]
    for (size_t i = 0; i < array_size(program); i++)
    {
        inst_arr[i * 2] = program[i] & 0xff;
        inst_arr[i * 2 + 1] = program[i] >> 8;
    }
    dissassemble(0x0, inst_arr, array_size(inst_arr));
}

int main()
{
    SystemInit();
    OLED_init();
    printf("Hello, world!\n");
    t1();
    NVIC_SystemReset();
}

void display_four_instructions(size_t offset) {
    clear();
    // display 4 16bit instructions in led 0-15, 16-31, 32-47, 48-63
    // Index of instructions is determined by offset.
    const uint8_t instruction_offset = (offset / num_of_instructions_display) * num_of_instructions_display;
    printf("offset: %d, num: %d, inst_offset: %d\n", offset, num_of_instructions_display, instruction_offset);
    printf("program: %lX, %lX, %lX, %lX\n", program[instruction_offset], program[instruction_offset + 1], program[instruction_offset + 2], program[instruction_offset + 3]);
    for (uint8_t i = 0; i < num_of_instructions_display; i++) {
        for (int bit = instruction_size - 1; bit >= 0; bit--) {
            if (program[i + instruction_offset] & (1 << bit)) {
                set_color(i * instruction_size + bit, instructions_color);
            }
        }
    }
    highlight_instruction(offset);
    WS2812BSimpleSend(LED_PINS, (uint8_t *)led_array, NUM_LEDS * 3);
}

void highlight_instruction(size_t offset) {
    // highlight the instruction at offset.
    for (int bit = instruction_size - 1; bit >= 0; bit--) {
        if (program[offset] & (1 << bit)) {
            set_color((offset % num_of_instructions_display) * instruction_size + bit, highlight_color);
        }
    }
}
