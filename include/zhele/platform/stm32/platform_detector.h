/**
 * @file
 * STM32: sets ZHELE_PLATFORM_STM32 from CMSIS / device macros if not set explicitly
 */
#pragma once

#if defined(ZHELE_PLATFORM_STM32)
#elif defined(STM32F0) || defined(STM32F1) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7) \
    || defined(STM32G0) || defined(STM32L4) || defined(STM32L0) || defined(STM32H7)
#define ZHELE_PLATFORM_STM32 1
#endif


