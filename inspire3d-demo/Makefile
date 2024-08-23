TARGET:=inspire3d-demo
include ../ch32v003fun/ch32v003fun.mk
clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).bin $(TARGET).map $(TARGET).lst \
		$(TARGET).o $(TARGET).d $(TARGET).srec $(TARGET).sym $(TARGET).lss
	$(MAKE) -f Makefile-emulator clean

emulator:
	$(MAKE) -f Makefile-emulator clean
	$(MAKE) -f Makefile-emulator 

.PHONY: emulator clean
