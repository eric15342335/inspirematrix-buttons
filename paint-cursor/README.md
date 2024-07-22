# paint-cursor

## Description

`paint-cursor.c`:
PIN connection:

* PC2 (3rd pin start from the left) to the IN pin in InspireMatrix
  * Used for ws2812 led toggling
* 5V on InspireMatrix: connect to 3.3v source
* 0 on InspireMatrix: connect to ground
* 6th PIN on J4M6 can connect to WCH-LinkE programmer RX pin for printf() USART,
    by defining `#define CH32V003J4M6_USE_PD6_AS_UART_TX` in funconfig.h (you can search that in `ch32v003fun.c` for what does it do)
