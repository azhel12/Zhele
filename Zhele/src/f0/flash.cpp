#if defined (STM32F0)

#include "../include/f0/flash.h"

namespace Zhele
{
    const static uint32_t MaxFlashFrequence = 24000000;
    void Flash::ConfigureFrequence(uint32_t frequence)
    {
        FLASH->ACR |= frequence > MaxFlashFrequence
            ? (FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY)
            : FLASH_ACR_PRFTBE;
    }
}
#endif