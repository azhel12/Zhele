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

    template<
        uint16_t _UsbVersion,
        DeviceAndInterfaceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        typename _Ep0,
        typename... _Configurations>
    using Device = DeviceBase<UsbRegs, USB_LP_IRQn, Zhele::Clock::UsbClock, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Ep0, _Configurations...>;
}

#endif //! ZHELE_USB_H