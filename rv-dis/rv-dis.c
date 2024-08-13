/**
 * Originally from `t2.c`.
*/

#include <stdio.h>

#include "riscv-disas.h"
#include "driver.h"
#include "oled_min.h"

#define array_size(arr) (sizeof(arr) / sizeof(arr[0]))

void dissassemble(uint64_t pc, const uint8_t *data, size_t data_len)
{
    size_t offset = 0, inst_len;
    rv_inst inst;
    while (offset < data_len)
    {
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
    0x0073
};

void t1()
{
    static uint8_t inst_arr[array_size(program) * 2];
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
    t1();
    NVIC_SystemReset();
}
