#ifndef SYSCLK_H
#define SYSCLK_H

#include <stdint.h>

// Initializes the system clock to your target frequency (e.g., 72MHz or 84MHz)
// Returns true if initialization succeeded and the PLL locked properly.
bool sysclk_init(void);

// Optional helper to get the current core clock frequency in Hz
uint32_t sysclk_get_freq(void);

#endif // SYSCLK_H