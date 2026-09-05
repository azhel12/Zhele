/**
 * @file
 * USB for stm32h5 series (not implemented)
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_USB_H
#define ZHELE_PLATFORM_STM32_H5_USB_H

// H503 has a USB_DRD_FS controller: CHEPnR is bit-compatible with the classic
// EPnR, but there is no BTABLE register and the registers are 32-bit, so the
// stack in common/usb/ does not apply as is.
#error "USB is not implemented for STM32H5 yet"

#endif //! ZHELE_PLATFORM_STM32_H5_USB_H
