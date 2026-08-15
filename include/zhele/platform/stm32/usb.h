/**
 * @file
 * United header for USB
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_USB_H
#define ZHELE_PLATFORM_STM32_USB_H

#if defined(STM32F0)
    #include "f0/usb.h"
#endif
#if defined(STM32F1)
    #include "f1/usb.h"
#endif
#if defined(STM32F4)
    #include "f4/usb.h"
#endif
#if defined(STM32L4)
    #include "l4/usb.h"
#endif
#if defined(STM32G0)
    #include "g0/usb.h"
#endif
#if defined(STM32G4)
    #error USB for STM32G4 is not implemented yet
#endif

#endif // ZHELE_PLATFORM_STM32_USB_H
