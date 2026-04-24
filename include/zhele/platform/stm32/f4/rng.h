/**
 * @file
 * Implements RNG for stm32f4 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license MIT
 */

#pragma once

#include <stm32f4xx.h>

#if defined (RNG)
    #include "../common/rng.h"
#else
    #error "THIS MCU does not support hardware RNF"
#endif

