/**
 * @file
 * CH32V00x flash: 1 KB erase sector, halfword or fast-page programming.
 *
 * @details CH32V003 keeps the classic standard programming mode (`CTLR.PG`);
 * the newer V00x parts (V002/V004/V005/V006/V007) dropped it and can only be
 * programmed through the 256-byte fast page buffer, so the write path is
 * selected here. Erase is the 1 KB `CTLR.PER` sector erase on both.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_FLASH_H
#define ZHELE_PLATFORM_CH32_V0_FLASH_H

#include <ch32v00x.h>

// Standard (halfword) programming is gone on the new V00x parts: no CTLR.PG.
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
        // V00x: 0 WS up to 8 MHz, 1 WS up to 24 MHz, 2 WS above (RM, FLASH_ACTLR).
        uint32_t ws = frequence <= 8000000 ? 0 : (frequence <= MaxFlashFrequence ? 1 : 2);
    #else
        // V003: 0 WS up to 24 MHz, 1 WS above.
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
        // No chip macro to go by (only the family one). 16 KB is the smallest V0
        // part, so page bounds stay conservative until the consumer says otherwise.
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
