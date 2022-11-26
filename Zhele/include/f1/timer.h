/**
 * @file
 * @brief Macros, templates for timers for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_H
#define ZHELE_TIMER_H


#include "../common/timer.h"
#include "../common/template_utils/pair.h"
#include "../common/template_utils/static_array.h"

#include "iopins.h"
#include "remap.h"

namespace Zhele::Timers
{
    namespace Private
    {
        class Tim1Regs; class Tim2Regs; class Tim3Regs; class Tim4Regs;
        using TimerRegs = TypeList<Tim1Regs, Tim2Regs, Tim3Regs, Tim4Regs>;
        using TimerRemaps = TypeList<Zhele::IO::Timer1Remap, Zhele::IO::Timer2Remap, Zhele::IO::Timer3Remap
#if defined (TIM4)
        , Zhele::IO::Timer4Remap
#endif
        >;

        template <typename _Regs>
        struct TimerRemapHelper
        {
            using type = typename GetType<TypeIndex<_Regs, TimerRegs>::value, TimerRemaps>::type;
        };

        template<typename Regs>
        using GetTimerRemap = typename TimerRemapHelper<Regs>::type;

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinsAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsAltFuncNumber;
            using Type = typename Pins::DataType;

            Type mask = 1 << pinNumber;

            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::AltFunc);
            Pins::SetDriverType(mask, Pins::DriverType::PushPull);
            GetTimerRemap<_Regs>::Set(GetNumberRuntime<PinsAltFuncNumbers>::Get(pinNumber));
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinsAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsAltFuncNumber;
            using Pin = typename Pins::template Pin<PinNumber>;
            
            Pin::Port::Enable();
            Pin::template SetConfiguration<Pins::AltFunc>();
            Pin::template SetDriverType<Pins::DriverType::PushPull>();
            GetTimerRemap<_Regs>::Set(GetNonTypeValueByIndex<Pins::template IndexOf<Pin>, PinsAltFuncNumbers>::value);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <typename Pin>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            static_assert(Pins::template IndexOf<Pin> >= 0);
            
            SelectPins<Pins::template IndexOf<Pin>>();
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            using PinsAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsAltFuncNumber;
            using Type = typename Pins::DataType;

            Type mask = 1 << pinNumber;

            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::In);
            GetTimerRemap<_Regs>::Set(GetNumberRuntime<PinsAltFuncNumbers>::Get(pinNumber));
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            using PinsAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsAltFuncNumber;
            using Pin = typename Pins::template Pin<PinNumber>;
            
            Pin::Port::Enable();
            Pin::template SetConfiguration<Pins::In>();
            GetTimerRemap<_Regs>::Set(GetNonTypeValueByIndex<Pins::template IndexOf<Pin>, PinsAltFuncNumbers>::value);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <typename Pin>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            static_assert(Pins::template IndexOf<Pin> >= 0);
            
            SelectPins<Pins::template IndexOf<Pin>>();
        }

        using namespace Zhele::IO;

        template<unsigned ChannelNumber> struct Tim1ChPins;
        template<> struct Tim1ChPins<0>{ using Pins = Pair<IO::PinList<Pa8, Pe9>, NonTypeTemplateArray<0, 3>>; };
        template<> struct Tim1ChPins<1>{ using Pins = Pair<IO::PinList<Pa9, Pe11>, NonTypeTemplateArray<0, 3>>; };
        template<> struct Tim1ChPins<2>{ using Pins = Pair<IO::PinList<Pa10, Pe13>, NonTypeTemplateArray<0, 3>>; };
        template<> struct Tim1ChPins<3>{ using Pins = Pair<IO::PinList<Pa11, Pe14>, NonTypeTemplateArray<0, 3>>; };		

        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ using Pins = Pair<IO::PinList<Pa0, Pa15, Pa0, Pa15>, NonTypeTemplateArray<0, 1, 2, 3>>; };
        template<> struct Tim2ChPins<1>{ using Pins = Pair<IO::PinList<Pa1, Pb3, Pa1, Pb3>, NonTypeTemplateArray<0, 1, 2, 3>>; };
        template<> struct Tim2ChPins<2>{ using Pins = Pair<IO::PinList<Pa2, Pb10>, NonTypeTemplateArray<0, 2>>; };
        template<> struct Tim2ChPins<3>{ using Pins = Pair<IO::PinList<Pa3, Pb11>, NonTypeTemplateArray<0, 2>>; };		

        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ using Pins = Pair<IO::PinList<Pa6, Pb4, Pc6>, NonTypeTemplateArray<0, 2, 3>>; };
        template<> struct Tim3ChPins<1>{ using Pins = Pair<IO::PinList<Pa7, Pb5, Pc7>, NonTypeTemplateArray<0, 2, 3>>; };
        template<> struct Tim3ChPins<2>{ using Pins = Pair<IO::PinList<Pb0, Pc8>, NonTypeTemplateArray<0, 3>>; };
        template<> struct Tim3ChPins<3>{ using Pins = Pair<IO::PinList<Pb1, Pc9>, NonTypeTemplateArray<0, 3>>; };

#if defined (TIM4)
        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ using Pins = Pair<IO::PinList<Pb6, Pd12>, NonTypeTemplateArray<0, 1>>; };
        template<> struct Tim4ChPins<1>{ using Pins = Pair<IO::PinList<Pb7, Pd13>, NonTypeTemplateArray<0, 1>>; };
        template<> struct Tim4ChPins<2>{ using Pins = Pair<IO::PinList<Pb8, Pd14>, NonTypeTemplateArray<0, 1>>; };
        template<> struct Tim4ChPins<3>{ using Pins = Pair<IO::PinList<Pb9, Pd15>, NonTypeTemplateArray<0, 1>>; };
#endif
        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
#if defined (TIM4)
        IO_STRUCT_WRAPPER(TIM4, Tim4Regs, TIM_TypeDef);
#endif
#if defined (TIM6)
        IO_STRUCT_WRAPPER(TIM6, Tim6Regs, TIM_TypeDef);
#endif
#if defined (TIM7)        
        IO_STRUCT_WRAPPER(TIM7, Tim7Regs, TIM_TypeDef);
#endif
    }

    using Timer1 = Private::AdvancedTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_UP_IRQn, Private::Tim1ChPins>;
    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
#if defined (TIM4)
    using Timer4 = Private::GPTimer<Private::Tim4Regs, Clock::Tim4Clock, TIM4_IRQn, Private::Tim4ChPins>;
#endif
#if defined (TIM6)
    using Timer6 = Private::BaseTimer<Private::Tim6Regs, Clock::Tim6Clock, TIM6_IRQn>;
#endif
#if defined (TIM7)
    using Timer7 = Private::BaseTimer<Private::Tim7Regs, Clock::Tim7Clock, TIM7_IRQn>;
#endif
}

#endif //! ZHELE_TIMER_H