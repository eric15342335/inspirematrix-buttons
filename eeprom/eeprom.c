#define CH32V003_I2C_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51
int main(void) {
    while (1) {
        SystemInit();
        i2c_init();
        uint8_t found = 0;
        i2c_result_e result = i2c_ping(EEPROM_ADDR, &found);
        printf("\nping result: %d\n", result);
        printf("found: %d\n", found);
        i2c_scan();
        Delay_Ms(1000);

        #define addr_begin 0
        #define addr_end 64 // warning: sequential write is limited to 64 bytes
        #define reg_size (addr_end - addr_begin + 1) // must <= 64
        #if reg_size > 64
        #pragma warning "reg_size must be <= 64"
        #endif
        #define matrix_hori (reg_size / 6)

        // write
        uint8_t data[reg_size];
        for (uint16_t i = 0; i < reg_size; i++) {
            data[i] = i;
        }
        result =
            i2c_write(EEPROM_ADDR, addr_begin, I2C_REGADDR_2B, data, reg_size);
        printf("write result: %d\n", result);
        printf("Written data as matrix:\n");
        for (int i = 0; i < reg_size; i++) {
            printf("%02X ", data[i]);
            if ((i + 1) % matrix_hori == 0) {
                printf("\n");
            }
        }
        printf("\n");
        Delay_Ms(1000);

        // read
        uint8_t read_data[reg_size] = {0};
        result =
            i2c_read(EEPROM_ADDR, addr_begin, I2C_REGADDR_2B, read_data, reg_size);
        printf("read result: %d\n", result);
        printf("Read data as matrix:\n");
        for (int i = 0; i < reg_size; i++) {
            printf("%02X ", read_data[i]);
            if ((i + 1) % matrix_hori == 0) {
                printf("\n");
            }
        }
        printf("\n");
        Delay_Ms(1000);
    }
}
