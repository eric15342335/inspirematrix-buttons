# InspireMatrix + CH32V003

## Description

Projects:

* paint-cursor
  * Draw images on InspireMatrix with direction buttons and matrix buttons
  * Connection:
    * PD2: SW (Analog reading)
    * PC6: IN (WS2812B control)

* tic-tac-toe
  * Play tic-tac-toe with bot
  * press button on the matrix to play
  * Green is player while Red is the bot
  * Try to win it (very easy)

* rv
  * Coding RISC-V Compressed instructions on a board with buttons, and showing the result on the matrix.

* movingnum
  * Animations of numbers moving from right to left, bottom to top. Uses math instead of hard coding.

* ch32v003_st
  * Simple spoken digit recognition.

* testing
  * Reading random analog pins and displaying random LED lights. Experimental.

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
* <https://github.com/eric15342335/BitNetMCU>
