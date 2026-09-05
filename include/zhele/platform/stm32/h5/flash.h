/**
 * @file
 * Implements FLASH for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_FLASH_H
#define ZHELE_PLATFORM_STM32_H5_FLASH_H

#include <stm32h5xx.h>

#include "../common/flash.h"

#include <cstdint>

namespace Zhele
{
    /// Sector size, and therefore the erase granularity
    const static uint32_t FlashSectorSize = 8192;
    /// Programming granularity: one 128-bit flash word
    const static uint32_t FlashWordSize = 16;

    /**
     * @brief Set flash latency for the given AHB frequency
     *
     * @details
     * The wait-state count depends on both frequency and the core voltage scale
     * currently applied (PWR_VOSCR.VOS), so the active scale is read back rather
     * than assumed. See RM0492 table 20.
     *
     * @param [in] frequence Flash interface (AHB) clock frequency, in Hz
     *
     * @par Returns
     *  Nothing
     */
    inline void Flash::ConfigureFrequence(uint32_t frequence)
    {
        // Highest frequency (in MHz) still served by 0, 1, 2, 3, 4 and 5 wait
        // states, per voltage scale (VOS3, VOS2, VOS1, VOS0).
        static constexpr uint32_t maxFreqMhz[4][6] = {
            {20, 40, 60, 80, 100, 100},   // VOS3
            {30, 60, 90, 120, 150, 150},  // VOS2
            {34, 68, 102, 136, 170, 200}, // VOS1
            {42, 84, 126, 168, 210, 250}, // VOS0
        };

        uint32_t voltageScale = (PWR->VOSCR & PWR_VOSCR_VOS_Msk) >> PWR_VOSCR_VOS_Pos;
        uint32_t frequenceMhz = frequence / 1000000;

        uint32_t latency = 5;
        for (uint32_t ws = 0; ws < 6; ++ws) {
            if (frequenceMhz <= maxFreqMhz[voltageScale][ws]) {
                latency = ws;
                break;
            }
        }

        // WRHIGHFREQ trims the programming delay: 00 up to 2 WS, 01 up to 3 WS, 10 above
        uint32_t writeDelay = latency <= 1
            ? 0u
            : latency <= 3
                ? 1u
                : 2u;

        FLASH->ACR = (FLASH->ACR & ~(FLASH_ACR_LATENCY_Msk | FLASH_ACR_WRHIGHFREQ_Msk))
            | (latency << FLASH_ACR_LATENCY_Pos)
            | (writeDelay << FLASH_ACR_WRHIGHFREQ_Pos);

        // The reference manual requires reading the new latency back before the
        // clock is switched.
        while (((FLASH->ACR & FLASH_ACR_LATENCY_Msk) >> FLASH_ACR_LATENCY_Pos) != latency)
            ;
    }

    inline constexpr uint32_t Flash::PageSize(unsigned page)
    {
        return FlashSectorSize;
    }

    inline constexpr uint32_t Flash::PageCount()
    {
        return FlashSize() / FlashSectorSize;
    }

    inline constexpr unsigned Flash::AddressToPage(const void* address)
    {
        uint32_t offset = reinterpret_cast<uint32_t>(address) - FLASH_BASE;

        return offset / FlashSectorSize;
    }

    /**
     * @brief Erase one 8 KByte sector
     *
     * @details
     * Sectors are numbered continuously across both banks, so the bank is
     * derived from the page number and written to BKSEL together with SNB.
     *
     * @param [in] page Sector number
     *
     * @retval true Sector erased
     * @retval false Invalid sector or erase error
     */
    inline bool Flash::ErasePage(uint32_t page)
    {
        if (page >= PageCount())
            return false;

        if (IsLock())
            Unlock();

        WaitWhileBusy();

        constexpr uint32_t sectorsPerBank = 8;
        uint32_t bank = page / sectorsPerBank;
        uint32_t sector = page % sectorsPerBank;

        auto& control = Private::FlashControlReg();

        control = (control & ~(FLASH_CR_SNB_Msk | FLASH_CR_BKSEL_Msk))
            | FLASH_CR_SER
            | (sector << FLASH_CR_SNB_Pos)
            | (bank != 0 ? FLASH_CR_BKSEL : 0u);
        control |= FLASH_CR_START;

        WaitWhileBusy();

        bool success = (Private::FlashStatusReg() & FLASH_SR_EOP) != 0;

        control &= ~(FLASH_CR_SER | FLASH_CR_SNB_Msk | FLASH_CR_BKSEL_Msk);
        Private::FlashStatusReg() = FLASH_SR_EOP;

        return success;
    }

    /**
     * @brief Write a buffer into flash
     *
     * @details
     * Flash is programmed one 128-bit word at a time. A destination that is not
     * word aligned, or a tail shorter than 16 bytes, is completed with the bytes
     * already present in flash so that a full word is always written.
     *
     * @param [in] dst Destination address in flash
     * @param [in] src Source buffer
     * @param [in] size Buffer size in bytes
     *
     * @retval true Write succeeded
     * @retval false Write error
     */
    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        if (IsLock())
            Unlock();

        auto& control = Private::FlashControlReg();
        auto& status = Private::FlashStatusReg();

        auto destination = reinterpret_cast<uint8_t*>(dst);
        auto source = reinterpret_cast<const uint8_t*>(src);

        control |= FLASH_CR_PG;

        bool success = true;

        while (size > 0 && success)
        {
            // Align down to the flash word this chunk starts in
            auto wordAddress = reinterpret_cast<uint32_t>(destination) & ~(FlashWordSize - 1);
            auto wordStart = reinterpret_cast<volatile uint32_t*>(wordAddress);
            unsigned offsetInWord = reinterpret_cast<uint32_t>(destination) - wordAddress;
            unsigned chunk = FlashWordSize - offsetInWord;
            if (chunk > size)
                chunk = size;

            alignas(FlashWordSize) uint8_t buffer[FlashWordSize];
            // Keep whatever is already there outside of the written range
            for (unsigned i = 0; i < FlashWordSize; ++i)
                buffer[i] = reinterpret_cast<const uint8_t*>(wordAddress)[i];
            for (unsigned i = 0; i < chunk; ++i)
                buffer[offsetInWord + i] = source[i];

            auto words = reinterpret_cast<const uint32_t*>(&buffer[0]);
            for (unsigned i = 0; i < FlashWordSize / sizeof(uint32_t); ++i)
                wordStart[i] = words[i];

            WaitWhileBusy();

            success = (status & FLASH_SR_EOP) != 0;
            status = FLASH_SR_EOP;

            destination += chunk;
            source += chunk;
            size -= chunk;
        }

        control &= ~FLASH_CR_PG;
        Lock();

        return success && ((status & (FLASH_SR_WRPERR | FLASH_SR_PGSERR | FLASH_SR_STRBERR | FLASH_SR_INCERR)) == 0);
    }
}

#endif //! ZHELE_PLATFORM_STM32_H5_FLASH_H
