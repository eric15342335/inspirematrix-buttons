#include "ch32v003fun.h"
#include "i2c_events.h"
#include "music.h"

#include <stdio.h>

#define TxAdderss 0x03
#define RXAdderss 0x04

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
    printf("IIC Slave mode\r\n");
    IIC_Init(RXAdderss);

    while (1) {
        printf("Waiting for receiver address match!\n");
        wait_for_event(I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED);
        printf("I2C Event slave receiver address matched!\n");

        (void)I2C1->STAR2; // Clear ADDR flag

        while (1) {
            // Receive PCM data
            for (i = 0; i < FFT_SIZE; i++) {
                wait_for_event(I2C_EVENT_SLAVE_BYTE_RECEIVED);
                buffer[i] =
                    I2C1->DATAR; // Assume this function receives 8-bit unsigned PCM data
                re[i] = (int)buffer[i] - 128; // Convert to signed
                im[i] = 0;
            }
            // Perform FFT
            simple_int_fft(FFT_SIZE);

            // Find the dominant frequency
            for (i = 1; i < FFT_SIZE / 2; i++) { // Start from 1 to skip DC component
                int magnitude = re[i] * re[i] + im[i] * im[i];
                if (magnitude > max_magnitude) {
                    max_magnitude = magnitude;
                    dominant_frequency_bin = i;
                }
            }
            // Calculate the actual frequency
            int frequency = dominant_frequency_bin * SAMPLE_RATE / FFT_SIZE;
            // Play the sound
            JOY_sound(frequency, 1);
            // printf("Dominant frequency: %d Hz\n", frequency);
            I2C1->CTLR1 &= I2C1->CTLR1;
        }
    }

    NVIC_SystemReset();
}