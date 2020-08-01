/**
 * @file
 * @brief Macros, templates for timers for stm32f0 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_H
#define ZHELE_TIMER_H

#include "iopins.h"

#include "../common/timer.h"

namespace Zhele::Timers
{
    namespace Private
    {
        using namespace Zhele::IO;
        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ enum{AltFunc = 1}; using Pins = PinList<Pa0, Pa15>; };
        template<> struct Tim2ChPins<1>{ enum{AltFunc = 1}; using Pins = PinList<Pa1, Pb3>; };
        template<> struct Tim2ChPins<2>{ enum{AltFunc = 1}; using Pins = PinList<Pa2, Pb10>; };
        template<> struct Tim2ChPins<3>{ enum{AltFunc = 1}; using Pins = PinList<Pa3, Pb11>; };
        
        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ enum{AltFunc = 2}; using Pins = PinList<Pa6, Pb4>; };
        template<> struct Tim3ChPins<1>{ enum{AltFunc = 2}; using Pins = PinList<Pa7, Pb5>; };
        template<> struct Tim3ChPins<2>{ enum{AltFunc = 2}; using Pins = PinList<Pb0, Pb10>; };
        template<> struct Tim3ChPins<3>{ enum{AltFunc = 2}; using Pins = PinList<Pb1, Pb11>; };
        
    #if defined (TIM1)
        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
    #endif
    #if defined (TIM2)
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
    #endif
    #if defined (TIM3)
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
    #endif
    }

    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
}

#endif //! ZHELE_TIMER_H