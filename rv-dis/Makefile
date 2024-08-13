TARGET:=rv-dis
ADDITIONAL_C_FILES:=riscv-disas.c ../ch32v003fun/i2c_tx.c ../ch32v003fun/oled_min.c
include ../ch32v003fun/ch32v003fun.mk
clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).bin $(TARGET).map $(TARGET).lst \
		$(TARGET).o $(TARGET).d $(TARGET).srec $(TARGET).sym $(TARGET).lss
	$(MAKE) -f Makefile-emulator clean

emulator:
	$(MAKE) -f Makefile-emulator

.PHONY: clean emulator
