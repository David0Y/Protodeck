#ifndef CMSIS_PIN_CONFIG_H
#define CMSIS_PIN_CONFIG_H

#include "stm32f4xx.h"
//gpio config input is missing
// Macro to configure a pin as a General Purpose Output
#define gpio_config_output(port, pin) \
    do { \
        GPIO##port->MODER &= ~(3UL << ((pin) * 2)); \
        GPIO##port->MODER |=  (1UL << ((pin) * 2)); \
    } while(0)

// Macros to drive pins HIGH or LOW instantly using BSRR
#define gpio_high(port, pin)   GPIO##port->BSRR = (1UL << (pin))
#define gpio_low(port, pin)    GPIO##port->BSRR = (1UL << ((pin) + 16))

#endif 