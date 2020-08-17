/**
 * @file
 * @brief Macros, templates for timers for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_H
#define ZHELE_TIMER_H

#include "iopins.h"

#include "../common/timer.h"
#include "remap.h"
namespace Zhele::Timers
{
    namespace Private
    {
        template <typename Periph>
        struct TimerRemapAdapter{};

        class Tim1Regs;
        class Tim2Regs;
        class Tim3Regs;
        class Tim4Regs;

        template<>
        struct TimerRemapAdapter<Tim1Regs>{using Remap = Zhele::IO::Timer1Remap;};
        template<>
        struct TimerRemapAdapter<Tim2Regs>{using Remap = Zhele::IO::Timer2Remap;};
        template<>
        struct TimerRemapAdapter<Tim3Regs>{using Remap = Zhele::IO::Timer3Remap;};
        template<>
        struct TimerRemapAdapter<Tim4Regs>{using Remap = Zhele::IO::Timer4Remap;};

        template<typename Regs>
        using TimerRemap = typename TimerRemapAdapter<Regs>::Remap;

        template<typename Regs>
        void RemapTimer(int altFuncNumber)
        {
            TimerRemap<Regs>::Set(altFuncNumber);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using Type = typename Pins::DataType;
            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::AltFunc);
            RemapTimer<_Regs>(pinNumber);
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
            RemapTimer<_Regs>(Pins::template IndexOf<Pin>);
        }

        using namespace Zhele::IO;
        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ using Pins = PinList<Pa0, Pa15, Pa0, Pa15>; };
        template<> struct Tim2ChPins<1>{ using Pins = PinList<Pa1, Pb3, Pa1, Pb3>; };
        template<> struct Tim2ChPins<2>{ using Pins = PinList<Pa2, Pa2, Pb10, Pb10>; };
        template<> struct Tim2ChPins<3>{ using Pins = PinList<Pa3, Pa3, Pb11, Pb11>; };		

        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ using Pins = PinList<Pa6, NullPin, Pb4, Pc6>; };
        template<> struct Tim3ChPins<1>{ using Pins = PinList<Pa7, NullPin, Pb5, Pc7>; };
        template<> struct Tim3ChPins<2>{ using Pins = PinList<Pb0, NullPin, Pb0, Pc8>; };
        template<> struct Tim3ChPins<3>{ using Pins = PinList<Pb1, NullPin, Pb1, Pc9>; };

        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ using Pins = PinList<Pb6, Pd12>; };
        template<> struct Tim4ChPins<1>{ using Pins = PinList<Pb7, Pd13>; };
        template<> struct Tim4ChPins<2>{ using Pins = PinList<Pb8, Pd14>; };
        template<> struct Tim4ChPins<3>{ using Pins = PinList<Pb9, Pd15>; };

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM4, Tim4Regs, TIM_TypeDef);
    }

    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
    using Timer4 = Private::GPTimer<Private::Tim4Regs, Clock::Tim4Clock, TIM4_IRQn, Private::Tim4ChPins>;
}

#endif //! ZHELE_TIMER_H