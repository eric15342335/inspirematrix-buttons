/**
 * Originally from `t2.c`.
*/

#include <stdio.h>

#include "riscv-disas.h"
#include "driver.h"
#include "oled_min.h"
#include "i2c_events.h"

#define array_size(arr) (sizeof(arr) / sizeof(arr[0]))
#define I2C_MASTER_PROGRAM 0
#define I2C_SLAVE_OLED 1
#define I2C_MODE I2C_MASTER_PROGRAM

#define MAIN_CPU_I2C_ADDRESS 0x02
#define SLAVE_CPU_I2C_ADDRESS 0x04
#define OLED_I2C_ADDRESS OLED_ADDR // Just a note to myself

void IIC_Init(uint16_t address) {
    // Enable clocks for GPIOC and I2C1
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

    // Configure PC2 and PC1 as open-drain
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);

    // Set module clock frequency
    uint32_t prerate =
        2000000; // I2C Logic clock rate, must be higher than the bus clock rate
    I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;
    // 24MHz?
    // Set clock configuration
    uint32_t clockrate =
        1000000; // I2C Bus clock rate, must be lower than the logic clock rate
    I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK / (3 * clockrate)) & I2C_CKCFGR_CCR) |
                   I2C_CKCFGR_FS; // Fast mode 33% duty cycle
    // I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK/(25*clockrate))&I2C_CKCFGR_CCR) |
    // I2C_CKCFGR_DUTY | I2C_CKCFGR_FS; // Fast mode 36% duty cycle I2C1->CKCFGR =
    // (FUNCONF_SYSTEM_CORE_CLOCK/(2*clockrate))&I2C_CKCFGR_CCR; // Standard mode good to
    // 100kHz

    I2C1->OADDR1 = address; // Set own address
    I2C1->OADDR2 = 0;
    /* DO NOT SWAP THESE TWO ORDER !!!!!!!!!!!!!
    This bit is set and cleared by
    software and cleared by hardware when start is
    sent or PE=0.*/
    I2C1->CTLR1 |= I2C_CTLR1_PE;  // Enable I2C
    I2C1->CTLR1 |= I2C_CTLR1_ACK; // Enable ACK
    printf("\nInitializing I2C...\n");
}


#if I2C_MODE == I2C_MASTER_PROGRAM

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

void split_and_flip_program(uint16_t program, uint8_t * first_half, uint8_t * second_half)
{
    // reverse the program[], chop each 16bit into 2 8 bits,
    // put least significant 8 bits into the first byte,
    // put most significant 8 bits into the second byte
    // and put them into inst_arr[]
    for (size_t i = 0; i < 2; i++)
    {
        *first_half = program & 0xff;
        *second_half = program >> 8;
    }
}


int main(void) {
    SystemInit();
    printf("IIC Host mode\r\n");
    IIC_Init(MAIN_CPU_I2C_ADDRESS);

    while (1) {
        uint32_t timeout = timeout_default;
        while (I2C1->STAR1 & I2C_STAR2_BUSY) {
            if (--timeout == 0) {
                NVIC_SystemReset();
            }
        }

        I2C1->CTLR1 |= I2C_CTLR1_START;
        printf("I2C generated start!\n");

        wait_for_event(I2C_EVENT_MASTER_MODE_SELECT);
        printf("I2C Event master mode selected!\n");

        I2C1->DATAR = SLAVE_CPU_I2C_ADDRESS;
        printf("I2C Write 7-bit Address\n");

        wait_for_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
        printf("I2C Sent 7-bit Address\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        for (int i = 0; i < 2; i++) {
            uint8_t first_half, second_half = 0;
            split_and_flip_program(program[i], &first_half, &second_half);
            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = first_half;

            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = second_half;
            printf("Sent [%d] with %d and %d\r\n", i, first_half, second_half);
        }
        printf("Sending finished!\r\n");

        wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
        printf("I2C Event master byte transmitted!\r\n");

        I2C1->CTLR1 |= I2C_CTLR1_STOP;
        printf("I2C Generated STOP!\r\n");
    }
}

#elif I2C_MODE == I2C_SLAVE_OLED

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

/*
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
*/
uint8_t program[2];

void t1()
{
    while (1) {
        printf("IIC Slave mode\r\n");
        IIC_Init(SLAVE_CPU_I2C_ADDRESS);
        printf("Waiting for receiver address match!\n");
        wait_for_event(I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED);
        printf("I2C Event slave receiver address matched!\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        uint8_t i = 0;
        while (i < 2) {
            wait_for_event(I2C_EVENT_SLAVE_BYTE_RECEIVED);
            program[i] = I2C1->DATAR;
            printf("Received! %d\n", program[i]);
            i++;
        }
        I2C1->CTLR1 &= I2C1->CTLR1;
        
        OLED_init();
        dissassemble(0x0, program, array_size(program));
    }
}

int main()
{
    SystemInit();
    t1();
    NVIC_SystemReset();
}
#endif
