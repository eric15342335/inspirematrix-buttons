#define CH32V003_I2C_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

int main(void) {
    SystemInit();
    i2c_init();
    uint8_t found = 0;
    i2c_result_e result = i2c_ping(EEPROM_ADDR, &found);
    
    printf("\nping result: %d\n", result);
    printf("found: %d\n", found);
    i2c_scan();
    Delay_Ms(1000);
    while (1) {

        #define addr_begin 0
        #define addr_end 0x39F
        #define reg_size (addr_end - addr_begin + 1)
        #define matrix_hori 16
        // write
        {
            uint8_t data[reg_size] = {0};
            for (uint16_t i = 0; i < reg_size; i++) {
                data[i] = i % 0xFF;
            }
            result =
                i2c_write_pages(EEPROM_ADDR, addr_begin, I2C_REGADDR_2B, data, reg_size);
            printf("write result: %d\n", result);
            printf("Written data as matrix:\n");
            for (uint16_t i = 0; i < reg_size; i++) {
                printf("%02X ", data[i]);
                if ((i + 1) % matrix_hori == 0) {
                    printf("\n");
                }
            }
            printf("\n");
            Delay_Ms(1000);
        }
        // read
        {
            uint8_t read_data[reg_size] = {0};
            result =
                i2c_read_pages(EEPROM_ADDR, addr_begin, I2C_REGADDR_2B, read_data, reg_size);
            printf("read result: %d\n", result);
            printf("Read data as matrix:\n");
            for (uint16_t i = 0; i < reg_size; i++) {
                printf("%02X ", read_data[i]);
                if ((i + 1) % matrix_hori == 0) {
                    printf("\n");
                }
            }
            printf("\n");
            Delay_Ms(1000);
        }
    }
}
