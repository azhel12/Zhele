/**
 * @file
 * Implements RNG for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_RNG_H
#define ZHELE_PLATFORM_STM32_H5_RNG_H

#include <stm32h5xx.h>

#if defined (RNG)
    #include "../common/rng.h"
#else
    #error "This MCU does not support hardware RNG"
#endif

#endif //! ZHELE_PLATFORM_STM32_H5_RNG_H
