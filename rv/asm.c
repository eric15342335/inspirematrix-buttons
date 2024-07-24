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

rv_u16 program[11] = {
    // description: multiply a0 by a1
    // load 5 into a0, 4 into a1
    // 0: c.li a0, 5
    0x4515,
    // 2: c.li a1, 4
    0x4591,
    // copy a0 to a2
    // 4: c.andi a2, 0
    0x8a01,
    // 6: c.or a2, a0
    0x8e49,
    // 8: c.addi a1, -1
    0x15fd,
    // 10: c.beqz a1, 8
    0xc581,
    // 12: c.addi a1, -1
    0x15fd,
    // 14: c.add a0, a2
    0x9532,
    // 16: c.j -6
    0xbfed,
    // 18: ecall
    0x0073
};

void display_all_registers(rv * cpu) {
    for (int i = 0; i < 32; i++) {
        if (cpu->r[i] != 0)
            printf("r%d: %d ", i, cpu->r[i]);
    }
    printf("\n");
}


int main(void) {
    SystemInit();
    printf("Hello, World!\n");
    rv_u8 mem[RAM_SIZE];
    rv cpu;
    rv_init(&cpu, (void *)mem, &bus_cb);
    memcpy((void *)mem, (void *)program, sizeof(program));
    clear();
    WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
    Delay_Ms(3000);
    while (1) {
        rv_u32 trap = rv_step(&cpu);
        clear();
        uint8_t next_pos = (cpu.pc - RAM_BASE) / sizeof(program[0]);
        for (int bit = 15; bit >= 0; bit--) {
            // display 16bit instruction at PC in led 0-15
            if (program[next_pos] & (1 << bit)) {
                set_color(bit, (color_t){255, 255, 255});
            }
            // display register r10, r11, r12 value at led 16-31, 32-47, 48-63
            if (cpu.r[10] & (1 << bit)) {
                set_color(bit + 16, (color_t){255, 0, 0});
            }
            if (cpu.r[11] & (1 << bit)) {
                set_color(bit + 32, (color_t){0, 255, 0});
            }
            if (cpu.r[12] & (1 << bit)) {
                set_color(bit + 48, (color_t){0, 0, 255});
            }
        }
        WS2812BSimpleSend(GPIOC, 1, (uint8_t *)led_array, NUM_LEDS * 3);
        printf("PC: %X, ", cpu.pc);
        printf("Trap: %X\n", trap);
        printf("Opcode: %X, ", program[next_pos]);
        display_all_registers(&cpu);
        if (trap == RV_EMECALL)
            break;
        Delay_Ms(500);
    }
    printf("Environment call @ %X\n", cpu.csr.mepc);
    display_all_registers(&cpu);
    NVIC_SystemReset();
}

