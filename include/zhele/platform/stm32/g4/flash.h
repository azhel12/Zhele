/**
 * @file
 * Implements FLASH for stm32g4 series
 *
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_FLASH_H
#define ZHELE_PLATFORM_STM32_G4_FLASH_H

#include <stm32g4xx.h>

// G4 CMSIS does not define FLASH_SIZE, so (like HAL does) read it from the flash size register.
// Define FLASH_SIZE explicitly to override.
#if !defined (FLASH_SIZE)
    #define FLASH_SIZE (((*reinterpret_cast<const volatile uint16_t*>(FLASHSIZE_BASE)) == 0xFFFFU) \
        ? 0x80000U \
        : (static_cast<uint32_t>(*reinterpret_cast<const volatile uint16_t*>(FLASHSIZE_BASE)) << 10U))
#endif

#include "../common/flash.h"

#include <algorithm>
#include <cstring>

namespace Zhele
{
    /**
     * @brief Configures flash latency (and range 1 boost mode) for target system frequency
     *
     * @details
     * Wait states are calculated for voltage range 1 (normal mode: 30 MHz per WS, up to 150 MHz).
     * Above 150 MHz boost mode is enabled and 4 WS are used (up to 170 MHz).
     * Note: RM0440 recommends AHB prescaler = 2 while switching to a frequency above 150 MHz
     * and restoring it after 1 us. This is not done automatically.
     */
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t ws;
        if (frequence > 150000000u)
        {
            RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
            PWR->CR5 &= ~PWR_CR5_R1MODE;
            ws = 4;
        }
        else
        {
            ws = (frequence - 1) / 30000000u;
            if (ws > 4)
                ws = 4;
        }

        FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY)
            | (ws << FLASH_ACR_LATENCY_Pos)
            | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

        while ((FLASH->ACR & FLASH_ACR_LATENCY) != (ws << FLASH_ACR_LATENCY_Pos)) continue;
    }

    /**
     * @brief Returns page size
     *
     * @details
     * Category 2 devices (G431/G441) have 2 KB pages.
     * Category 3/4 devices (G47x/G48x/G49x) have 2 KB pages in dual bank mode (DBANK = 1, default)
     * and 4 KB pages in single bank mode. Single bank mode is not supported.
     */
    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
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

        FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR
            | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR;

        uint32_t cr = FLASH_CR_PER | FLASH_CR_EOPIE;
#if defined (FLASH_CR_BKER)
        // Dual bank devices: pages of the second bank are addressed relative to it
        constexpr uint32_t pagesPerBank = 128;
        if (page >= pagesPerBank)
        {
            cr |= FLASH_CR_BKER;
            page -= pagesPerBank;
        }
#endif
        FLASH->CR |= cr | (page << FLASH_CR_PNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;

        __asm("nop"); // The software should start checking if the BSY bit equals “0” at least one CPU cycle after setting the STRT bit.

        WaitWhileBusy();

        uint32_t clearMask = FLASH_CR_PER | FLASH_CR_EOPIE | FLASH_CR_PNB_Msk;
#if defined (FLASH_CR_BKER)
        clearMask |= FLASH_CR_BKER;
#endif

        if ((FLASH->SR & FLASH_SR_EOP) == 0) {
            FLASH->CR &= ~clearMask;
            return false;
        }

        FLASH->CR &= ~clearMask;
        FLASH->SR = FLASH_SR_EOP;

        return true;
    }

    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        if (IsLock())
            Unlock();

        const uint32_t* aligned_src = reinterpret_cast<const uint32_t*>(src);
        uint32_t* aligned_dst = reinterpret_cast<uint32_t*>(dst);

        FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR
            | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR;

        FLASH->CR |= FLASH_CR_PG | FLASH_CR_EOPIE;

        if ((reinterpret_cast<uint32_t>(aligned_src) & 0x3) == 0) {
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

            for(unsigned i = 0; i < size; ++i) {
                reinterpret_cast<volatile uint8_t*>(&buffer[0])[i] = reinterpret_cast<const uint8_t*>(aligned_src)[i];
            }
            for(unsigned i = 0; i < sizeof(buffer) - size; ++i) {
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

#endif //! ZHELE_PLATFORM_STM32_G4_FLASH_H
