/**
 * @file
 * United header for ADC
 *
 * @author Alexey Zhelonkin
 * @date 2019
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_ADC_H
#define ZHELE_PLATFORM_STM32_ADC_H

#if defined(STM32F0)
    #include "f0/adc.h"
#endif
#if defined(STM32F1)
    #include "f1/adc.h"
#endif
#if defined(STM32F4)
    #include "f4/adc.h"
#endif
#if defined(STM32G0)
    #include "g0/adc.h"
#endif
#if defined(STM32G4)
    #include "g4/adc.h"
#endif

#endif // ZHELE_PLATFORM_STM32_ADC_H
