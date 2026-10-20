/**
 * @file
 * Bind USB device stack to STM32 USB controller
 *
 * @author Aleksei Zhelonkin
 * @date 2021
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_COMMON_USB_DEVICE_H
#define ZHELE_PLATFORM_STM32_COMMON_USB_DEVICE_H

#include "hw.h"

#include "../ioreg.h"

#include <zhele/common/usb/device.h>

#if defined (ZHELE_USB_PMA)
    #include "pma/controller.h"
#elif defined (ZHELE_USB_OTG)
    #include "otg/controller.h"
#endif

namespace Zhele::Usb
{
#if defined (ZHELE_USB_DRD)
    IO_STRUCT_WRAPPER(USB_DRD_FS, UsbRegs, USB_DRD_TypeDef);
#elif defined (ZHELE_USB_PMA)
    IO_STRUCT_WRAPPER(USB, UsbRegs, USB_TypeDef);
#elif defined (ZHELE_USB_OTG)
    IO_STRUCT_WRAPPER(USB_OTG_FS, UsbRegs, USB_OTG_GlobalTypeDef);
    IO_STRUCT_WRAPPER(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE, UsbDeviceRegs, USB_OTG_DeviceTypeDef);
#endif

// On some families (e.g. STM32F1) USB_LP_IRQn/USB_HP_IRQn are preprocessor macros and
// `defined(USB_LP_IRQn)` correctly detects them. On others (e.g. STM32G4) they are
// IRQn_Type enumerators, invisible to `#if defined()` - such a family header must
// `#define USB_IRQ` itself before including this file, which the guard below respects.
#if !defined (USB_IRQ)
#if defined (ZHELE_USB_DRD)
    #define USB_IRQ USB_DRD_FS_IRQn
#elif defined (USB_LP_IRQn)
    #define USB_IRQ USB_LP_IRQn
#elif defined (ZHELE_USB_PMA)
    #define USB_IRQ USB_IRQn
#elif defined (ZHELE_USB_OTG)
    #define USB_IRQ OTG_FS_IRQn
#endif
#endif

#if defined (ZHELE_USB_PMA)
    using UsbController = PmaController<UsbRegs, USB_IRQ, Zhele::Clock::UsbClock>;
#elif defined (ZHELE_USB_OTG)
    using UsbController = OtgController<UsbRegs, UsbDeviceRegs, USB_IRQ, Zhele::Clock::OtgFsClock>;
#endif

    template<
        uint16_t _UsbVersion,
        DeviceAndInterfaceClass _Class,
        uint8_t _SubClass,
        uint8_t _Protocol,
        uint16_t _VendorId,
        uint16_t _ProductId,
        uint16_t _DeviceReleaseNumber,
        auto _Manufacturer,
        auto _Product,
        auto _Serial,
        typename _Ep0,
        typename... _Configurations>
    using DeviceWithStrings = DeviceBase<UsbController, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, _Manufacturer, _Product, _Serial, _Ep0, _Configurations...>;

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
    using Device = DeviceBase<UsbController, _UsbVersion, _Class, _SubClass, _Protocol, _VendorId, _ProductId, _DeviceReleaseNumber, template_utils::EmptyFixedString16, template_utils::EmptyFixedString16, template_utils::EmptyFixedString16, _Ep0, _Configurations...>;
}

#endif //! ZHELE_PLATFORM_STM32_COMMON_USB_DEVICE_H
