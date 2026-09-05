/**
 * @file
 * @brief Implement flash
 * @author Alexey Zhelonkin
 * @date 2024
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_IMPL_FLASH_H
#define ZHELE_PLATFORM_STM32_COMMON_IMPL_FLASH_H

#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        /**
         * @brief Flash controller registers, by role
         *
         * @details
         * Every family calls them KEYR/CR/SR except H5, where the ones reachable
         * from non-secure code are named NSKEYR/NSCR/NSSR. The *bit* names
         * (FLASH_CR_LOCK, FLASH_SR_BSY, ...) are shared, so only the register
         * accessors need indirection.
         */
    #if defined (STM32H5)
        inline volatile uint32_t& FlashKeyReg() { return FLASH->NSKEYR; }
        inline volatile uint32_t& FlashControlReg() { return FLASH->NSCR; }
        inline volatile uint32_t& FlashStatusReg() { return FLASH->NSSR; }
    #else
        inline volatile uint32_t& FlashKeyReg() { return FLASH->KEYR; }
        inline volatile uint32_t& FlashControlReg() { return FLASH->CR; }
        inline volatile uint32_t& FlashStatusReg() { return FLASH->SR; }
    #endif
    }

    inline constexpr unsigned Flash::SqrtOfPowerOfTwo(uint32_t value)
    {
        unsigned result = 0;

        if (value & (value - 1))
            return 0xffffffff;

        while (value != (1 << result))
            ++result;

        return result;
    }

    inline constexpr uint32_t Flash::FlashSize()
    {
    #if defined (FLASH_SIZE)
        return FLASH_SIZE;
    #elif defined (FLASH_END)
        return FLASH_END - FLASH_BASE;
    #elif defined (FLASH_BANK2_END)
        return FLASH_BANK2_END - FLASH_BASE;
    #elif defined (FLASH_BANK1_END)
        return FLASH_BANK1_END - FLASH_BASE;
    #else
        #error "Cannot determine flash size"
    #endif
    }

    inline constexpr uint32_t Flash::PageAddress(unsigned page)
    {
        return FLASH_BASE + page * PageSize(page);
    }

    inline bool Flash::Unlock()
    {
        static constexpr uint32_t flashKey1 = 0x45670123UL;
        static constexpr uint32_t flashKey2 = 0xCDEF89ABUL;

        Private::FlashKeyReg() = flashKey1;
        Private::FlashKeyReg() = flashKey2;

        WaitWhileBusy();

        return (Private::FlashControlReg() & FLASH_CR_LOCK) > 0;
    }

    inline void Flash::Lock()
    {
        Private::FlashControlReg() |= FLASH_CR_LOCK;
    }

    inline bool Flash::IsLock()
    {
        return (Private::FlashControlReg() & FLASH_CR_LOCK) != 0;
    }

    inline bool Flash::WritePage(void* dst, const void* src, unsigned size)
    {
        unsigned page = AddressToPage(dst);
		uint32_t offset = reinterpret_cast<uint32_t>(dst) - PageAddress(page);

        if(page > PageCount())
            return false;
        
        if(offset + size > PageSize(page))
            return false;

		return WriteFlash(dst, src, size);
    }

    inline bool Flash::WritePage(unsigned page, const void* src, unsigned size, unsigned offset)
    {
        if(page > PageCount())
            return false;
        
        if(offset + size > PageSize(page))
            return false;

		return WriteFlash(reinterpret_cast<uint8_t*>(PageAddress(page)) + offset, src, size);
    }

    inline void Flash::WaitWhileBusy()
    {
    #if defined (FLASH_SR_BSY1)
        while(Private::FlashStatusReg() & FLASH_SR_BSY1) continue;
    #else
        while(Private::FlashStatusReg() & FLASH_SR_BSY) continue;
    #endif
    }
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_IMPL_FLASH_H