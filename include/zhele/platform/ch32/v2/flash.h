/**
 * @file
 * CH32V20x flash: 4 KB erase sector, halfword programming.
 *
 * @details The V20x FPEC keeps the standard programming mode (`CTLR.PG`), but
 * unlike the V0 parts its `CTLR.PER` erases a 4 KB sector, not 1 KB.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_FLASH_H
#define ZHELE_PLATFORM_CH32_V2_FLASH_H

#include <ch32v20x.h>

#include "../common/flash.h"

namespace Zhele
{
    inline void Flash::ConfigureFrequence(uint32_t)
    {
        // Nothing to do: the V20x flash has no wait states. Its register map
        // starts at KEYR — there is no ACTLR (see the CH32V203/CH32V208 SVD, and
        // note that the WCH CMSIS header declares no bits for it either, while
        // the ACTLR field it does declare in the struct always reads back 0).
    }

    inline constexpr uint32_t Flash::FlashSize()
    {
    #if defined(ZHELE_FLASH_SIZE)
        return ZHELE_FLASH_SIZE;
    #elif defined(CH32V20x_D8) || defined(CH32V20x_D8W)
        return 128 * 1024;
    #else
        // D6 line (CH32V203x6 / x8). Override with ZHELE_FLASH_SIZE for smaller parts.
        return 64 * 1024;
    #endif
    }

    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
        return 4096;
    }
}

#endif // ZHELE_PLATFORM_CH32_V2_FLASH_H
