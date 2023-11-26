/**
 * @file
 * Implement USB protocol for stm32f1 series
 * 
 * @author Alexey Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_USB_H
#define ZHELE_USB_H

#include <stm32f1xx.h>

#include "../common/usb/device.h"

namespace Zhele::Usb
{
    enum class ClockSource
    {
        Pll,
        PllDividedOneAndHalf
    };
    
    USB_DEVICE_TEMPLATE_ARGS
    template<typename T>
    void USB_DEVICE_TEMPLATE_QUALIFIER::SelectClockSource(T clockSource)
    {
        static_assert(std::is_same_v<T, Zhele::Usb::ClockSource>, "Clock source argument must be ClockSource enum value.");
        if (clockSource == Zhele::Usb::ClockSource::Pll)
        {
            RCC->CFGR |= RCC_CFGR_USBPRE;
        }
    }
}

#endif //! ZHELE_USB_H