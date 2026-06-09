/**
 * @file
 * United header for DMA
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#ifndef ZHELE_DMA_H
#define ZHELE_DMA_H

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
  #include "platform/stm32/dma.h"
#elif defined(ZHELE_PLATFORM_CH32)
  #include "platform/ch32/dma.h"
#elif defined(ZHELE_PLATFORM_NIIET)
  #include "platform/niiet/dma.h"
#else
  #error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers."
#endif

#endif // ZHELE_DMA_H
