# InspireMatrix + CH32V003

## Description

Projects:

* `ch32v003_stt`
  * Simple spoken digit recognition.
  * Originally from <https://github.com/brian-smith-github/ch32v003_stt>

* `ch32v003fun`
  * `driver.h`: Contains the most frequently used functions for the CH32V003.
  * `i2c_events.`: Contains some frequently used I2C functions.
  * Originally from <https://github.com/cnlohr/ch32v003fun>

* `data`
  * `buttons.h`: Button ADC calibration data.
  * `colors.h`: RGB color data for WS2812B LEDs.
  * `fonts.h`: Display numbers and characters in the size of 3x5 on WS2812B LEDs.
  * `music.h`: Frequencies, durations and functions for playing music using a buzzer.

* `emulator`
  * Support development of basic embedded system software on Windows/MacOS without requiring
  physical hardware.
  * Aims to achieve function compatibility with the `ch32v003fun` library.

* `i2c-comm` (Working in progress)
  * Communication between two boards using I2C.

* `misc`
  * `libgcc.a` required by the `ch32v003fun` library on MacOS. See [here](misc/README.md) for more information.

* `movingnum`
  * Animations of numbers moving from right to left, bottom to top. Uses math instead of hard coding.

* `paint-cursor`
  * Draw images on `InspireMatrix` with direction buttons and matrix buttons
  * Connection:
    * PD2: SW (Analog to Digital PIN)
    * PC6: IN (WS2812B Data PIN)

* `paint-exe`
  * Paint on `InspireMatrix`

* `rv`
  * Coding RISC-V Compressed instructions on a board with buttons, and showing the result on the matrix.
  * Originally from <https://github.com/brian-smith-github/rv>

* `rv-dis` (Working in progress)
  * Disassemble RISC-V compressed instructions and print the result on an external OLED display (e.g. SSD1306).
  * Originally from <https://github.com/michaeljclark/riscv-disassembler>

* `testing`
  * This folder contains a bunch of test programs that are subject to change, and are not guaranteed to work for
  your specific hardware.

* `tic-tac-toe`
  * Play tic-tac-toe with a bot
  * Press button to start
  * Green is player while Red is the bot
  * Try to win it (very easy)

## Compilation

Two options are available for compilation:

* `make`
  * Compile and output `*.hex` and `*.bin` files which can be used for flashing the program into
  CH32V003.

* `make emulator`
  * Compile an executable using `GCC` which runs natively on your operating system, simulating behaviour on real hardware.

## Credits

Great thanks to these projects (LICENSE included):

* <https://github.com/cnlohr/ch32v003fun>
* <https://github.com/brian-smith-github/ch32v003_stt>
* <https://github.com/mnurzia/rv>

## Check out our other projects as well

* <https://github.com/eric15342335/inspirelab-game>
  * A game console based on the `CH32V003J4M6` MCU.
  * Originally from <https://github.com/wagiminator/CH32V003-GameConsole>

* <https://github.com/eric15342335/BitNetMCU>
  * Receives image data via UART and predict the digit using an ML model.
  * Originally from <https://github.com/cpldcpu/BitNetMCU>
