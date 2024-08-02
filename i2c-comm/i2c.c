/*
 *@Note
 *7-bit addressing mode, master/slave mode, transceiver routine:
 *I2C1_SCL(PC2)\I2C1_SDA(PC1).
 *This routine demonstrates that Master sends and Slave receives.
 *Note: The two boards download the Master and Slave programs respectively,
 *    and power on at the same time.
 *      Hardware connection:
 *            MCU 1    MCU 2
 *            [PC2]-----[PC2  PD2] (Internal resistor)
 *                   |_________|
 *
 *            [PC1]-----[PC1  PD1] (Internal resistor)
 *                   |_________|
 *
 */

#include "ch32v003fun.h"
#include <stdio.h>

/* I2C Mode Definition */
#define HOST_MODE 0
#define SLAVE_MODE 1

/* I2C Communication Mode Selection */
//#define I2C_MODE   HOST_MODE
#define I2C_MODE 1

/* Global define */
#define Size 6
#define RXAdderss 0x02
#define TxAdderss 0x02

/* Global Variable */
volatile uint8_t TxData[Size] = {0x10,0x11,0x12,0x13,0x14,0x15};
volatile uint8_t RxData[5][Size];

void IIC_Init(uint32_t bound, uint16_t address) {
    // Enable clocks for GPIOC and I2C1
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

    // Configure PC2 and PC1 as open-drain
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 2);
    GPIOC->CFGLR &= ~GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);
    GPIOC->CFGLR |= GPIO_CFGLR_OUT_50Mhz_AF_OD << (4 * 1);

    // Configure I2C1
    I2C1->CTLR2 = (FUNCONF_SYSTEM_CORE_CLOCK / 1000000); // PCLK1 frequency in MHz
    I2C1->CKCFGR = (bound / (2 * FUNCONF_SYSTEM_CORE_CLOCK)) & 0xFFF; // Set clock speed
    I2C1->OADDR1 = I2C_AcknowledgedAddress_7bit | address; // Set own address
    I2C1->OADDR2 = 0;
    /* DO NOT SWAP THESE TWO ORDER !!!!!!!!!!!!!
    This bit is set and cleared by
    software and cleared by hardware when start is
    sent or PE=0.*/
    I2C1->CTLR1 |= I2C_CTLR1_PE; // Enable I2C
    I2C1->CTLR1 |= I2C_CTLR1_ACK; // Enable ACK
    printf("\nInitializing I2C...\n");
}

#define I2C_SPEED 80000

uint8_t check_i2c_event(uint32_t event) {
    uint16_t STAR1, STAR2 __attribute__((unused));
    STAR1 = I2C1->STAR1;
    STAR2 = I2C1->STAR2;
    uint32_t status = (STAR1 | (STAR2 << 16)) & 0x00FFFFFF;
    // Check if the event matches the status
    return (status & event) == event;
}

const uint32_t timeout_default = 1000000;
int main(void) {
    SystemInit();
#if (I2C_MODE == HOST_MODE)
    printf("\nIIC Host mode\r\n");
    IIC_Init(I2C_SPEED, TxAdderss);

    for (uint8_t j = 0; j < 5; j++) {
        uint32_t timeout = timeout_default;
        while (I2C1->STAR1 & I2C_STAR2_BUSY) {
            if (--timeout == 0) {
                IIC_Init(I2C_SPEED, TxAdderss);
                NVIC_SystemReset();
            }
        }

        I2C1->CTLR1 |= I2C_CTLR1_START;
        printf("I2C generated start!\n");

        timeout = timeout_default;
        while (!check_i2c_event(I2C_EVENT_MASTER_MODE_SELECT)) {
            if (--timeout == 0) {
                IIC_Init(I2C_SPEED, TxAdderss);
                NVIC_SystemReset();
            }
        }
        printf("I2C Event master mode selected!\n");

        I2C1->DATAR = RXAdderss;
        printf("I2C Write 7-bit Address\n");
        timeout = timeout_default;
        while (!check_i2c_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
            if (--timeout == 0) {
                IIC_Init(I2C_SPEED, TxAdderss);
                NVIC_SystemReset();
            }
        }
        printf("I2C Sent 7-bit Address\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        for (uint8_t i = 0; i < 6; i++) {
            timeout = timeout_default;
            while (!check_i2c_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) {
                if (--timeout == 0) {
                    IIC_Init(I2C_SPEED, TxAdderss);
                    NVIC_SystemReset();
                }
            }
            printf("Sent %d\r\n", TxData[i]);
            I2C1->DATAR = TxData[i];
        }
        printf("Sending finished!\r\n");

        timeout = timeout_default;
        while (!check_i2c_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if (--timeout == 0) {
                IIC_Init(I2C_SPEED, TxAdderss);
                NVIC_SystemReset();
            }
        }
        printf("I2C Event master byte transmitted!\r\n");
        
        I2C1->CTLR1 |= I2C_CTLR1_STOP;
        printf("I2C Generated STOP!\r\n");
    }

#elif (I2C_MODE == SLAVE_MODE)
    printf("IIC Slave mode\r\n");
    IIC_Init(I2C_SPEED, RXAdderss);

    for (uint8_t p = 0; p < 5; p++) {
        uint8_t i = 0;
        printf("Waiting for receiver address match!\n");
        uint32_t timeout = timeout_default;
        while (!check_i2c_event(I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED)) {
            if (--timeout == 0) {
                IIC_Init(I2C_SPEED, RXAdderss);
                NVIC_SystemReset();
            }
        }
        printf("I2C Event slave receiver address matched!\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        while (i < 6) {
            timeout = timeout_default;
            while (!check_i2c_event(I2C_EVENT_SLAVE_BYTE_RECEIVED)) {
                if (--timeout == 0) {
                    IIC_Init(I2C_SPEED, RXAdderss);
                    NVIC_SystemReset();
                }
            }
            RxData[p][i] = I2C1->DATAR;
            printf("Received! %d\r\n", RxData[p][i]);
            i++;
        }
        I2C1->CTLR1 &= I2C1->CTLR1;
    }
    printf("RxData:\r\n");
    for (uint8_t p = 0; p < 5; p++) {
        for (uint8_t i = 0; i < 6; i++) {
            printf("%02x ", RxData[p][i]);
        }
        printf("\r\n");
    }

#endif
    NVIC_SystemReset();
}