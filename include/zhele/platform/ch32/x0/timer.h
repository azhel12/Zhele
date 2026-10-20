/**
 * @file
 * @brief Implements timers for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_TIMER_H
#define ZHELE_PLATFORM_CH32_X0_TIMER_H

#include <ch32x035.h>

#include "clock.h"
#include "iopins.h"

#include <zhele/pinlist.h>

#include "../common/timer.h"
#include "../common/ioreg.h"

namespace Zhele::Timers
{
    namespace Private
    {
        using namespace Zhele::IO;

        template<unsigned> struct Tim1ChPins;
        template<> struct Tim1ChPins<0> { using Pins = PinList<Pb9>; };
        template<> struct Tim1ChPins<1> { using Pins = PinList<Pb10>; };
        template<> struct Tim1ChPins<2> { using Pins = PinList<Pb11>; };
        template<> struct Tim1ChPins<3> { using Pins = PinList<Pc16>; };

        template<unsigned> struct Tim2ChPins;
        template<> struct Tim2ChPins<0> { using Pins = PinList<Pa0>; };
        template<> struct Tim2ChPins<1> { using Pins = PinList<Pa1>; };
        template<> struct Tim2ChPins<2> { using Pins = PinList<Pa2>; };
        template<> struct Tim2ChPins<3> { using Pins = PinList<Pa3>; };

        template<unsigned> struct Tim3ChPins;
        template<> struct Tim3ChPins<0> { using Pins = PinList<Pa6>; };
        template<> struct Tim3ChPins<1> { using Pins = PinList<Pa7>; };

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
    }

    using Timer1 = Private::AdvancedTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_UP_IRQn, Private::Tim1ChPins>;
    using Timer2 = Private::AdvancedTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_UP_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
}

#endif // ZHELE_PLATFORM_CH32_X0_TIMER_H
