/**
 * @file
 * Implement USB protocol for stm32g4 series
 *
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_USB_H
#define ZHELE_PLATFORM_STM32_G4_USB_H

#include <stm32g4xx.h>

#include "clock.h"

// USB_LP_IRQn/USB_HP_IRQn are IRQn_Type enumerators, not preprocessor macros: the common
// code's `#if defined(USB_LP_IRQn)` can't see them, so define USB_IRQ here (interrupts for
// USB device events are wired to the low-priority vector on G4).
#define USB_IRQ USB_LP_IRQn

#include "../common/usb/device.h"

namespace Zhele::Usb
{
    /**
     * @brief 48 MHz clock source for the USB peripheral (RCC_CCIPR CLK48SEL)
     */
    enum class ClockSource
    {
        Hsi48, ///< HSI48 RC oscillator, trimmed against USB SOF via CRS (crystal-less USB, reset default)
        Pll,   ///< PLLQ output (requires PllClock::SetUsbOutputDivider<>() and an HSE crystal)
    };

    USB_DEVICE_TEMPLATE_ARGS
    template<auto clockSource>
    void USB_DEVICE_TEMPLATE_QUALIFIER::SelectClockSource()
    {
        static_assert(std::is_same_v<decltype(clockSource), Zhele::Usb::ClockSource>, "Clock source argument must be ClockSource enum value.");

        if constexpr (clockSource == Zhele::Usb::ClockSource::Hsi48)
        {
            Zhele::Clock::Hsi48Clock::Enable();

            RCC->APB1ENR1 |= RCC_APB1ENR1_CRSEN;
            RCC->CCIPR &= ~RCC_CCIPR_CLK48SEL; // 00: HSI48
            CRS->CR |= CRS_CR_AUTOTRIMEN;
            CRS->CR |= CRS_CR_CEN;
        }
        if constexpr (clockSource == Zhele::Usb::ClockSource::Pll)
        {
            RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_CLK48SEL) | RCC_CCIPR_CLK48SEL_0; // 01: PLLQ
        }
    }
}

#endif //! ZHELE_PLATFORM_STM32_G4_USB_H
