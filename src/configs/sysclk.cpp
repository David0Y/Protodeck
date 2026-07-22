#include "sysclk.h"
#include "stm32f4xx.h"


uint32_t sysclk_get_freq(void) {
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

bool sysclk_init(void) {
    // 1. Enable PWR clock and set Power Scale 2 (default for <= 84MHz)
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    
    // 2. Enable HSE (External Crystal) and wait until ready
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // 3. Configure Flash latency BEFORE speeding up the clock
    // 2 Wait States required for 84MHz at 3.3V
    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_2WS;

    // 4. Configure Bus Prescalers
    // HCLK = SYSCLK (84MHz), PCLK1 = HCLK/2 (42MHz), PCLK2 = HCLK/1 (84MHz)
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;

    // 5. Configure PLL: Main PLL clock source = HSE
    // M = 25, N = 168, P = 2 (PLLP = 00)
    // VCO in = 25MHz / 25 = 1MHz | VCO out = 1MHz * 168 = 168MHz | SYSCLK = 168MHz / 2 = 84MHz
    RCC->PLLCFGR = (25 << RCC_PLLCFGR_PLLM_Pos)   |
                   (168 << RCC_PLLCFGR_PLLN_Pos)  |
                   (0 << RCC_PLLCFGR_PLLP_Pos)    | // 00 means /2
                   RCC_PLLCFGR_PLLSRC_HSE;

    // 6. Enable PLL and wait for lock
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // 7. Select PLL as System Clock Source
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL);
}