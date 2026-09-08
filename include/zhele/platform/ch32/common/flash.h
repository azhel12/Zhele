/**
 * @file
 * Flash controller for CH32 (WCH FPEC)
 *
 * Standard mode programs halfwords via CTLR.PG; the newer V00x parts dropped it
 * and use the 256-byte fast page buffer instead — the family header selects that
 * by defining ZHELE_CH32_FLASH_FAST_PROGRAMMING. Erase is CTLR.PER either way.
 *
 * Addresses may be given in either flash alias (0x00000000 execution, as in the
 * linker scripts, or 0x08000000 programming, as in the reference manual) and are
 * normalized before reaching the controller.
 *
 * @author Aleksei Zhelonkin (based on the STM32 flash implementation by Konstantin Chizhov)
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_COMMON_FLASH_H
#define ZHELE_PLATFORM_CH32_COMMON_FLASH_H

#include <cstdint>

namespace Zhele
{
    class Flash
    {
        /// @brief FPEC unlock keys
        static constexpr uint32_t FlashKey1 = 0x45670123UL;
        static constexpr uint32_t FlashKey2 = 0xCDEF89ABUL;

        /// @brief Mask which strips the alias (0x00000000 / 0x08000000) from an address
        static constexpr uint32_t AliasMask = 0x0007FFFFUL;

    public:
        /**
         * @brief Configure flash wait states for target system frequency
         *
         * @param [in] frequence Target HCLK frequency
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
         * @brief Returns flash page (erase granularity) size
         *
         * @param [in] page Page number
         *
         * @returns Page size in bytes
        */
        static constexpr uint32_t PageSize(unsigned page);

        /**
         * @brief Returns flash page count
         *
         * @returns Page count
        */
        static constexpr uint32_t PageCount();

        /**
         * @brief Calculates page begin address
         *
         * @param [in] page Page number
         *
         * @returns Page address (execution alias, as used by the linker scripts)
        */
        static constexpr uint32_t PageAddress(unsigned page);

        /**
         * @brief Calculates page number by address
         *
         * @param [in] address Address (either alias)
         *
         * @returns Page number
        */
        static constexpr unsigned AddressToPage(const void* address);

        /**
         * @brief Unlock flash
         *
         * @retval true Flash is unlocked
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
         * @note Do not confirm the result by blank-checking for 0xFF: on CH32V203
         * an erased sector reads back as 0xE339E339 in every word until something
         * is programmed into it (V00x does read 0xFF).
         *
         * @param [in] page Page number
         *
         * @retval true Erase success
         * @retval false Erase failed
        */
        static bool ErasePage(uint32_t page);

        /**
         * @brief Writes data to flash, checking that it stays inside one page
         *
         * @param [in] dst Destination address
         * @param [in] src Data to write
         * @param [in] size Data size
         *
         * @retval true Write success
         * @retval false Write failed
        */
        static bool WritePage(void* dst, const void* src, unsigned size);

        /**
         * @brief Writes data to the given flash page
         *
         * @param [in] page Page number
         * @param [in] src Data to write
         * @param [in] size Data size
         * @param [in] offset Offset inside the page
         *
         * @retval true Write success
         * @retval false Write failed
        */
        static bool WritePage(unsigned page, const void* src, unsigned size, unsigned offset);

        /**
         * @brief Writes data to flash
         *
         * @note The target area must be erased beforehand. In standard mode the
         * destination must be halfword-aligned and an odd size is padded with 0xFF.
         *
         * @param [in] dst Destination address
         * @param [in] src Data to write
         * @param [in] size Data size
         *
         * @retval true Write success
         * @retval false Write failed
        */
        static bool WriteFlash(void* dst, const void* src, unsigned size);

    private:
        /**
         * @brief Translates any flash alias into the programming one (0x08000000)
         *
         * @param [in] address Address to translate
         *
         * @returns Address in the programming alias
        */
        static constexpr uint32_t Normalize(uint32_t address);

        /**
         * @brief Block execution while flash busy
         *
         * @par Returns
         *  Nothing
        */
        static void WaitWhileBusy();

        /**
         * @brief Wait for a just-triggered erase/program to finish
         *
         * @note Waits for STATR.BSY to appear before waiting for it to drop, then
         * requires STATR.EOP, which hardware raises without CTLR.EOPIE.
         *
         * @retval true Operation completed
         * @retval false Timed out
        */
        static bool WaitForOperation();

        /// @brief Poll count allowed for one erase/program before giving up
        static constexpr uint32_t OperationTimeout = 2000000;

        /**
         * @brief Clears sticky status flags before starting an operation
         *
         * @par Returns
         *  Nothing
        */
        static void ClearStatus();

        /**
         * @brief Clears sticky status flags and reports whether last operation succeeded
         *
         * @retval true Last operation completed without errors
         * @retval false Write protection or programming error was detected
        */
        static bool CheckAndClearStatus();

    #if defined(ZHELE_CH32_FLASH_FAST_PROGRAMMING)
        /// @brief Fast-mode page (program granularity) size in bytes
        static constexpr uint32_t FastPageSize = 256;

        // Spelled out because the WCH V00x header casts them to uint16_t
        // (#define FLASH_CTLR_PAGE_PG ((uint16_t)0x00010000)), truncating to zero.
        static constexpr uint32_t CtlrPageProgram = 0x00010000UL;
        static constexpr uint32_t CtlrPageErase = 0x00020000UL;
        static constexpr uint32_t CtlrBufferLoad = 0x00040000UL;
        static constexpr uint32_t CtlrBufferReset = 0x00080000UL;

        /**
         * @brief Unlock flash and fast program/erase mode
         *
         * @retval true Both locks are released
         * @retval false Unlock failed
        */
        static bool UnlockFast();

        /**
         * @brief Lock fast program/erase mode and flash itself
         *
         * @par Returns
         *  Nothing
        */
        static void LockFast();
    #endif
    };
}

#include "impl/flash.h"

#endif //! ZHELE_PLATFORM_CH32_COMMON_FLASH_H
