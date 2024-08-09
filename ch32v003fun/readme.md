# List of custom modifications done to the original repository

## Description

This folder contains the CH32V003fun header and ws2812b_simple header for powering the CH32V003J4M6 CPU
driver.h links up all the headers with extra functions for convinence.

The original link of the driver is in below:
[CH32V003fun](https://github.com/cnlohr/ch32v003fun/tree/master/ch32v003fun)
[ws2812b_simple](https://github.com/cnlohr/ch32v003fun/blob/master/extralibs/ws2812b_simple.h)

## How to use

Most custom modifications are located in `driver.h` and `i2c_events.h`.

### pin connections

* PC2 (3rd pin start from the left) to the IN pin in InspireMatrix
  * Used for ws2812 led toggling
* 5V on InspireMatrix: connect to 3.3v source
* 0 on InspireMatrix: connect to ground
* 6th PIN on J4M6 can connect to WCH-LinkE programmer RX pin for printf() USART,
    by defining `#define CH32V003J4M6_USE_PD6_AS_UART_TX` in funconfig.h (you can search that in `ch32v003fun.c` for what does it do)
