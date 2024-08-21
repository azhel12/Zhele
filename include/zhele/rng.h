/**
 * @file
 * United header for RNG (hardware random generation)
 *
 * @author Alexey Zhelonkin
 * @date 2024
 * @licence FreeBSD
 */

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