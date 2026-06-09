/**
 * @file
 * United header for USART
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#ifndef ZHELE_USART_H
#define ZHELE_USART_H

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
  #include "platform/stm32/usart.h"
#elif defined(ZHELE_PLATFORM_CH32)
  #include "platform/ch32/usart.h"
#elif defined(ZHELE_PLATFORM_NIIET)
  #include "platform/niiet/usart.h"
#else
  #error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers."
#endif

#endif // ZHELE_USART_H
