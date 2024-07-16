# Description

* Windows/MacOS port for OLED/Buzzer/keyboard functions
* 'Emulator' folder is for compiling Windows and MacOs version

## Notes for emulator

* Highly recommended to use dark theme terminal for best user experience
* _OLED_refresh_display() is needed for emulators to print out the monitor compare to the RISC-V version
* For Windows, Disable "Device default effects" before using the `JOY_sound()` function.
* For MacOS version, go to settings -> Privacy & Security -> Accessibility and allow terminal/your IDE to control your computer to enable keyboard control (will pop up during first run, need to restart the program after enable)
