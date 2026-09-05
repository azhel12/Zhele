/**
 * @file
 * Macros, templates for timers for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_TIMER_H
#define ZHELE_PLATFORM_STM32_H5_TIMER_H

#include <stm32h5xx.h>

#include "iopins.h"
#include "../common/timer.h"

#include <array>
#include <cstddef>
#include <cstdint>

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
            Pins::SetConfiguration(mask, Pins::Configuration::AltFunc);
            Pins::AltFuncNumber(mask, PinsConfig::alt_functions[static_cast<size_t>(pinNumber)]);
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

        using namespace Zhele::IO;

        template<unsigned ChannelNumber> struct Tim1ChPins;
        template<> struct Tim1ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa8, Pa13, Pb1, Pb7, Pc6>; static constexpr std::array<uint8_t, 5> alt_functions{1, 1, 14, 14, 1}; }; };
        template<> struct Tim1ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa9, Pa14, Pb4, Pb6, Pc7>; static constexpr std::array<uint8_t, 5> alt_functions{1, 1, 14, 14, 1}; }; };
        template<> struct Tim1ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa1, Pa10, Pb5, Pc8>; static constexpr std::array<uint8_t, 4> alt_functions{14, 1, 1, 1}; }; };
        template<> struct Tim1ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa2, Pa11, Pc9, Pc12>; static constexpr std::array<uint8_t, 4> alt_functions{14, 1, 1, 14}; }; };

        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa0, Pa5, Pa15, Pb2>; static constexpr std::array<uint8_t, 4> alt_functions{1, 1, 1, 14}; }; };
        template<> struct Tim2ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa1, Pb3, Pc11>; static constexpr std::array<uint8_t, 3> alt_functions{1, 1, 1}; }; };
        template<> struct Tim2ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa2, Pa7, Pb10, Pd2>; static constexpr std::array<uint8_t, 4> alt_functions{1, 14, 1, 1}; }; };
        template<> struct Tim2ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa3, Pa12, Pc4, Pc12>; static constexpr std::array<uint8_t, 4> alt_functions{1, 14, 1, 1}; }; };

        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa0, Pa6, Pa14, Pb4, Pc6>; static constexpr std::array<uint8_t, 5> alt_functions{2, 2, 2, 2, 2}; }; };
        template<> struct Tim3ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa7, Pa11, Pb5, Pc7>; static constexpr std::array<uint8_t, 4> alt_functions{2, 2, 2, 2}; }; };
        template<> struct Tim3ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa8, Pb0, Pb6, Pc8>; static constexpr std::array<uint8_t, 4> alt_functions{2, 2, 2, 2}; }; };
        template<> struct Tim3ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa12, Pb1, Pb15, Pc9>; static constexpr std::array<uint8_t, 4> alt_functions{2, 2, 14, 2}; }; };

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
    #if defined (TIM2)
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
    #endif
    #if defined (TIM3)
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
    #endif
    #if defined (TIM6)
        IO_STRUCT_WRAPPER(TIM6, Tim6Regs, TIM_TypeDef);
    #endif
    #if defined (TIM7)
        IO_STRUCT_WRAPPER(TIM7, Tim7Regs, TIM_TypeDef);
    #endif
    }

    // TIM1 spreads its events over four vectors; the capture/compare one is what
    // the generic timer code raises, so it is the one bound here.
    using Timer1 = Private::GPTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_CC_IRQn, Private::Tim1ChPins>;
#if defined (TIM2)
    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
#endif
#if defined (TIM3)
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
#endif
#if defined (TIM6)
    using Timer6 = Private::BaseTimer<Private::Tim6Regs, Clock::Tim6Clock, TIM6_IRQn>;
#endif
#if defined (TIM7)
    using Timer7 = Private::BaseTimer<Private::Tim7Regs, Clock::Tim7Clock, TIM7_IRQn>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_H5_TIMER_H
