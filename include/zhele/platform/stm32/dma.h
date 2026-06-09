/**
 * @file
 * United header for DMA
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_DMA_H
#define ZHELE_PLATFORM_STM32_DMA_H

#if defined(STM32F0)
    #include "f0/dma.h"
#endif
#if defined(STM32F1)
    #include "f1/dma.h"
#endif
#if defined(STM32F4)
    #include "f4/dma.h"
#endif
#if defined(STM32L4)
    #include "l4/dma.h"
#endif
#if defined(STM32G0)
    #include "g0/dma.h"
#endif
#if defined(STM32C0)
    #include "c0/dma.h"
#endif

#endif // ZHELE_PLATFORM_STM32_DMA_H
