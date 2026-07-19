/**
 * @file
 * CH32V00x: instantiate the timers.
 *
 * @note BaseTimer only (counting/update); output-compare pin maps not defined
 *       for the V0 GPIO layout yet.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_TIMER_H
#define ZHELE_PLATFORM_CH32_V0_TIMER_H

#include <ch32v00x.h>

#include "clock.h"

#include "../common/timer.h"
#include "../common/ioreg.h"

namespace Zhele::Timers
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
    }

    // TIM3 is left out: its TIM3_IRQn is not declared in the V00x CMSIS headers.
    using Timer1 = Private::BaseTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_UP_IRQn>;
    using Timer2 = Private::BaseTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn>;
}

#endif // ZHELE_PLATFORM_CH32_V0_TIMER_H
