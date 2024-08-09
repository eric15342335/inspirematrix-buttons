# Run make on all game directories
default: all-emulator

all-emulator:
	$(MAKE) emulator -C rv
	$(MAKE) emulator -C tic-tac-toe
	$(MAKE) emulator -C paint-cursor
	$(MAKE) emulator -C movingnum
	$(MAKE) emulator -C paint-exe

all-riscv:
	$(MAKE) -C rv
	$(MAKE) -C tic-tac-toe
	$(MAKE) -C paint-cursor
	$(MAKE) -C i2c-comm
	$(MAKE) -C movingnum
	$(MAKE) -C paint-exe
	$(MAKE) -C testing

.PHONY: all-emulator all-riscv
