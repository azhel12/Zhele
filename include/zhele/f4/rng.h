/**
 * @file
 * Implements RNG for stm32f4 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license FreeBSD
 */

#ifndef ZHELE_RNG_H
#define ZHELE_RNG_H

#include <stm32f4xx.h>

#if defined (RNG)
    #include "../common/rng.h"
#else
    #error "THIS MCU does not support hardware RNF"
#endif

#endif //! ZHELE_RNG_H