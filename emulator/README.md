# Description

* Windows/MacOS port for OLED/Buzzer/keyboard functions
* 'Emulator' folder is for compiling Windows and MacOs version

## How to use

* Type `make emulator` for compiling the emulator version on Windows/MacOs
* Run the executable

## Controls

press keyboard to control the matrix emulator, listing out the keys

* I -> up
* J -> left
* K -> down
* L -> right
* P -> ACT
* U -> XKey
* O -> YKey

for the buttons on the matrix, it follows the coding writting on the circuit board
![circuit board](/inspire_matrix.jpeg)

first press ```ABCD``` on button to choose row, the press ```0-F``` to choose the specific button in HEX

## Notes for emulator

* Highly recommended to use dark theme terminal for best user experience
* _OLED_refresh_display() is needed for emulators to print out the monitor compare to the RISC-V version
* For Windows, Disable "Device default effects" before using the `JOY_sound()` function.
* For MacOS version, go to settings -> Privacy & Security -> Accessibility and allow terminal/your IDE to control your computer to enable keyboard control (will pop up during first run, need to restart the program after enable)
