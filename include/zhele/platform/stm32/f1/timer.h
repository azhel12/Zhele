/**
 * @file
 * @brief Macros, templates for timers for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license MIT
 */

#pragma once

#include "../common/timer.h"

#include "iopins.h"
#include "remap.h"

#include <cstddef>
#include <cstdint>

namespace Zhele::Timers
{
    namespace Private
    {
        class Tim1Regs; class Tim2Regs; class Tim3Regs; class Tim4Regs;
        using TimerRegs = template_utils::type_list<Tim1Regs, Tim2Regs, Tim3Regs, Tim4Regs>;
        using TimerRemaps = template_utils::type_list<Zhele::IO::Timer1Remap, Zhele::IO::Timer2Remap, Zhele::IO::Timer3Remap
#if defined (TIM4)
        , Zhele::IO::Timer4Remap
#endif
        >;

        template <typename _Regs>
        struct TimerRemapHelper
        {
            using type = template_utils::type_unbox<TimerRemaps::template get<TimerRegs::template search<_Regs>()>()>;
        };

        template<typename Regs>
        using GetTimerRemap = typename TimerRemapHelper<Regs>::type;

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
            GetTimerRemap<_Regs>::Set(PinsConfig::alt_functions[static_cast<size_t>(pinNumber)]);
        }

        template <typename _Regs, typename _ClockEnReg, IRQn_Type _IRQNumber, template<unsigned> typename _ChPins>
        template <unsigned _ChannelNumber>
        template <unsigned PinNumber>
        void GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::SelectPins()
        {
            using Pins = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::Pins;
            using PinsConfig = GPTimer<_Regs, _ClockEnReg, _IRQNumber, _ChPins>::OutputCompare<_ChannelNumber>::PinsConfig;
            using Pin = Pins::template Pin<PinNumber>;
            
            Pin::Port::Enable();
            Pin::template SetConfiguration<Pins::Configuration::AltFunc>();
            Pin::template SetDriverType<Pins::DriverType::PushPull>();
            GetTimerRemap<_Regs>::Set(PinsConfig::alt_functions[PinNumber]);
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
            GetTimerRemap<_Regs>::Set(PinsConfig::alt_functions[static_cast<size_t>(pinNumber)]);
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
            GetTimerRemap<_Regs>::Set(PinsConfig::alt_functions[PinNumber]);
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
        template<> struct Tim1ChPins<0>{ using Pins = IO::AltPinList<{0, 3},
            Pa8, Pe9>; };
        template<> struct Tim1ChPins<1>{ using Pins = IO::AltPinList<{0, 3},
            Pa9, Pe11>; };
        template<> struct Tim1ChPins<2>{ using Pins = IO::AltPinList<{0, 3},
            Pa10, Pe13>; };
        template<> struct Tim1ChPins<3>{ using Pins = IO::AltPinList<{0, 3},
            Pa11, Pe14>; };

        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ using Pins = IO::AltPinList<{0, 1, 2, 3},
            Pa0, Pa15, Pa0, Pa15>; };
        template<> struct Tim2ChPins<1>{ using Pins = IO::AltPinList<{0, 1, 2, 3},
            Pa1, Pb3, Pa1, Pb3>; };
        template<> struct Tim2ChPins<2>{ using Pins = IO::AltPinList<{0, 2},
            Pa2, Pb10>; };
        template<> struct Tim2ChPins<3>{ using Pins = IO::AltPinList<{0, 2},
            Pa3, Pb11>; };

        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ using Pins = IO::AltPinList<{0, 2, 3},
            Pa6, Pb4, Pc6>; };
        template<> struct Tim3ChPins<1>{ using Pins = IO::AltPinList<{0, 2, 3},
            Pa7, Pb5, Pc7>; };
        template<> struct Tim3ChPins<2>{ using Pins = IO::AltPinList<{0, 3},
            Pb0, Pc8>; };
        template<> struct Tim3ChPins<3>{ using Pins = IO::AltPinList<{0, 3},
            Pb1, Pc9>; };

#if defined (TIM4)
        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ using Pins = IO::AltPinList<{0, 1},
            Pb6, Pd12>; };
        template<> struct Tim4ChPins<1>{ using Pins = IO::AltPinList<{0, 1},
            Pb7, Pd13>; };
        template<> struct Tim4ChPins<2>{ using Pins = IO::AltPinList<{0, 1},
            Pb8, Pd14>; };
        template<> struct Tim4ChPins<3>{ using Pins = IO::AltPinList<{0, 1},
            Pb9, Pd15>; };
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

