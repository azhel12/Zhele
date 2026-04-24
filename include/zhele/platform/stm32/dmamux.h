/**
 * @file
 * United header for DMA
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#pragma once

#if defined(STM32F0)
#endif
#if defined(STM32F1)
#endif
#if defined(STM32F4)
#endif
#if defined(STM32L4)
    #include "l4/dmamux.h"
#endif
#if defined(STM32G0)
    #include "g0/dmamux.h"
#endif


