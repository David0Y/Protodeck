#include "stm32f4xx.h"

//this cpp file is for bitbanging every pin as output, even the input ones for testing purposes
// CSN (I and II), SCK (I and II), IRQ (I and II)
//unlike the first test main file a bare metal approach is used here for maximum speed efficiency, so the pins drive high and low as closely together as possible
//for the speed cycle 50khz is targeted at with 150 cycles of delay between each pin toggle, which is 20us per cycle



int main(void) {
    // 1. Enable clocks for GPIOA, GPIOB, and GPIOC (for the onboard LED)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // 2. Configure PC13 (Onboard LED) as general-purpose output (01)
    GPIOC->MODER &= ~(3UL << (13 * 2)); // Clear bits 26 & 27
    GPIOC->MODER |=  (1UL << (13 * 2)); // Set bit 26

    // 3. Configure PA2 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (2 * 2)); // Clear bits 4 & 5
    GPIOA->MODER |=  (1UL << (2 * 2)); // Set bit 4

     // 3. Configure PA3 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (3 * 2)); // Clear bits 6 & 7
    GPIOA->MODER |=  (1UL << (3 * 2)); // Set bit 6

    // 4. Configure PA4 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (4 * 2)); // Clear bits 12 & 13
    GPIOA->MODER |=  (1UL << (4 * 2)); // Set bit 12

    // 5. Configure PA5 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (5 * 2)); // Clear bits 10 & 11
    GPIOA->MODER |=  (1UL << (5 * 2)); // Set bit 10

     // 4. Configure PA6 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (6 * 2)); // Clear bits 12 & 13
    GPIOA->MODER |=  (1UL << (6 * 2)); // Set bit 12

    // 5. Configure PA7 as general-purpose output (01)
    GPIOA->MODER &= ~(3UL << (7 * 2)); // Clear bits 14 & 15
    GPIOA->MODER |=  (1UL << (7 * 2)); // Set bit 14

    // 6. Configure PB0 as general-purpose output (01)
    GPIOB->MODER &= ~(3UL << (0)); // Clear bits 0 & 1
    GPIOB->MODER |=  (1UL << (0)); // Set bits 0  & 1

    // 6. Configure PB1 as general-purpose output (01)
    GPIOB->MODER &= ~(3UL << (1*2)); // Clear bits 2 & 3
    GPIOB->MODER |=  (1UL << (1*2)); // Set bits 2  & 3
    
    // 6. Configure PB10 as general-purpose output (01)
    GPIOB->MODER &= ~(3UL << (10 * 2)); // Clear bits 20 & 21
    GPIOB->MODER |=  (1UL << (10 * 2)); // Set bit 20

    while (1) {
       
        // Toggle every pin using the Bit Set/Reset Register (BSRR)
        GPIOA->BSRR = (1UL << 2); // Set High
        GPIOA->BSRR = (1UL << 4); // Set High
        GPIOA->BSRR = (1UL << 5); // Set High
        GPIOA->BSRR = (1UL << 6); // Set High
        GPIOA->BSRR = (1UL << 7); // Set High
        GPIOB->BSRR = (1UL << 0); // Set High
        GPIOB->BSRR = (1UL << 1); // Set High
        GPIOB->BSRR = (1UL << 10); // Set High
        //GPIOC->BSRR = (1UL << 13); // Set High, i know leds work inverted, but this is just a heartbeat LED, so it doesn't matter
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < 150; i++); // Short delay 

        GPIOA->BSRR = (1UL << (2 + 16)); // Set Low
        GPIOA->BSRR = (1UL << (4 + 16)); // Set Low
        GPIOA->BSRR = (1UL << (5 + 16)); // Set Low
        GPIOA->BSRR = (1UL << (6 + 16)); // Set Low
        GPIOA->BSRR = (1UL << (7 + 16)); // Set Low
        GPIOB->BSRR = (1UL << (0 + 16)); // Set Low
        GPIOB->BSRR = (1UL << (1 + 16)); // Set Low
        GPIOB->BSRR = (1UL << (10 + 16)); // Set Low
        //GPIOC->BSRR = (1UL << (13 + 16)); // Set Low
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < 150; i++); // Short delay
    }
}