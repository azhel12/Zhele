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
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        static constexpr uint32_t MaxFlashFrequence = 24000000;

        FLASH->ACR |= frequence > MaxFlashFrequence
            ? (FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY)
            : FLASH_ACR_PRFTBE;
    }

    inline constexpr uint32_t Flash::PageSize(unsigned page)
    {
        // TODO:: Implement this method
        return 1024;
    }

    inline constexpr uint32_t Flash::PageCount()
    {
        return FlashSize() / PageSize(0);
    }

    inline constexpr unsigned Flash::AddressToPage(const void* address)
    {
        uint32_t offset = reinterpret_cast<uint32_t>(address) - FLASH_BASE;

        return offset / PageSize(0);
    }

    inline bool Flash::ErasePage(uint32_t page)
    {
        if (page >= PageCount())
            return false;

        if (IsLock())
            Unlock();

        WaitWhileBusy();

        FLASH->CR |= FLASH_CR_PER;
        FLASH->AR = PageAddress(page);
        FLASH->CR |= FLASH_CR_STRT;

        __asm("nop"); // The software should start checking if the BSY bit equals “0” at least one CPU cycle after setting the STRT bit.

        WaitWhileBusy();

        if ((FLASH->SR & FLASH_SR_EOP) == 0) {
            FLASH->CR &= (~FLASH_CR_PER);
            return false;
        }
        
        FLASH->CR &= (~FLASH_CR_PER);
        FLASH->SR = FLASH_SR_EOP;

        return true;
    }

    inline bool Flash::WritePage(unsigned page, const void* data, unsigned size, unsigned offset)
    {
        if(page > PageCount())
            return false;
        
        if(offset + size > PageSize(page))
            return false;

        if (IsLock())
            Unlock();

        const uint16_t* src = reinterpret_cast<const uint16_t*>(data);
        uint16_t* dst = reinterpret_cast<uint16_t*>(PageAddress(page) + offset);

        FLASH->CR |= FLASH_CR_PG;

        for(int i = 0; i < size / 2; ++i)
        {
            dst[i] = src[i];
            WaitWhileBusy();

            if ((FLASH->SR & FLASH_SR_EOP) == 0)
                break;

            FLASH->SR = FLASH_SR_EOP;
        }

        FLASH->CR &= (~FLASH_CR_PG);
        Lock();

        return (FLASH->SR & (FLASH_SR_WRPRTERR | FLASH_SR_PGERR) == 0);
    }
}

#endif //! ZHELE_FLASH_H