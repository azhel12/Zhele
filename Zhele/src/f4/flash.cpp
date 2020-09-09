#if defined (STM32F4)

#include "../include/f4/flash.h"

namespace Zhele
{
    const static uint32_t MaxFlashFrequence = 24000000;
    void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t ws = (frequence - 1) / MaxFlashFrequence;
        if(ws > 7)
            ws = 7;
        FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | ws;
    }
}

#endif