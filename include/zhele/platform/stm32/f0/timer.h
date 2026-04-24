/**
 * @file
 * @brief Macros, templates for timers for stm32f0 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#pragma once

#include "../common/timer.h"

#include "iopins.h"

#include <cstddef>
#include <cstdint>

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
            using PinsConfig = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsConfig;
            using Type = typename Pins::DataType;

            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(Pins::Configuration::AltFunc, mask);
            Pins::SetDriverType(Pins::DriverType::PushPull, mask);
            Pins::AltFuncNumber(PinsConfig::alt_functions[static_cast<size_t>(pinNumber)], mask);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinsConfig = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsConfig;
            using Pin = typename Pins::template Pin<PinNumber>;

            Pin::Port::Enable();
            Pin::template SetConfiguration<Pin::Port::Configuration::AltFunc>();
            Pin::template SetDriverType<Pin::Port::DriverType::PushPull>();
            Pin::template AltFuncNumber<PinsConfig::alt_functions[PinNumber]>();
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
            using PinsConfig = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsConfig;
            using Type = typename Pins::DataType;

            Type mask = 1 << pinNumber;
            Pins::Enable();
            Pins::SetConfiguration(Pins::Configuration::AltFunc, mask);
            Pins::AltFuncNumber(PinsConfig::alt_functions[static_cast<size_t>(pinNumber)], mask);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::Pins;
            using PinsConfig = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::InputCapture<_ChannelNumber>::PinsConfig;
            using Pin = typename Pins::template Pin<PinNumber>;

            Pin::Port::Enable();
            Pin::template SetConfiguration<Pin::Configuration::AltFunc>();
            Pin::template AltFuncNumber<PinsConfig::alt_functions[PinNumber]>();
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


        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ using Pins = IO::AltPinList<{1, 1},
            Pa6, Pb4>; };
        template<> struct Tim3ChPins<1>{ using Pins = IO::AltPinList<{1, 1},
            Pa7, Pb5>; };
        template<> struct Tim3ChPins<2>{ using Pins = IO::AltPinList<{1, 1},
            Pb0, Pb10>; };
        template<> struct Tim3ChPins<3>{ using Pins = IO::AltPinList<{1, 1},
            Pb1, Pb11>; };
        
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

