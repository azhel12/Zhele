/**
 * @file
 * @brief Macros, templates for timers for stm32f4 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_TIMER_H
#define ZHELE_TIMER_H

#include <stm32f4xx.h>

#include "iopins.h"

#include "../common/timer.h"
#include "../common/template_utils/pair.h"
#include "../common/template_utils/static_array.h"

namespace Zhele::Timers
{
    namespace Private
    {
        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsAltFuncNumber;
            using Type = typename Pins::DataType;
            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::AltFunc);
            Pins::SetDriverType(mask, Pins::DriverType::PushPull);
            Pins::AltFuncNumber(mask, GetNumberRuntime<PinAltFuncNumbers>::Get(pinNumber));
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsAltFuncNumber;
            using Pin = typename Pins::template Pin<PinNumber>;

            Pin::Port::Enable();
            Pin::template SetConfiguration<Pin::Port::AltFunc>();
            Pin::template SetDriverType<Pin::Port::DriverType::PushPull>();
            Pin::template AltFuncNumber<GetNonTypeValueByIndex<PinNumber, PinAltFuncNumbers>::value>();
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <typename Pin>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            static_assert(Pins::template IndexOf<Pin> >= 0);
            
            SelectPins<Pins::template IndexOf<Pin>>();
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins(int pinNumber)
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            using PinAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsAltFuncNumber;
            using Type = typename Pins::DataType;
            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(mask, Pins::In);
            Pins::AltFuncNumber(mask, GetNumberRuntime<PinAltFuncNumbers>::Get(pinNumber));
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            using PinAltFuncNumbers = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsAltFuncNumber;
            using Pin = typename Pins::template Pin<PinNumber>;

            Pin::Port::Enable();
            Pin::template SetConfiguration<Pin::Port::In>();
            Pin::template AltFuncNumber<GetNonTypeValueByIndex<PinNumber, PinAltFuncNumbers>::value>();
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <typename Pin>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins()
        {
            static_assert(Pins::template IndexOf<Pin> >= 0);
            
            SelectPins<Pins::template IndexOf<Pin>>();
        }

        using namespace Zhele::IO;
        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ using Pins = Pair<IO::PinList<Pa0, Pa5, Pa15>, NonTypeTemplateArray<1, 1, 1>>; };
        template<> struct Tim2ChPins<1>{ using Pins = Pair<IO::PinList<Pa1, Pb3>, NonTypeTemplateArray<1, 1, 1>>; };
        template<> struct Tim2ChPins<2>{ using Pins = Pair<IO::PinList<Pa2, Pb10>, NonTypeTemplateArray<1, 1, 1>>; };
        template<> struct Tim2ChPins<3>{ using Pins = Pair<IO::PinList<Pa3, Pb11>, NonTypeTemplateArray<1, 1, 1>>; };
        
        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ using Pins = Pair<IO::PinList<Pa6, Pb4, Pc6>, NonTypeTemplateArray<2, 2, 2>>; };
        template<> struct Tim3ChPins<1>{ using Pins = Pair<IO::PinList<Pa7, Pb5, Pc7>, NonTypeTemplateArray<2, 2, 2>>; };
        template<> struct Tim3ChPins<2>{ using Pins = Pair<IO::PinList<Pb0, Pc8>, NonTypeTemplateArray<2, 2>>; };
        template<> struct Tim3ChPins<3>{ using Pins = Pair<IO::PinList<Pb1, Pc9>, NonTypeTemplateArray<2, 2>>; };
        
        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ using Pins = Pair<IO::PinList<Pb6, Pd12>, NonTypeTemplateArray<2, 2>>; };
        template<> struct Tim4ChPins<1>{ using Pins = Pair<IO::PinList<Pb7, Pd13>, NonTypeTemplateArray<2, 2>>; };
        template<> struct Tim4ChPins<2>{ using Pins = Pair<IO::PinList<Pb8, Pd14>, NonTypeTemplateArray<2, 2>>; };
        template<> struct Tim4ChPins<3>{ using Pins = Pair<IO::PinList<Pb9, Pd15>, NonTypeTemplateArray<2, 2>>; };
        
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