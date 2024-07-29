#define WS2812BSIMPLE_IMPLEMENTATION
#include <stdio.h>
#include "driver.h"
#include "colors.h"
#include "ws2812b_simple.h"
#include "rv.h"
#include <string.h>
#define RAM_BASE 0x80000000
#define RAM_SIZE 0x100

rv_res bus_cb(
    void * user, rv_u32 addr, rv_u8 * data, rv_u32 is_store, rv_u32 width) {
    rv_u8 * mem = (rv_u8 *)user + addr - RAM_BASE;
    if (addr < RAM_BASE || addr + width >= RAM_BASE + RAM_SIZE)
        return RV_BAD;
    memcpy(is_store ? mem : data, is_store ? data : mem, width);
    return RV_OK;
}
rv_u16 program[4] = {
    // c.li x10, -11
    0x5555,
    // c.li x11, 11
    0x45ad,
    // ecall
    0x0073
};

void display_all_registers(rv * cpu) {
    for (int i = 0; i < 32; i++) {
        if (cpu->r[i] != 0)
            printf("r%d: %ld ", i, cpu->r[i]);
    }
    printf("\n");
}

void instructionDisplay(void) {
    // display 2 16bit instructions in led 0-15, 16-31
    for (int bit = 15; bit >= 0; bit--) {
        if (program[0] & (1 << bit)) {
            set_color(bit, (color_t){255, 0, 0});
        }
    }
    for (int bit = 15; bit >= 0; bit--) {
        if (program[1] & (1 << bit)) {
            set_color(bit + 16, (color_t){255, 0, 0});
        }
    }
}

void inputProgram(void) {
    // input 2 16bit instructions in led 0-15, 16-31
    while (1) {
        int8_t input = matrix_pressed(ADC_read_smallboard);
        Delay_Ms(100);
        if (input == no_buttons_pressed) {
            continue;
        }
        if (input == 32) {
            break;
        }
        printf("program: %lX\n", program[input / 16]);
        printf("bit: %d\n", input % 16);
        program[input / 16] ^= (1 << (input % 16));
        printf("program: %lX\n", program[input / 16]);
        clear();
        instructionDisplay();
        WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
    }
}

int main(void) {
    SystemInit();
    ADC_init();
    printf("Hello, World!\n");
    rv_u8 mem[RAM_SIZE];
    rv cpu;
    rv_init(&cpu, (void *)mem, &bus_cb);
    memcpy((void *)mem, (void *)program, sizeof(program));

    clear();
    set_color(32, (color_t){255, 255, 255});
    instructionDisplay();
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
    
    inputProgram();

    printf("Matrix Pressed\n");

    while (1) {
        rv_u32 trap = rv_step(&cpu);
        clear();
        uint8_t next_pos = (cpu.pc - RAM_BASE) / sizeof(program[0]);
        for (int bit = 15; bit >= 0; bit--) {
            // display register r10, r11 value at led 32-47, 48-63
            if (cpu.r[10] & (1 << bit)) {
                set_color(bit + 32, (color_t){0, 255, 0});
            }
            if (cpu.r[11] & (1 << bit)) {
                set_color(bit + 48, (color_t){0, 0, 255});
            }
        }
        instructionDisplay();
        WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
        printf("PC: %lX, ", cpu.pc);
        printf("Trap: %lX\n", trap);
        printf("Opcode: %X, ", program[next_pos]);
        display_all_registers(&cpu);
        if (trap == RV_EMECALL)
            break;
        Delay_Ms(1000);
    }
    printf("Environment call @ %lX\n", cpu.csr.mepc);
    display_all_registers(&cpu);
    while (!JOY_act_pressed())
        ;
    NVIC_SystemReset();
}

