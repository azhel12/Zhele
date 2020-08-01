/**
 * @file
 * Implements FLASH for stm32f1 series
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_H
#define ZHELE_FLASH_H

#include <stm32f1xx.h>

#include "../common/flash.h"

namespace Zhele
{
    const static uint32_t MaxFlashFrequence = 24000000;
    void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t ws = (frequence - 1) / MaxFlashFrequence;
        if(ws > 7)
            ws = 7;
        FLASH->ACR |= FLASH_ACR_PRFTBE | ws;
    }
}

#endif //! ZHELE_FLASH_H