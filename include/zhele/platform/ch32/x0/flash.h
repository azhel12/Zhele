/**
 * @file
 * @brief Implements flash for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_FLASH_H
#define ZHELE_PLATFORM_CH32_X0_FLASH_H

#include <ch32x035.h>

#define ZHELE_CH32_FLASH_FAST_PROGRAMMING 1

#include "../common/flash.h"

namespace Zhele
{
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t latency = frequence <= 12000000 ? 0 : (frequence <= 24000000 ? 1 : 2);

        FLASH->ACTLR = (FLASH->ACTLR & ~static_cast<uint32_t>(FLASH_ACTLR_LATENCY)) | latency;
    }

    inline constexpr uint32_t Flash::FlashSize()
    {
#if defined(ZHELE_FLASH_SIZE)
        return ZHELE_FLASH_SIZE;
#else
        return 62 * 1024;
#endif
    }

    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
        return 1024;
    }
}

#endif // ZHELE_PLATFORM_CH32_X0_FLASH_H
