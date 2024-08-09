# Run make on all game directories
default: all-emulator

all-emulator:
	$(MAKE) emulator -C rv-asm
	$(MAKE) emulator -C tic-tac-toe
	$(MAKE) emulator -C paint-cursor
	$(MAKE) emulator -C movingnum
	$(MAKE) emulator -C paint

all-riscv:
	$(MAKE) -C rv-asm
	$(MAKE) -C tic-tac-toe
	$(MAKE) -C paint-cursor
	$(MAKE) -C i2c-comm
	$(MAKE) -C movingnum
	$(MAKE) -C paint
	$(MAKE) -C testing

clean:
	$(MAKE) -C rv-asm clean
	$(MAKE) -C tic-tac-toe clean
	$(MAKE) -C paint-cursor clean
	$(MAKE) -C i2c-comm clean
	$(MAKE) -C movingnum clean
	$(MAKE) -C paint clean
	$(MAKE) -C testing clean

.PHONY: all-emulator all-riscv clean
