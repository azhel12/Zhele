/**
 * @file
 * USB hardware flavor detection and compatibility layer
 *
 * Zhele supports three different USB controllers:
 *  - the classic PMA-based device controller (F0/F1/F3/L4/G0), selected by ZHELE_USB_PMA;
 *  - its USB_DRD_FS descendant (G0Bx/H5/U5), which keeps the very same endpoint
 *    register layout but widens the registers to 32 bit, drops the BTABLE register
 *    and allows word accesses to the packet memory only. Selected by
 *    ZHELE_USB_PMA *and* ZHELE_USB_DRD;
 *  - the Synopsys OTG core (F4), selected by ZHELE_USB_OTG.
 *
 * @author Aleksei Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_HW_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_HW_H

#include <stdint.h>
#include <cstring>

#if defined (USB_DRD_FS)
    #define ZHELE_USB_PMA
    #define ZHELE_USB_DRD
#elif defined (USB)
    #define ZHELE_USB_PMA
#elif defined (USB_OTG_FS)
    #define ZHELE_USB_OTG
#endif

#if defined (ZHELE_USB_DRD)
/// CMSIS renamed every endpoint register field for the DRD controller.
/// The bit positions are unchanged, so plain aliases are enough.
    #define USB_EP_CTR_RX   USB_CHEP_VTRX
    #define USB_EP_CTR_TX   USB_CHEP_VTTX
    #define USB_EPREG_MASK  USB_CHEP_REG_MASK
    #define USB_EPRX_STAT   USB_CHEP_RX_STRX
    #define USB_EPTX_STAT   USB_CHEP_TX_STTX
    #define USB_ISTR_EP_ID  USB_ISTR_IDN
    #define USB_PMAADDR     USB_DRD_PMAADDR
#endif

namespace Zhele::Usb
{
#if defined (ZHELE_USB_DRD)
    /// Endpoint (CHEPnR) registers are 32-bit wide and USB_CHEP_REG_MASK does not fit in 16 bit.
    using EndpointRegisterType = uint32_t;
#else
    using EndpointRegisterType = uint16_t;
#endif

/// Thanks ST very much O_o
#if defined (STM32F1) || defined (STM32F3)
    #define PMA_ALIGN_MULTIPLIER 2
    const unsigned PmaAlignMultiplier = 2;
#else
    #define PMA_ALIGN_MULTIPLIER 1
    const unsigned PmaAlignMultiplier = 1;
#endif

#if defined (ZHELE_USB_DRD)
    /// DRD accepts word accesses to the packet memory only, so every endpoint buffer
    /// must start at a word boundary (see USB_DRD_SET_CHEP_TX_ADDRESS in ST`s HAL).
    const unsigned PmaBufferAlign = 4;
#else
    const unsigned PmaBufferAlign = 2;
#endif

    /**
     * @brief Rounds endpoint buffer size up to the packet memory granularity
     *
     * @param [in] size Requested size
     *
     * @returns Size occupied in the packet memory
     */
    consteval unsigned PmaBufferSize(unsigned size)
    {
        return (size + PmaBufferAlign - 1) & ~(PmaBufferAlign - 1);
    }

    /**
     * @brief Writes a 16-bit field of the packet memory at a run-time address
     *
     * @param [in] address Field address
     * @param [in] value Value to write
     *
     * @par Returns
     *  Nothing
     */
    inline void PmaHalfWordSet(uint32_t address, uint16_t value)
    {
#if defined (ZHELE_USB_DRD)
        volatile uint32_t* word = reinterpret_cast<volatile uint32_t*>(address & ~uint32_t(3));
        const unsigned shift = (address & 2) != 0 ? 16 : 0;
        *word = (*word & ~(uint32_t(0xffff) << shift)) | (uint32_t(value) << shift);
#else
        *reinterpret_cast<volatile uint16_t*>(address) = value;
#endif
    }

    /**
     * @brief 16-bit field of the packet memory (buffer descriptor table cell)
     *
     * @details On the classic controller such a field is an ordinary half-word.
     * DRD tolerates word accesses only, so writing one half becomes a read-modify-write
     * of the whole descriptor word - exactly what ST`s USB_DRD_SET_CHEP_*_CNT macros do.
     *
     * @tparam _Address Field address
     */
    template<uint32_t _Address>
    class PmaHalfWord
    {
#if defined (ZHELE_USB_DRD)
        static constexpr uint32_t WordAddress = _Address & ~uint32_t(3);
        static constexpr unsigned Shift = (_Address & 2) != 0 ? 16 : 0;
#endif
    public:
        using DataT = uint16_t;

        static uint16_t Get()
        {
#if defined (ZHELE_USB_DRD)
            return static_cast<uint16_t>(*reinterpret_cast<volatile uint32_t*>(WordAddress) >> Shift);
#else
            return *reinterpret_cast<volatile uint16_t*>(_Address);
#endif
        }

        static void Set(uint16_t value)
        {
#if defined (ZHELE_USB_DRD)
            volatile uint32_t* word = reinterpret_cast<volatile uint32_t*>(WordAddress);
            *word = (*word & ~(uint32_t(0xffff) << Shift)) | (uint32_t(value) << Shift);
#else
            *reinterpret_cast<volatile uint16_t*>(_Address) = value;
#endif
        }
    };

    inline void CopyFromUsbPma(void* destination, const void* source, unsigned size)
    {
#if defined (ZHELE_USB_DRD)
        const volatile uint32_t* words = reinterpret_cast<const volatile uint32_t*>(source);
        uint8_t* target = reinterpret_cast<uint8_t*>(destination);

        for(unsigned i = 0; i < size / 4; ++i) {
            uint32_t word = *words++;
            memcpy(target, &word, 4);
            target += 4;
        }
        if(unsigned tail = size % 4; tail != 0) {
            uint32_t word = *words;
            memcpy(target, &word, tail);
        }
#else
        if constexpr(PmaAlignMultiplier != 1) {
            for(unsigned i = 0; i < size / 2; ++i) {
                reinterpret_cast<uint16_t*>(destination)[i] = reinterpret_cast<const uint16_t*>(source)[PmaAlignMultiplier * i];
            }
            if(size & 0x01) {
                reinterpret_cast<uint8_t*>(destination)[size - 1] = reinterpret_cast<const uint8_t*>(source)[PmaAlignMultiplier * (size - 1)];
            }
        }
        else {
            memcpy(destination, source, size);
        }
#endif
    }

    inline void CopyToUsbPma(void* destination, const void* source, unsigned size)
    {
#if defined (ZHELE_USB_DRD)
        volatile uint32_t* words = reinterpret_cast<volatile uint32_t*>(destination);
        const uint8_t* origin = reinterpret_cast<const uint8_t*>(source);

        for(unsigned i = 0; i < size / 4; ++i) {
            uint32_t word;
            memcpy(&word, origin, 4);
            *words++ = word;
            origin += 4;
        }
        if(unsigned tail = size % 4; tail != 0) {
            uint32_t word = 0;
            memcpy(&word, origin, tail);
            *words = word;
        }
#else
        if constexpr(PmaAlignMultiplier != 1) {
            for(unsigned i = 0; i < size / 2; ++i) {
                reinterpret_cast<uint16_t*>(destination)[PmaAlignMultiplier * i] = reinterpret_cast<const uint16_t*>(source)[i];
            }
            if(size & 0x01) {
                reinterpret_cast<uint8_t*>(destination)[PmaAlignMultiplier * (size - 1)] = reinterpret_cast<const uint8_t*>(source)[size - 1];
            }
        }
        else {
            memcpy(destination, source, size);
        }
#endif
    }
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_HW_H
