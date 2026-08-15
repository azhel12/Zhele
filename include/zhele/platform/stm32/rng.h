/**
 * @file
 * STM32: family dispatch for RNG
 */

#ifndef ZHELE_PLATFORM_STM32_RNG_H
#define ZHELE_PLATFORM_STM32_RNG_H

#if defined(STM32F0)
    #error STM32F0 does not support RNG
#endif
#if defined(STM32F1)
    #error STM32F1 does not support RNG
#endif
#if defined(STM32F4)
    #include "f4/rng.h"
#endif
#if defined(STM32L4)
    #include "l4/rng.h"
#endif
#if defined(STM32G0)
    #error STM32G0 does not support RNG
#endif
#if defined(STM32G4)
    #include "g4/rng.h"
#endif

#endif // ZHELE_PLATFORM_STM32_RNG_H
