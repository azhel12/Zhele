/**
 * @file
 * CH32V20x flash: 4 KB erase sector, halfword programming.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_FLASH_H
#define ZHELE_PLATFORM_CH32_V2_FLASH_H

#include <ch32v20x.h>

#include "../common/flash.h"

namespace Zhele
{
    /// @note No-op: the V20x flash has no wait states and no ACTLR register.
    inline void Flash::ConfigureFrequence(uint32_t)
    {
    }

    inline constexpr uint32_t Flash::FlashSize()
    {
    #if defined(ZHELE_FLASH_SIZE)
        return ZHELE_FLASH_SIZE;
    #elif defined(CH32V20x_D8) || defined(CH32V20x_D8W)
        return 128 * 1024;
    #else
        return 64 * 1024;
    #endif
    }

    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
        return 4096;
    }
}

#endif // ZHELE_PLATFORM_CH32_V2_FLASH_H
