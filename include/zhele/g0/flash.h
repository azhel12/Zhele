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

#include <algorithm>
#include <cstring>

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

    inline constexpr uint32_t Flash::PageSize(unsigned page)
    {
        // TODO:: Implement this method
        return 2048;
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

        FLASH->CR |= FLASH_CR_PER | (page << FLASH_CR_PNB_Pos) | FLASH_CR_EOPIE;
        FLASH->CR |= FLASH_CR_STRT;

        __asm("nop"); // The software should start checking if the BSY bit equals “0” at least one CPU cycle after setting the STRT bit.

        WaitWhileBusy();

        if ((FLASH->SR & FLASH_SR_EOP) == 0) {
            FLASH->CR &= (~FLASH_CR_PER);
            return false;
        }
        
        FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_EOPIE | FLASH_CR_PNB_Msk);
        FLASH->SR = FLASH_SR_EOP;

        return true;
    }

    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        if (IsLock())
            Unlock();

        const uint32_t* aligned_src = reinterpret_cast<const uint32_t*>(src);
        uint32_t* aligned_dst = reinterpret_cast<uint32_t*>(dst);

        FLASH->CR |= FLASH_CR_PG | FLASH_CR_EOPIE;

        if ((reinterpret_cast<uint32_t>(aligned_src) & 0x111) == 0) {
            while (size >= (2 * sizeof(uint32_t))) {
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                size -= (2 * sizeof(uint32_t));
                WaitWhileBusy();

                if ((FLASH->SR & FLASH_SR_EOP) == 0)
                    return false;

                FLASH->SR = FLASH_SR_EOP;
            }
        } else {
            alignas(2 * sizeof(uint32_t)) volatile uint32_t buffer[2];
            while (size >= (2 * sizeof(uint32_t))) {
                for(int i = 0; i < 8; ++i) {
                    reinterpret_cast<volatile uint8_t*>(&buffer[0])[i] = reinterpret_cast<const uint8_t*>(aligned_src)[i];
                }

                *aligned_dst++ = buffer[0];
                *aligned_dst++ = buffer[1];
                aligned_src += 2;

                size -= (2 * sizeof(uint32_t));
                WaitWhileBusy();

                if ((FLASH->SR & FLASH_SR_EOP) == 0) {
                    return false;
                }

                FLASH->SR = FLASH_SR_EOP;
            }
        }

        if (size > 0) {
            alignas(2 * sizeof(uint32_t)) uint32_t buffer[2];

            for(int i = 0; i < size; ++i) {
                reinterpret_cast<volatile uint8_t*>(&buffer[0])[i] = reinterpret_cast<const uint8_t*>(aligned_src)[i];
            }
            for(int i = 0; i < sizeof(buffer) - size; ++i) {
                reinterpret_cast<volatile uint8_t*>(&buffer[0])[size + i] = reinterpret_cast<const uint8_t*>(aligned_dst)[i];
            }

            *aligned_dst++ = buffer[0];
            *aligned_dst++ = buffer[1];
            WaitWhileBusy();

            if ((FLASH->SR & FLASH_SR_EOP) == 0) {
                return false;
            }

            FLASH->SR = FLASH_SR_EOP;
        }

        FLASH->CR &= ~(FLASH_CR_PG | FLASH_CR_EOPIE);
        Lock();

        return ((FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PROGERR)) == 0);
    }
}

#endif //! ZHELE_FLASH_H