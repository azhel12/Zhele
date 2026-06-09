/**
 * @file
 * United header for DMA
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_DMAMUX_H
#define ZHELE_PLATFORM_STM32_DMAMUX_H

#if defined(STM32F0)
#endif
#if defined(STM32F1)
#endif
#if defined(STM32F4)
#endif
#if defined(STM32L4)
    #include "l4/dmamux.h"
#endif
#if defined(STM32G0)
    #include "g0/dmamux.h"
#endif
#if defined(STM32C0)
    #include "c0/dmamux.h"
#endif

#endif // ZHELE_PLATFORM_STM32_DMAMUX_H
