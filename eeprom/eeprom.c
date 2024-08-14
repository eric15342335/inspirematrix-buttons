#define CH32V003_I2C_IMPLEMENTATION
#include <stdio.h>
#include "ch32v003_i2c.h"
// obtained from i2c_scan(), before shifting by 1 bit
#define EEPROM_ADDR 0x51

/**
 * @brief AT24C256 (EEPROM) forbids sequential write across multiple pages (one page = 64 bytes)
 * This is a wrapper function for the original i2c_write() in ch32v003_i2c.h
 */
i2c_result_e i2c_write_pages(uint16_t devAddr, uint16_t regAddr, i2c_regAddr_bytes_e regAddrBytes, uint8_t *data, uint8_t sz);

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
        #define addr_end 64
        #define reg_size (addr_end - addr_begin + 1)
        #define matrix_hori (reg_size / 6)

        // write
        uint8_t data[reg_size];
        for (uint16_t i = 0; i < reg_size; i++) {
            data[i] = i;
        }
        result =
            i2c_write_pages(EEPROM_ADDR, addr_begin, I2C_REGADDR_2B, data, reg_size);
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


i2c_result_e i2c_write_pages(uint16_t devAddr, uint16_t regAddr, i2c_regAddr_bytes_e regAddrBytes, uint8_t *data, uint8_t sz) {
    i2c_result_e result = I2C_RESULT_OK;
    uint8_t *data_ptr = data;
    uint8_t *data_end = data + sz;
    while (data_ptr < data_end) {
        uint8_t page_sz = data_end - data_ptr;
        if (page_sz > 64) {
            page_sz = 64;
        }
        result = i2c_write(devAddr, regAddr, regAddrBytes, data_ptr, page_sz);
        if (result != I2C_RESULT_OK) {
            break;
        }
        data_ptr += page_sz;
        regAddr += page_sz;
        //
        Delay_Ms(5);
    }
    return result;
}
