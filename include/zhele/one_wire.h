/**
 * @file
 * United header for one-wire protocol
 *
 * @author Alexey Zhelonkin
 * @license MIT
 */

#pragma once

#include "platform_detector.h"

#if defined(ZHELE_PLATFORM_STM32)
  #include "platform/stm32/one_wire.h"
#elif defined(ZHELE_PLATFORM_CH32)
  #include "platform/ch32/one_wire.h"
#else
  #error "Zhele: unsupported platform. Define ZHELE_PLATFORM_XX or include CMSIS device headers."
#endif


