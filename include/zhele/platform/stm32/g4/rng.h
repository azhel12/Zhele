/**
 * @file
 * Implements RNG for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_RNG_H
#define ZHELE_PLATFORM_STM32_G4_RNG_H

#include <stm32g4xx.h>

#if defined (RNG)
    // RNG kernel clock (RCC_CCIPR_CLK48SEL) resets to HSI48: enable it before Rng::Init()
    // (Zhele::Clock::Hsi48Clock::Enable(), see clock.h) or select another 48 MHz source.
    #include "../common/rng.h"
#else
    #error "THIS MCU does not support hardware RNG"
#endif

#endif //! ZHELE_PLATFORM_STM32_G4_RNG_H
