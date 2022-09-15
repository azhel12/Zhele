/**
 * @file
 * United header for timers
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */
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