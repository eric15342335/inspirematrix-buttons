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
#define I2C_MODE SLAVE_MODE

/* Global define */
#define Size 6
#define RXAdderss 0x02
#define TxAdderss 0x02

/* Global Variable */
volatile uint8_t TxData[Size] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
volatile uint8_t RxData[5][Size];

/*********************************************************************
 * @fn      IIC_Init
 *
 * @brief   Initializes the IIC peripheral.
 *
 * @return  none
 */
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
    I2C1->OADDR1 = address << 1; // Set own address
    I2C1->CTLR1 = I2C_CTLR1_ACK; // Enable ACK
    I2C1->CTLR1 |= I2C_CTLR1_PE; // Enable I2C
}

#define I2C_SPEED 80000

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void) {

    SystemInit();

    Delay_Ms(1000);

#if (I2C_MODE == HOST_MODE)
    printf("IIC Host mode\r\n");
    IIC_Init(I2C_SPEED, TxAdderss);

    for (uint8_t j = 0; j < 5; j++) {
        while (I2C1->STAR1 & I2C_STAR2_BUSY);

        I2C1->CTLR1 |= I2C_CTLR1_START;
        printf("I2C generated start!\n");

        while (!(I2C1->STAR1 & I2C_STAR1_SB));
        printf("I2C Event master mode selected!\n");

        // the 8th-bit indicates direction of the transfer
        // 0: Master sends data to Slave
        // 1: Master receives data from Slave
        I2C1->DATAR = RXAdderss;
        printf("I2C Write 7-bit Address\n");
        while (!(I2C1->STAR1 & I2C_STAR1_ADDR));
        printf("I2C Sent 7-bit Address\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        for (uint8_t i = 0; i < 6; i++) {
            while (!(I2C1->STAR1 & I2C_STAR1_TXE));
            Delay_Ms(100);
            printf("Sent %d\r\n", TxData[i]);
            I2C1->DATAR = TxData[i];
        }
        printf("Sending finished!\r\n");

        while (!(I2C1->STAR1 & I2C_STAR1_BTF));
        printf("I2C Event master byte transmitted!\r\n");
        
        I2C1->CTLR1 |= I2C_CTLR1_STOP;
        printf("I2C Generated STOP!\r\n");
        Delay_Ms(1000);
    }

#elif (I2C_MODE == SLAVE_MODE)
    printf("IIC Slave mode\r\n");
    IIC_Init(I2C_SPEED, RXAdderss);

    for (uint8_t p = 0; p < 5; p++) {
        uint8_t i = 0;
        printf("Waiting for receiver address match!\n");
        while () {
            uint16_t flag1 = I2C1->STAR1;
            uint16_t flag2 = I2C1->STAR2;
            flag2 = flag2 << 16;

            uint16_t lastevent = (flag1 | flag2) & 0x00FFFFFF;
            printf("last=%d\n", flag1);
            printf("match=%d\n", I2C_STAR1_ADDR & lastevent);
            if (!(flag1 & I2C_STAR1_ADDR)) {
                break;
            }
            Delay_Ms(100);
        }
        printf("I2C Event slave receiver address matched!\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        while (i < 6) {
            while (!(I2C1->STAR1 & I2C_STAR1_RXNE));
            RxData[p][i] = I2C1->DATAR;
            printf("Received! %d\r\n", RxData[p][i]);
            i++;
        }
    }
    printf("RxData:\r\n");
    for (uint8_t p = 0; p < 5; p++) {
        for (uint8_t i = 0; i < 6; i++) {
            printf("%02x ", RxData[p][i]);
        }
        printf("\r\n");
    }

#endif

    while (1);
}