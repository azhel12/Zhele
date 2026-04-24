/**
 * @file
 * Resolves ZHELE_PLATFORM_*: tries each backend's platform_detector.h unless the user
 * already defined a platform (e.g. -DZHELE_PLATFORM_STM32)
 */
#pragma once

#if !defined(ZHELE_PLATFORM_STM32) && !defined(ZHELE_PLATFORM_CH32) && !defined(ZHELE_PLATFORM_NIIET)
  #include "platform/stm32/platform_detector.h"
  #include "platform/ch32/platform_detector.h"
  #include "platform/niiet/platform_detector.h"
#endif


