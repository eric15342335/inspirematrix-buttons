# InspireMatrix + CH32V003

This repository contains various projects and utilities for
working with the CH32V003 microcontroller.

Older hardware (InspireMatrix):

![InspireMatrix](inspire_matrix.jpeg)

## Project Structure

* `.github`:
  * `workflows`: GitHub Actions workflows.
  * `Doxyfile`: Doxygen configuration.

* `.vscode`:
  * `settings.json`: VSCode settings:

    ```json
    "C_Cpp.default.compilerPath": "riscv-none-elf-gcc",
    ```

  To set the default compiler to `riscv-none-elf-gcc` for IDE integration.

* `ch32v003_stt`
  * Simple spoken digit recognition.
  * Originally from <https://github.com/brian-smith-github/ch32v003_stt>
  * Read its [README.md](ch32v003_stt/STT-README.md) for more information.

* `ch32v003fun`
  * `driver.h`: Contains the most frequently used functions for the CH32V003.
  * `i2c_events.h`: Contains some frequently used I2C functions written manually.
  * `i2c_tx.c`, `i2c_tx.h`, `oled_min.c`, `oled_min.h`: Contains some frequently used functions for the SSD1306 OLED display. Comes from <https://github.com/eric15342335/inspirelab-game>
  * `ws2812b_simple.h`: Contains one function for controlling the WS2812B LEDs.
  You need to declare the following variables in your code:
  In `funconfig.h`:
  
    ```c
    #define FUNCONF_SYSTICK_USE_HCLK 1
    ```

    In your code (e.g. `main.c`):
  
    ```c
    #define WS2812BSIMPLE_IMPLEMENTATION
    // ...
    #include "ws2812b_simple.h"
    ```

  * Originally from the `extralibs` folder in <https://github.com/cnlohr/ch32v003fun>

* `data`
  * `buttons.h`: Button ADC calibration data.

    Contains two sets of data, one for the first prototype
    `InspireMatrix` and one for the second prototype `InspireComputer` (which uses two ADC channels for buttons). If you are using the `InspireComputer`, declare the following in `funconfig.h`:

    ```c
    #define INTERNAL_INSPIRE_MATRIX
    ```

  * `colors.h`: Contains the color palette for the `InspireMatrix` or `InspireComputer`.
  
    One global variable `led_array[]` act as buffer to store the color data to be displayed.
    Provides functions to manipulate the `led_array[]` buffer.

  * `fonts.h`: Display numbers and characters in the size of 3x5 on WS2812B LEDs.

  * `music.h`: Frequencies, durations and functions for playing music using a buzzer.
  To play sound, use `JOY_sound()`.

* `emulator`
  * Support development of basic embedded system software on Windows/MacOS without requiring
  physical hardware.
  * Aims to achieve function compatibility with the `ch32v003fun` library.

* `i2c-comm`
  * Communication between two boards using I2C protocol. The code contains a master and a slave.

* `misc`
  * `libgcc.a` required by the `ch32v003fun` library on MacOS. See [here](misc/README.md) for more information.

* `movingnum`
  * Animations of numbers moving from right to left, bottom to top. Uses math instead of hard coding.
  Demonstrates the use of `fonts.h`.

* `paint-cursor`
  * Draw images on `InspireMatrix` with direction buttons and matrix buttons
  * Connection:
    * PD2: SW (Analog to Digital PIN)
    * PC6: IN (WS2812B Data PIN)

* `paint`
  * Paint on `InspireMatrix`.

* `rv-asm`
  * Coding RISC-V Compressed instructions on a board with buttons, and showing the result on the matrix.
  * Originally from <https://github.com/mnurzia/rv>

* `rv-dis` (Working in progress)
  * Disassemble RISC-V compressed instructions and print the result on an external OLED display (e.g. SSD1306).
  * Originally from <https://github.com/michaeljclark/riscv-disassembler>

* `testing`
  * This folder contains a bunch of test programs that are subject to change, and are not guaranteed to work for
  your specific hardware.

* `savepaint`
  * Todo

* `save-rvasm`
  * Todo

* `snake-game`
  * Porting the classic snake game to `InspireMatrix`, which has `8x8`=`64` LEDs and `up / down / left / right` controls.

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
