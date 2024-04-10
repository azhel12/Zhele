/**
 * @file
 * United header for DMA
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */
#if defined(STM32F0)
    #error "No dmamux available for stm32f0"
#endif
#if defined(STM32F1)
    #error "No dmamux available for stm32f1"
#endif
#if defined(STM32F4)
    #error "No dmamux available for stm32f4"
#endif
#if defined(STM32L4)
    #include "l4/dmamux.h"
#endif
#if defined(STM32G0)
    #include "g0/dmamux.h"
#endif