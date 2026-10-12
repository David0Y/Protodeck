#include "stm32f4xx.h"
#include "cmsis_pin_config.h"
#include "sysclk.h"

# define NOP_value 3   //3 is the value that is closest to 0.5 mus

//this cpp file is for crisscross bitbanging in following sequence for each female header pin bulk:
// CSN (I and II), SCK (I and II), IRQ (I and II)
//unlike the first testmain file a bare metal approach is used here for maximum speed efficiency, so the pins drive high and low as closely together as possible
//for the speed cycle 50khz is targeted at with 150 cycles of delay between each pin toggle, which is 20us per cycle

//A2 is IRQ2,A4 is CSN2,A5 is SCK1/2,PB1 is IRQ1, PB0 is CSN1 

int main(void) {
  
  sysclk_init();
    

    // 1. Enable clocks for GPIOA, GPIOB, and GPIOC (for the onboard LED)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // 2. Configure PC13 (Onboard LED) as general-purpose output (01)
    gpio_config_output(C, 13);
    

    // 3. Configure PA2 as general-purpose output (01)
    gpio_config_output(A, 2);

    // 4. Configure PA4 as general-purpose output (01)
    gpio_config_output(A, 4);

    // 5. Configure PA5 as general-purpose output (01)
    gpio_config_output(A, 5);

    // 6. Configure PB0 as general-purpose output (01)
    gpio_config_output(B, 0);

    // 6. Configure PB1 as general-purpose output (01)
    gpio_config_output(B, 1);

    while (1) {
       
        // Toggle PA2, PA4, PA5, PB0, PB1 and PC13 using the Bit Set/Reset Register (BSRR)
        gpio_high(A, 2);
        gpio_high(A, 4);
        gpio_high(A, 5);
        gpio_high(B, 0);
        gpio_high(B, 1);
        //gpio_high(C, 13); // Set High, i know leds work inverted, but this is just a heartbeat LED, so it doesn't matter
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay

        gpio_low(A, 2);
        gpio_low(A, 4);
        gpio_low(A, 5);
        gpio_low(B, 0);
        gpio_low(B, 1);
        //gpio_low(C, 13); // Set Low
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay
    }
}