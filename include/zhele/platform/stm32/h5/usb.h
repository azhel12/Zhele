/**
 * @file
 * Implement USB protocol for stm32h5 series
 *
 * @details H503 carries the USB_DRD_FS controller in device mode. Its endpoint
 * registers keep the classic bit layout, so the common stack applies as is; the
 * differences (32-bit registers, no BTABLE, word-only packet memory) are handled
 * in common/usb/hw.h.
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_USB_H
#define ZHELE_PLATFORM_STM32_H5_USB_H

#include <stm32h5xx.h>

#include "clock.h"

#include "../common/usb/device.h"

namespace Zhele::Usb
{
    enum class ClockSource
    {
        Hsi48, ///< HSI48 trimmed by CRS against the USB SOF
        Pll1Q, ///< PLL1 Q output (must be tuned to 48 MHz by the application)
        Pll2Q  ///< PLL2 Q output (must be tuned to 48 MHz by the application)
    };

    USB_DEVICE_TEMPLATE_ARGS
    template<auto clockSource>
    void USB_DEVICE_TEMPLATE_QUALIFIER::SelectClockSource()
    {
        static_assert(std::is_same_v<decltype(clockSource), Zhele::Usb::ClockSource>, "Clock source argument must be ClockSource enum value.");

        if constexpr (clockSource == Zhele::Usb::ClockSource::Hsi48)
        {
            Zhele::Clock::Hsi48Clock::Enable();

            MODIFY_REG(RCC->CCIPR4, RCC_CCIPR4_USBSEL, RCC_CCIPR4_USBSEL_0 | RCC_CCIPR4_USBSEL_1);

            // Trim HSI48 against the start of frame packets, as the free running
            // oscillator alone does not meet the full speed tolerance.
            Zhele::Clock::CrsClock::Enable();
            MODIFY_REG(CRS->CFGR, CRS_CFGR_SYNCSRC, CRS_CFGR_SYNCSRC_1);
            CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
        }
        if constexpr (clockSource == Zhele::Usb::ClockSource::Pll1Q)
        {
            MODIFY_REG(RCC->CCIPR4, RCC_CCIPR4_USBSEL, RCC_CCIPR4_USBSEL_0);
        }
        if constexpr (clockSource == Zhele::Usb::ClockSource::Pll2Q)
        {
            MODIFY_REG(RCC->CCIPR4, RCC_CCIPR4_USBSEL, RCC_CCIPR4_USBSEL_1);
        }
    }
}

#endif //! ZHELE_PLATFORM_STM32_H5_USB_H
