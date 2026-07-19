/**
 * @file
 * CH32V20x: instantiate the timers (TIM1 advanced, TIM2-4 general-purpose).
 *
 * @note TIM1 is an advanced timer — call Timer1::EnableMainOutput() before its
 *       channel outputs will drive the pins.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_TIMER_H
#define ZHELE_PLATFORM_CH32_V2_TIMER_H

#include <ch32v20x.h>

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
        template<> struct Tim1ChPins<0> { using Pins = PinList<Pa8>; };
        template<> struct Tim1ChPins<1> { using Pins = PinList<Pa9>; };
        template<> struct Tim1ChPins<2> { using Pins = PinList<Pa10>; };
        template<> struct Tim1ChPins<3> { using Pins = PinList<Pa11>; };

        template<unsigned> struct Tim2ChPins;
        template<> struct Tim2ChPins<0> { using Pins = PinList<Pa0>; };
        template<> struct Tim2ChPins<1> { using Pins = PinList<Pa1>; };
        template<> struct Tim2ChPins<2> { using Pins = PinList<Pa2>; };
        template<> struct Tim2ChPins<3> { using Pins = PinList<Pa3>; };

        template<unsigned> struct Tim3ChPins;
        template<> struct Tim3ChPins<0> { using Pins = PinList<Pa6>; };
        template<> struct Tim3ChPins<1> { using Pins = PinList<Pa7>; };
        template<> struct Tim3ChPins<2> { using Pins = PinList<Pb0>; };
        template<> struct Tim3ChPins<3> { using Pins = PinList<Pb1>; };

        template<unsigned> struct Tim4ChPins;
        template<> struct Tim4ChPins<0> { using Pins = PinList<Pb6>; };
        template<> struct Tim4ChPins<1> { using Pins = PinList<Pb7>; };
        template<> struct Tim4ChPins<2> { using Pins = PinList<Pb8>; };
        template<> struct Tim4ChPins<3> { using Pins = PinList<Pb9>; };

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
#if defined(TIM4)
        IO_STRUCT_WRAPPER(TIM4, Tim4Regs, TIM_TypeDef);
#endif
    }

    using Timer1 = Private::AdvancedTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_UP_IRQn, Private::Tim1ChPins>;
    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
#if defined(TIM4)
    using Timer4 = Private::GPTimer<Private::Tim4Regs, Clock::Tim4Clock, TIM4_IRQn, Private::Tim4ChPins>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_TIMER_H
