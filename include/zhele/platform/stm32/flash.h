/**
 * @file
 * United header for Flash
 *
 * @author Alexey Zhelonkin
 * @date 2020
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_FLASH_H
#define ZHELE_PLATFORM_STM32_FLASH_H

#if defined(STM32C0)
    #include "c0/flash.h"
#endif
#if defined(STM32F0)
    #include "f0/flash.h"
#endif
#if defined(STM32F1)
    #include "f1/flash.h"
#endif
#if defined(STM32F4)
    #include "f4/flash.h"
#endif
#if defined(STM32L4)
    #include "l4/flash.h"
#endif
#if defined(STM32G0)
    #include "g0/flash.h"
#endif

#endif // ZHELE_PLATFORM_STM32_FLASH_H
