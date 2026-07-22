#include "stm32f4xx.h"

int main(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3UL << (3 * 2));
    GPIOA->MODER |=  (1UL << (3 * 2)); // PA3 Output

    while (1) {
        GPIOA->BSRR = (1 << 3);        // High
        GPIOA->BSRR = (1 << (3 + 16)); // Low
    }
}