/**
 * @file
 * Implements FLASH for stm32g0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_H
#define ZHELE_FLASH_H

#include <stm32g0xx.h>

#include "../common/flash.h"

namespace Zhele
{
    const static uint32_t MaxFlashFrequence = 24000000;
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t ws = (frequence - 1) / MaxFlashFrequence;
        if(ws > 2)
            ws = 2;
        FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | ws;
    }
}

#endif //! ZHELE_FLASH_H