#pragma once
#include <stdint.h>

#define GPIOA ((GPIO_TypeDef *)0)
#define GPIOC ((GPIO_TypeDef *)0)
#define GPIOD ((GPIO_TypeDef *)0)
#define __IO volatile
typedef struct {
    __IO uint32_t CFGLR;
    __IO uint32_t CFGHR;
    __IO uint32_t INDR;
    __IO uint32_t OUTDR;
    __IO uint32_t BSHR;
    __IO uint32_t BCR;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

#define NVIC_SystemReset() // suppose to restart the program, unimplemented
