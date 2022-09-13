/**
 * @file
 * United header for Watchdog
 *
 * @author Alexey Zhelonkin
 * @date 2022
 * @licence FreeBSD
 */

#if defined(STM32F0)
    #include <stm32f0xx.h>
#endif
#if defined(STM32F1)
    #include <stm32f1xx.h>
#endif
#if defined(STM32F4)
    #include <stm32f4xx.h>
#endif
#if defined(STM32L4)
    #include <stm32l4xx.h>
#endif


#include <common/watchdog.h>