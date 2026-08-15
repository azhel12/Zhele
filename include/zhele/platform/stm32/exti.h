/**
 * @file
 * United header for EXTI
 *
 * @author Alexey Zhelonkin
 * @date 2019
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_EXTI_H
#define ZHELE_PLATFORM_STM32_EXTI_H

#if defined(STM32F0)
    #include "f0/exti.h"
#endif
#if defined(STM32F1)
    #include "f1/exti.h"
#endif
#if defined(STM32F4)
    #include "f4/exti.h"
#endif
#if defined(STM32L4)
    #include "l4/exti.h"
#endif
#if defined(STM32G0)
    #include "g0/exti.h"
#endif
#if defined(STM32G4)
    #include "g4/exti.h"
#endif

#endif // ZHELE_PLATFORM_STM32_EXTI_H
