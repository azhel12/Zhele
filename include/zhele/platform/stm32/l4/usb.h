/**
 * @file
 * Implement USB protocol for stm32l4 series
 * 
 * @author Alexey Zhelonkin
 * @date 2022
 * @license MIT
 */

#pragma once

#include <stm32l4xx.h>

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
    }
}

