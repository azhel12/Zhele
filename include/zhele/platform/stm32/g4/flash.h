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
    namespace Private
    {
        /// Category 3/4 devices run dual bank (2 KB pages) when OPTR.DBANK = 1 (factory default),
        /// single bank (4 KB pages) otherwise. Category 2 devices (G431/G441) are always single bank, 2 KB pages.
        inline bool FlashIsDualBank()
        {
#if defined (FLASH_OPTR_DBANK)
            return (FLASH->OPTR & FLASH_OPTR_DBANK) != 0;
#else
            return false;
#endif
        }

        /// Instruction and data caches must be reset after flash contents change (RM0440, flash cache management);
        /// a cache may be reset only while it is disabled.
        inline void FlashFlushCaches()
        {
            uint32_t acr = FLASH->ACR;
            FLASH->ACR = acr & ~(FLASH_ACR_ICEN | FLASH_ACR_DCEN);
            FLASH->ACR |= FLASH_ACR_ICRST | FLASH_ACR_DCRST;
            FLASH->ACR &= ~(FLASH_ACR_ICRST | FLASH_ACR_DCRST);
            FLASH->ACR = acr;
        }

        inline void FlashClearErrors()
        {
            FLASH->SR = FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR
                | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR;
        }
    }

    /**
     * @brief Configures flash latency (and range 1 boost mode) for target system frequency
     *
     * @details
     * Wait states are calculated for voltage range 1 (normal mode: 30 MHz per WS, up to 150 MHz).
     * Above 150 MHz boost mode is enabled and 4 WS are used (up to 170 MHz).
     * The AHB /2 step RM0440 requires when switching above 80 MHz is done by SysClock::SelectClockSource().
     */
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        uint32_t ws;
        if (frequence > 150000000u)
        {
            RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
            (void)RCC->APB1ENR1; // Peripheral clock enable delay before PWR access
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
     * and 4 KB pages in single bank mode.
     */
    inline constexpr uint32_t Flash::PageSize(unsigned)
    {
#if defined (FLASH_OPTR_DBANK)
        return Private::FlashIsDualBank() ? 2048 : 4096;
#else
        return 2048;
#endif
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
        Private::FlashClearErrors();

        uint32_t cr = FLASH_CR_PER | FLASH_CR_EOPIE;
#if defined (FLASH_CR_BKER)
        // Dual bank mode: pages of the second bank are numbered from 0 again
        if (Private::FlashIsDualBank())
        {
            const uint32_t pagesPerBank = PageCount() / 2;
            if (page >= pagesPerBank)
            {
                cr |= FLASH_CR_BKER;
                page -= pagesPerBank;
            }
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
        FLASH->CR &= ~clearMask;
        Private::FlashFlushCaches();

        if ((FLASH->SR & FLASH_SR_EOP) == 0)
            return false;

        FLASH->SR = FLASH_SR_EOP;

        return true;
    }

    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        // Flash is programmed by double words (RM0440 §3.3.7)
        if ((reinterpret_cast<uint32_t>(dst) & 0x7) != 0)
            return false;

        if (IsLock())
            Unlock();

        const uint8_t* source = reinterpret_cast<const uint8_t*>(src);
        uint32_t* aligned_dst = reinterpret_cast<uint32_t*>(dst);

        WaitWhileBusy();
        Private::FlashClearErrors();

        FLASH->CR |= FLASH_CR_PG | FLASH_CR_EOPIE;

        bool result = true;
        while (size > 0)
        {
            // Bytes past the end of the source keep the current flash contents
            const unsigned chunk = size < 8 ? size : 8;
            uint32_t buffer[2];
            std::memcpy(buffer, aligned_dst, sizeof(buffer));
            std::memcpy(buffer, source, chunk);

            *aligned_dst++ = buffer[0];
            *aligned_dst++ = buffer[1];
            source += chunk;
            size -= chunk;

            WaitWhileBusy();

            if ((FLASH->SR & FLASH_SR_EOP) == 0)
            {
                result = false;
                break;
            }

            FLASH->SR = FLASH_SR_EOP;
        }

        FLASH->CR &= ~(FLASH_CR_PG | FLASH_CR_EOPIE);
        Private::FlashFlushCaches();
        Lock();

        return result && ((FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PROGERR)) == 0);
    }
}

#endif //! ZHELE_PLATFORM_STM32_G4_FLASH_H
