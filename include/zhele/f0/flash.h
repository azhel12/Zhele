/**
 * @file
 * Implements FLASH for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_H
#define ZHELE_FLASH_H

#include <stm32f0xx.h>

#include "../common/flash.h"

#include <algorithm>

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

    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        if (IsLock())
            Unlock();

        const uint16_t* aligned_src = reinterpret_cast<const uint16_t*>(src);
        uint16_t* aligned_dst = reinterpret_cast<uint16_t*>(dst);

        FLASH->CR |= FLASH_CR_PG;

        if ((reinterpret_cast<uint32_t>(aligned_src) & 0x1) == 0) {
            while (size >= sizeof(uint16_t)) {
                *aligned_dst++ = *aligned_src++;
                size -= sizeof(uint16_t);
                WaitWhileBusy();

                if ((FLASH->SR & FLASH_SR_EOP) == 0)
                    break;

                FLASH->SR = FLASH_SR_EOP;
            }
        } else {
            alignas(sizeof(uint16_t)) uint16_t buffer;
            while (size >= sizeof(uint16_t)) {
                std::copy_n(reinterpret_cast<const uint8_t*>(aligned_src), sizeof(buffer), reinterpret_cast<uint8_t*>(&buffer));

                *aligned_dst++ = buffer;
                ++aligned_src;
                size -= sizeof(uint16_t);

                WaitWhileBusy();

                if ((FLASH->SR & FLASH_SR_EOP) == 0)
                    break;

                FLASH->SR = FLASH_SR_EOP;
            }
        }

        if (size > 0) {
            alignas(sizeof(uint16_t)) uint16_t buffer;

            std::copy_n(reinterpret_cast<const uint8_t*>(aligned_src), size, reinterpret_cast<uint8_t*>(&buffer));
            std::copy_n(reinterpret_cast<const uint8_t*>(aligned_dst) + size, sizeof(buffer) - size, reinterpret_cast<uint8_t*>(&buffer) + size);

            *aligned_dst++ = buffer;
            WaitWhileBusy();

            FLASH->SR = FLASH_SR_EOP;
        }

        FLASH->CR &= (~FLASH_CR_PG);
        Lock();

        return (FLASH->SR & (FLASH_SR_WRPRTERR | FLASH_SR_PGERR) == 0);
    }
}

#endif //! ZHELE_FLASH_H