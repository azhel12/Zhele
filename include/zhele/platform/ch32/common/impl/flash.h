/**
 * @file
 * Flash controller implementation for CH32 (WCH FPEC)
 *
 * @author Aleksei Zhelonkin
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_IMPL_FLASH_H
#define ZHELE_PLATFORM_CH32_COMMON_IMPL_FLASH_H

#include <cstdint>

namespace Zhele
{
    inline constexpr uint32_t Flash::Normalize(uint32_t address)
    {
        return (address & AliasMask) | FLASH_BASE;
    }

    inline constexpr uint32_t Flash::PageCount()
    {
        return FlashSize() / PageSize(0);
    }

    inline constexpr uint32_t Flash::PageAddress(unsigned page)
    {
        return page * PageSize(page);
    }

    inline constexpr unsigned Flash::AddressToPage(const void* address)
    {
        return (reinterpret_cast<uint32_t>(address) & AliasMask) / PageSize(0);
    }

    inline void Flash::WaitWhileBusy()
    {
        while (FLASH->STATR & FLASH_STATR_BSY) continue;
    }

    inline bool Flash::WaitForOperation()
    {
        // Bounded: an operation that has already finished never shows BSY.
        for (unsigned guard = 64; guard != 0 && (FLASH->STATR & FLASH_STATR_BSY) == 0; --guard)
            continue;

        WaitWhileBusy();

        for (uint32_t timeout = OperationTimeout; timeout != 0; --timeout) {
            if (FLASH->STATR & FLASH_STATR_EOP)
                return true;
        }

        return false;
    }

    inline void Flash::ClearStatus()
    {
        FLASH->STATR = FLASH_STATR_EOP | FLASH_STATR_WRPRTERR
    #if defined(FLASH_STATR_PGERR)
            | FLASH_STATR_PGERR
    #endif
            ;
    }

    inline bool Flash::CheckAndClearStatus()
    {
        constexpr uint32_t errors = FLASH_STATR_WRPRTERR
    #if defined(FLASH_STATR_PGERR)
            | FLASH_STATR_PGERR
    #endif
            ;

        bool result = (FLASH->STATR & errors) == 0;
        FLASH->STATR = FLASH_STATR_EOP | errors;

        return result;
    }

    inline bool Flash::Unlock()
    {
        if (!IsLock())
            return true;

        FLASH->KEYR = FlashKey1;
        FLASH->KEYR = FlashKey2;

        return !IsLock();
    }

    inline void Flash::Lock()
    {
        FLASH->CTLR |= FLASH_CTLR_LOCK;
    }

    inline bool Flash::IsLock()
    {
        return (FLASH->CTLR & FLASH_CTLR_LOCK) != 0;
    }

    inline bool Flash::ErasePage(uint32_t page)
    {
        if (page >= PageCount())
            return false;

        if (!Unlock())
            return false;

        WaitWhileBusy();
        ClearStatus();

    #if defined(ZHELE_CH32_FLASH_FAST_PROGRAMMING)
        // A previous fast cycle must not leave its mode bits behind.
        FLASH->CTLR &= ~(static_cast<uint32_t>(FLASH_CTLR_OPTER) | CtlrPageErase | CtlrPageProgram);
    #endif

        uint32_t address = Normalize(PageAddress(page));

        FLASH->CTLR |= FLASH_CTLR_PER;
        FLASH->ADDR = address;
        FLASH->CTLR |= FLASH_CTLR_STRT;

        bool result = WaitForOperation();

        FLASH->CTLR &= ~static_cast<uint32_t>(FLASH_CTLR_PER);

        result = CheckAndClearStatus() && result;
        Lock();

        return result;
    }

#if defined(ZHELE_CH32_FLASH_FAST_PROGRAMMING)
    inline bool Flash::UnlockFast()
    {
        FLASH->KEYR = FlashKey1;
        FLASH->KEYR = FlashKey2;

        FLASH->MODEKEYR = FlashKey1;
        FLASH->MODEKEYR = FlashKey2;

        return !IsLock() && (FLASH->CTLR & FLASH_CTLR_FLOCK) == 0;
    }

    inline void Flash::LockFast()
    {
        FLASH->CTLR |= FLASH_CTLR_FLOCK;
        FLASH->CTLR |= FLASH_CTLR_LOCK;
    }

    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        uint32_t address = Normalize(reinterpret_cast<uint32_t>(dst));
        auto data = reinterpret_cast<const uint8_t*>(src);

        if (!UnlockFast())
            return false;

        ClearStatus();

        bool result = true;

        while (size > 0) {
            // A fast programming cycle always rewrites a whole page, so the words
            // the caller does not touch are reloaded from their current content.
            uint32_t pageAddress = address & ~(FastPageSize - 1);
            unsigned offset = address - pageAddress;
            unsigned chunk = FastPageSize - offset < size ? FastPageSize - offset : size;

            FLASH->CTLR &= ~(static_cast<uint32_t>(FLASH_CTLR_OPTER) | CtlrPageErase);
            FLASH->CTLR |= CtlrPageProgram;
            FLASH->CTLR |= CtlrBufferReset;
            WaitWhileBusy();

            for (unsigned i = 0; i < FastPageSize; i += sizeof(uint32_t)) {
                // Reading flash while the page buffer is being filled is safe —
                // this very loop is fetched from flash. Only the programming
                // cycle itself (STRT) stalls the core.
                uint32_t word = *reinterpret_cast<const volatile uint32_t*>(pageAddress + i);

                for (unsigned byte = 0; byte < sizeof(uint32_t); ++byte) {
                    unsigned position = i + byte;
                    if (position >= offset && position < offset + chunk) {
                        reinterpret_cast<uint8_t*>(&word)[byte] = data[position - offset];
                    }
                }

                *reinterpret_cast<volatile uint32_t*>(pageAddress + i) = word;
                FLASH->CTLR |= CtlrBufferLoad;
                WaitWhileBusy();
            }

            FLASH->ADDR = pageAddress;
            FLASH->CTLR |= FLASH_CTLR_STRT;
            bool completed = WaitForOperation();
            FLASH->CTLR &= ~CtlrPageProgram;

            if (!CheckAndClearStatus() || !completed) {
                result = false;
                break;
            }

            address += chunk;
            data += chunk;
            size -= chunk;
        }

        LockFast();

        return result;
    }
#else
    inline bool Flash::WriteFlash(void* dst, const void* src, unsigned size)
    {
        uint32_t address = Normalize(reinterpret_cast<uint32_t>(dst));
        auto data = reinterpret_cast<const uint8_t*>(src);

        // Programming granularity is a halfword, so an odd size gets its tail
        // padded with 0xFF (an erased flash byte) and an odd address is rejected.
        if (address & 0x01)
            return false;

        if (!Unlock())
            return false;

        WaitWhileBusy();
        ClearStatus();
        FLASH->CTLR |= FLASH_CTLR_PG;

        bool result = true;
        for (unsigned i = 0; i < size; i += sizeof(uint16_t)) {
            uint8_t halfword[sizeof(uint16_t)] = {
                data[i],
                static_cast<uint8_t>((i + 1) < size ? data[i + 1] : 0xFF)
            };

            *reinterpret_cast<volatile uint16_t*>(address + i)
                = static_cast<uint16_t>(halfword[0]) | static_cast<uint16_t>(halfword[1] << 8);

            bool completed = WaitForOperation();

            if (!CheckAndClearStatus() || !completed) {
                result = false;
                break;
            }
        }

        FLASH->CTLR &= ~static_cast<uint32_t>(FLASH_CTLR_PG);
        Lock();

        return result;
    }
#endif

    inline bool Flash::WritePage(void* dst, const void* src, unsigned size)
    {
        unsigned page = AddressToPage(dst);
        uint32_t offset = (reinterpret_cast<uint32_t>(dst) & AliasMask) - PageAddress(page);

        if (page >= PageCount())
            return false;

        if (offset + size > PageSize(page))
            return false;

        return WriteFlash(dst, src, size);
    }

    inline bool Flash::WritePage(unsigned page, const void* src, unsigned size, unsigned offset)
    {
        if (page >= PageCount())
            return false;

        if (offset + size > PageSize(page))
            return false;

        return WriteFlash(reinterpret_cast<uint8_t*>(PageAddress(page)) + offset, src, size);
    }
}

#endif //! ZHELE_PLATFORM_CH32_COMMON_IMPL_FLASH_H
