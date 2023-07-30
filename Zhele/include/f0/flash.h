/**
 * @file
 * Implements FLASH for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_H
#define ZHELE_FLASH_H

#include <stm32f0xx.h>

#include "../common/flash.h"

namespace Zhele
{
    const static uint32_t MaxFlashFrequence = 24000000;
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        FLASH->ACR |= frequence > MaxFlashFrequence
            ? (FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY)
            : FLASH_ACR_PRFTBE;
    }
}

#endif //! ZHELE_FLASH_H