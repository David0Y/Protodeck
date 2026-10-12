#include "stm32f4xx.h"
#include "cmsis_pin_config.h"
#include "sysclk.h"
//this cpp file is for bitbanging every pin as output, even the input ones for testing purposes
// CSN (I and II), SCK (I and II), IRQ (I and II)
//unlike the first test main file a bare metal approach is used here for maximum speed efficiency, so the pins drive high and low as closely together as possible
//for the speed cycle 50khz is targeted at with 150 cycles of delay between each pin toggle, which is 20us per cycle

# define NOP_value 3  //3 is the value thats the closest to 0.5mus

int main(void) {
    sysclk_init();
    

    // 1. Enable clocks for GPIOA, GPIOB, and GPIOC (for the onboard LED)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // 2. Configure PC13 (Onboard LED) as general-purpose output (01)
    gpio_config_output(C, 13);
    

    // 3. Configure PA2 as general-purpose output (01)
    gpio_config_output(A, 2);

    // 3. Configure PA3 as general-purpose output (01)
    gpio_config_output(A, 3);

    
    

    // 4. Configure PA4 as general-purpose output (01)
    gpio_config_output(A, 4);

    // 5. Configure PA5 as general-purpose output (01)
    gpio_config_output(A, 5);

    // 4. Configure PA6 as general-purpose output (01)
    gpio_config_output(A, 6);

    // 5. Configure PA7 as general-purpose output (01)
    gpio_config_output(A, 7);


    // 6. Configure PB0 as general-purpose output (01)
    gpio_config_output(B, 0);

    // 6. Configure PB1 as general-purpose output (01)
    gpio_config_output(B, 1);

    // 6. Configure PB10 as general-purpose output (01)
    gpio_config_output(B, 10);

    while (1) {
       
        // Toggle  every PA PB and PC using the Bit Set/Reset Register (BSRR)
        gpio_high(A, 2);
        gpio_high(A, 3);
        gpio_high(A, 4);
        gpio_high(A, 5);
        gpio_high(A, 6);
        gpio_high(A, 7);
        gpio_high(B, 0);
        gpio_high(B, 1);
        gpio_high(B, 10);
        //gpio_high(C, 13); // Set High, i know leds work inverted, but this is just a heartbeat LED, so it doesn't matter
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay

        gpio_low(A, 2);
        gpio_low(A, 3);
        gpio_low(A, 4);
        gpio_low(A, 5);
        gpio_low(A, 6);
        gpio_low(A, 7);
        gpio_low(B, 0);
        gpio_low(B, 1);
        gpio_low(B, 10);

        //gpio_low(C, 13); // Set Low
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay
    }
}

