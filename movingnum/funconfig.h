#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define CH32V003 1
#define FUNCONF_USE_DEBUGPRINTF 0
#define FUNCONF_USE_UARTPRINTF 1
#define FUNCONF_UART_PRINTF_BAUD 115200
#define FUNCONF_SYSTICK_USE_HCLK 1
#define horizontalButtons 8
#define verticalButtons 24
#define NUM_LEDS (horizontalButtons * verticalButtons)

#endif
