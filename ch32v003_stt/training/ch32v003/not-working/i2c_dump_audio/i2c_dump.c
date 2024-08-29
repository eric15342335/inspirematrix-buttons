#include "ch32v003fun.h"
#include "i2c_events.h"

#include <stdio.h>

int result, total = 0, count = 0, b = 0;
// value should be one of 1,2,4,8
#define average_multiplier 1
void init_timer() {
    // TIMER
    printf("Initializing timer...\r\n");
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
    TIM1->CTLR1 |= TIM_CounterMode_Up | TIM_CKD_DIV1;
    TIM1->CTLR2 = TIM_MMS_1;
    TIM1->ATRLR =
        80 / average_multiplier; // lower = higher sample rate. 800 for 6400sam/sec
    TIM1->PSC = 10 - 1;
    TIM1->RPTCR = 0;
    TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    TIM1->INTFR = ~TIM_FLAG_Update;
    TIM1->DMAINTENR |= TIM_IT_Update;
    TIM1->CTLR1 |= TIM_CEN;
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

//-----------------------------------------------------------------------------------
void init_adc() {
    // printf("Initializing ADC... (bjs pin PC4 I think...)\r\n");

    // ADCCLK = 24 MHz => RCC_ADCPRE divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOC and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1;

    // PC4 is analog input chl 2
    GPIOC->CFGLR &= ~(0xf << (4 * 4)); // pin C4 analog-in  now

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Set sequencer to channel 2 only
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 2;

    // set sampling time for chl 7
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 2));
    ADC1->SAMPTR2 |= 7 << (3 * 2); // 0:7 => 3/9/15/30/43/57/73/241 cycles
    // Keep CALVOL register with initial value
    ADC1->CTLR2 = ADC_ADON | ADC_EXTSEL;

    // Reset calibrate
    printf("Calibrating ADC...\r\n");
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL)
        ;
    // Calibrate
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL)
        ;

    printf("Calibrating done...\r\n");
}

#define TxAddress 0x03
#define RxAddress 0x04
//-----------------------------------------------------------------------------------
// Handle the Interrup....

void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
    if (TIM1->INTFR & TIM_FLAG_Update) {
        TIM1->INTFR = ~TIM_FLAG_Update;
        result = ADC1->RDATAR;
        result -= 512;
        total += result;
        count++;
        if (count == 8 * average_multiplier) {
            total >>= 3 + (average_multiplier >> 1);
            total += 127;
            printf("%d\r\n", total);
            wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
            I2C1->DATAR = total;
            // printf("Sending finished!\r\n");

            total = 0;
            count = 0;
        }
        ADC1->CTLR2 |= ADC_SWSTART; // start next ADC conversion
    }
}

//-----------------------------------------------------------------------------------
int main() {
    SystemInit();

    init_adc();

    printf("IIC Host mode\r\n");
    IIC_Init(TxAddress);
    uint32_t timeout = timeout_default;
    while (I2C1->STAR1 & I2C_STAR2_BUSY) {
        if (--timeout == 0) {
            NVIC_SystemReset();
        }
    }

    I2C1->CTLR1 |= I2C_CTLR1_START;

    wait_for_event(I2C_EVENT_MASTER_MODE_SELECT);
    printf("I2C Event master mode selected!\n");

    I2C1->DATAR = RxAddress;
    printf("I2C Write 7-bit Address\n");

    wait_for_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    printf("I2C Sent 7-bit Address\n");

    (void)I2C1->STAR2; // Clear ADDR flag

    init_timer();
    while (1) {
    }
    /*
    wait_for_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    printf("I2C Generated STOP!\r\n");
    */
}
