#include "stm32f4xx.h"
#include "cmsis_pin_config.h"
#include "sysclk.h"

# define NOP_value 3  //3 is the value thats the closest to 0.5mus
//# define NOP_value 40   //40 is the value that is closest to 5 mus
//this cpp file is for crisscross bitbanging in following sequence for each female header pin bulk:
// CE (I and II), MOSI (I and II), MISO (I and II)
//unlike the first test main file a bare metal approach is used here for maximum speed efficiency, so the pins drive high and low as closely together as possible
//for the speed cycle 50khz is targeted at with 150 cycles of delay between each pin toggle, which is 20us per cycle



int main(void) {

    sysclk_init();
    

    // 1. Enable clocks for GPIOA, GPIOB, and GPIOC (for the onboard LED)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // 2. Configure PC13 (Onboard LED) as general-purpose output (01)
    gpio_config_output(C, 13);
    

    // 3. Configure PA3 as general-purpose output (01)
    gpio_config_output(A, 3);

    // 4. Configure PA6 as general-purpose output (01)
    gpio_config_output(A, 6);

    // 5. Configure PA7 as general-purpose output (01)
    gpio_config_output(A, 7);

    // 6. Configure PB10 as general-purpose output (01)
    gpio_config_output(B, 10);

    while (1) {
       
        // Toggle PA3, PA6, PA7, PB10 and PC13 using the Bit Set/Reset Register (BSRR)
        gpio_high(A, 3);
        gpio_high(A, 6);
        gpio_high(A, 7);
        gpio_high(B, 10);
        //gpio_high(C, 13); // Set High, i know leds work inverted, but this is just a heartbeat LED, so it doesn't matter
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay

        gpio_low(A, 3);
        gpio_low(A, 6);
        gpio_low(A, 7);
        gpio_low(B, 10);
        //gpio_low(C, 13); // Set Low
        //for (volatile int i = 0; i < 500000; i++); // Delay
        //for (volatile int i = 0; i < 4000000; i++); // Huge delay
        for (volatile int i = 0; i < NOP_value; i++); // Short delay
    }
}