/**
 * @file
 * United header for clock
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_CLOCK_H
#define ZHELE_PLATFORM_STM32_CLOCK_H

#if defined (STM32C0)
    #include "c0/clock.h"
#endif
#if defined (STM32F0)
    #include "f0/clock.h"
#endif
#if defined(STM32F1)
    #include "f1/clock.h"
#endif
#if defined(STM32F4)
    #include "f4/clock.h"
#endif
#if defined(STM32L4)
    #include "l4/clock.h"
#endif
#if defined(STM32G0)
    #include "g0/clock.h"
#endif

#endif // ZHELE_PLATFORM_STM32_CLOCK_H
