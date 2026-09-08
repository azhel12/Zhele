/**
 * @file
 * CH32V00x flash: 1 KB erase sector, halfword (V003) or fast-page (V00x) programming.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_FLASH_H
#define ZHELE_PLATFORM_CH32_V0_FLASH_H

#include <ch32v00x.h>

#if !defined(FLASH_CTLR_PG)
#  define ZHELE_CH32_FLASH_FAST_PROGRAMMING 1
#endif

#include "../common/flash.h"

namespace Zhele
{
    /// @brief Max flash frequence without wait states
    const static uint32_t MaxFlashFrequence = 24000000;

    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
    #if defined(CH32V00X)
        uint32_t ws = frequence <= 8000000 ? 0 : (frequence <= MaxFlashFrequence ? 1 : 2);
    #else
        uint32_t ws = frequence <= MaxFlashFrequence ? 0 : 1;
    #endif

        FLASH->ACTLR = (FLASH->ACTLR & ~static_cast<uint32_t>(FLASH_ACTLR_LATENCY)) | ws;
    }

    inline constexpr uint32_t Flash::FlashSize()
    {
    #if defined(ZHELE_FLASH_SIZE)
        return ZHELE_FLASH_SIZE;
    #elif defined(CH32V003) || defined(CH32V002)
        return 16 * 1024;
    #elif defined(CH32V004) || defined(CH32V005)
        return 32 * 1024;
    #elif defined(CH32V006) || defined(CH32V007_M007)
        return 62 * 1024;
    #else
        #warning "Zhele: unknown CH32V00x flash size, assuming 16 KB. Define ZHELE_FLASH_SIZE (in bytes)."
        return 16 * 1024;
    #endif
    }

    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
        return 1024;
    }
}

#endif // ZHELE_PLATFORM_CH32_V0_FLASH_H
