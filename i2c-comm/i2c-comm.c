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
#include "i2c_events.h"
#include "music.h"

#include <stdio.h>

/* I2C Mode Definition */
#define HOST_MODE 0
#define SLAVE_MODE 1

/* I2C Communication Mode Selection */
//#define I2C_MODE   HOST_MODE
#define I2C_MODE HOST_MODE

/* Global define */
#define RXAdderss 0x02
#define TxAdderss 0x02
#define FREQ_SIZE 2
#define DURATION_SIZE 1
/* Global Variable */
uint8_t RxData[FREQ_SIZE + DURATION_SIZE];

int8_t convert_uint8_to_int8(uint8_t data) { return (int8_t)(data - 0x80); }

int16_t convert_two_uint8_to_int16(uint8_t first, uint8_t two) {
    return (int16_t)((first << 8) | two) - 0x8000;
}

uint8_t convert_int8_to_uint8(int8_t data) { return (uint8_t)(data + 0x80); }

void convert_int16_to_two_uint8(int16_t data, uint8_t * first, uint8_t * two) {
    *first = (uint8_t)((data >> 8) + 0x80);
    *two = (uint8_t)(data & 0xFF);
}

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

int main(void) {
    SystemInit();
#if (I2C_MODE == HOST_MODE)
    printf("IIC Host mode\r\n");
    IIC_Init(TxAdderss);

    for (uint8_t j = 0; j < 5; j++) {
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

        I2C1->DATAR = RXAdderss;
        printf("I2C Write 7-bit Address\n");

        wait_for_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
        printf("I2C Sent 7-bit Address\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        for (int i = 0; i < notes; i++) {
            uint8_t notes_properties[2] = {0, 0};
            convert_int16_to_two_uint8(melody[i * 2], &notes_properties[0], &notes_properties[1]);

            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = notes_properties[0];

            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = notes_properties[1];

            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = convert_int8_to_uint8(melody[i * 2 + 1]);
            printf("Sent [%d] with %d and %d\r\n", i, melody[i * 2], melody[i * 2 + 1]);
        }
        printf("Sending finished!\r\n");

        wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
        printf("I2C Event master byte transmitted!\r\n");

        I2C1->CTLR1 |= I2C_CTLR1_STOP;
        printf("I2C Generated STOP!\r\n");
    }

#elif (I2C_MODE == SLAVE_MODE)
    printf("IIC Slave mode\r\n");
    IIC_Init(RXAdderss);

    while (1) {
        printf("Waiting for receiver address match!\n");
        wait_for_event(I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED);
        printf("I2C Event slave receiver address matched!\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        while (1) {
            uint8_t i = 0;
            while (i < 3) {
                wait_for_event(I2C_EVENT_SLAVE_BYTE_RECEIVED);
                RxData[i] = I2C1->DATAR;
                printf("Received! %d\n", RxData[i]);
                i++;
            }
            int16_t _note = convert_two_uint8_to_int16(RxData[0], RxData[1]);
            int8_t _duration = convert_uint8_to_int8(RxData[2]);
            printf("Note: %d, Duration: %d\n", _note, _duration);
            // Delay_Ms(convertDuration(_duration));
            JOY_sound(_note, convertDuration(_duration));
            Delay_Ms(60 + convertDuration(_duration)*0.1);
            I2C1->CTLR1 &= I2C1->CTLR1;
        }
    }

#endif
    NVIC_SystemReset();
}