/**
 * @file
 * United header for timers
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_TIMER_H
#define ZHELE_PLATFORM_STM32_TIMER_H

#if defined(STM32F0)
    #include "f0/timer.h"
#endif
#if defined(STM32F1)
    #include "f1/timer.h"
#endif
#if defined(STM32F4)
    #include "f4/timer.h"
#endif
#if defined(STM32L4)
    #include "l4/timer.h"
#endif
#if defined(STM32G0)
    #include "g0/timer.h"
#endif
#if defined(STM32C0)
    #include "c0/timer.h"
#endif

#endif // ZHELE_PLATFORM_STM32_TIMER_H
