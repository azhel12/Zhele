/**
 * @file
 * @brief Implement flash
 * @author Konstantin Chizhov
 * @date 2013
 * @license FreeBSD
 */

#ifndef ZHELE_FLASH_COMMON_H
#define ZHELE_FLASH_COMMON_H

#include <cstdint>

namespace Zhele
{
    class Flash
	{
    public:
        /**
         * @brief Configure flash for target system frequency
         * 
         * @param frequence Flash frequence
         * 
         * @par Returns
         *  Nothing 
        */
        static void ConfigureFrequence(uint32_t frequence);

        /**
         * @brief Returns total flash size
         * 
         * @returns Flash size in bytes
        */
        static constexpr uint32_t FlashSize();

        /**
         * @brief Returns flash page flash size
         * 
         * @returns page size in bytes
        */
        static constexpr uint32_t PageSize(unsigned page);

        /**
         * @brief Returns flash page count
         * 
         * @returns page count
        */
        static constexpr uint32_t PageCount();

        /**
         * @brief Calculates page begin address
         * 
         * @param page Page number
         * 
         * @returns Page address
        */
        static constexpr uint32_t PageAddress(unsigned page);

        /**
         * @brief Calculates page number by address
         * 
         * @param page Page number
         * 
         * @returns Page address
        */
        static constexpr unsigned AddressToPage(const void* address);

        /**
         * @brief Unlock flash
         * 
         * @retval true Unlock success
         * @retval false Unlock failed
        */
        static bool Unlock();

        /**
         * @brief Lock flash
         * 
         * @par Returns
         *  Nothing
        */
        static void Lock();

        /**
         * @brief Returns flash lock status
         * 
         * @retval true Flash is locked
         * @retval false Flash is unlocked
        */
        static bool IsLock();

        /**
         * @brief Erase flash page
         * 
         * @param page Page number
         * 
         * @retval true Erase success
         * @retval false Erase failed
        */
        static bool ErasePage(uint32_t page);

        /**
         * @brief Writes data to flash
         * 
         * @param page Page number
         * @param data Data to write
         * @param size Data size
         * @param offset Offset
         * 
         * @retval true Write success
         * @retval false Write failed
        */
        static bool WritePage(unsigned page, const void* data, unsigned size, unsigned offset = 0);

        /**
         * @brief Writes data to flash
         * 
         * @param dst Destination address
         * @param src Data to write
         * @param size Data size
         * 
         * @retval true Write success
         * @retval false Write failed
        */
        static bool WritePage(void* dst, const void* src, unsigned size);

    private:
        /**
         * @brief Block execution while flash busy
         * 
         * @par Returns
         *  Nothing
        */
        static void WaitWhileBusy();
    };
}

#include "impl/flash.h"

#endif //! ZHELE_FLASH_COMMON_H