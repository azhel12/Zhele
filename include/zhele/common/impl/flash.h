/**
 * @file
 * @brief Implement flash
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_IMPL_COMMON_H
#define ZHELE_FLASH_IMPL_COMMON_H

#include <cstdint>

namespace Zhele
{
    inline constexpr uint32_t Flash::FlashSize()
    {
    #if defined (FLASH_END)
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
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
        
        WaitWhileBusy();

        return (FLASH->CR & FLASH_CR_LOCK) > 0;
    }

    inline void Flash::Lock()
    {
        FLASH->CR |= FLASH_CR_LOCK;
    }

    inline bool Flash::IsLock()
    {
        return (FLASH->CR & FLASH_CR_LOCK) != 0;
    }

    inline bool Flash::WritePage(void* dst, const void* src, unsigned size)
    {
        unsigned page = AddressToPage(dst);
		uint32_t offset = reinterpret_cast<uint32_t>(dst) - PageAddress(page);
		return WritePage(page, src, size, offset);
    }

    inline void Flash::WaitWhileBusy()
    {
        while(FLASH->SR & FLASH_SR_BSY) continue;
    }
}

#endif //! ZHELE_FLASH_IMPL_COMMON_H