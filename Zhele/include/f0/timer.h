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

using namespace Zhele::IO;

namespace Zhele::Timers
{
    namespace Private
    {
        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using Type = typename Pins::DataType;
            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::AltFunc);
            Pins::AltFuncNumber(mask, _ChPins<_ChannelNumber>::AltFunc);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <typename Pin>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            static_assert(Pins::template IndexOf<Pin> >= 0);
            Pin::Port::Enable();
            Pin::template SetConfiguration<Pins::AltFunc>();
            Pin::template AltFuncNumber<_ChPins<_ChannelNumber>::AltFunc>();
        }


        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ enum{AltFunc = 1}; using Pins = PinList<Pa6, Pb4>; };
        template<> struct Tim3ChPins<1>{ enum{AltFunc = 1}; using Pins = PinList<Pa7, Pb5>; };
        template<> struct Tim3ChPins<2>{ enum{AltFunc = 1}; using Pins = PinList<Pb0, Pb10>; };
        template<> struct Tim3ChPins<3>{ enum{AltFunc = 1}; using Pins = PinList<Pb1, Pb11>; };
        
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