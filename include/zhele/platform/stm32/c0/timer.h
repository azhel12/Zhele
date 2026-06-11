/**
 * @file
 * @brief Macros, templates for timers for stm32c0 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_C0_TIMER_H
#define ZHELE_PLATFORM_STM32_C0_TIMER_H

#include <stm32c0xx.h>

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

        // Channel pin / alt-function tables per STM32C011 datasheet (DS13866, Tables 13-19).
        // Pin types come from common/iopins.h, so the full family superset is listed here;
        // the bigger C0 parts (C031/C051/C071/C091) keep the same TIM3/TIM14 AF mapping.
        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa6, Pb6, Pb7>; static constexpr std::array<uint8_t, 3> alt_functions{1, 12, 11}; }; };
        template<> struct Tim3ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa7, Pb6, Pc14>; static constexpr std::array<uint8_t, 3> alt_functions{1, 13, 11}; }; };
        template<> struct Tim3ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa8, Pb6, Pc15>; static constexpr std::array<uint8_t, 3> alt_functions{11, 3, 3}; }; };
        template<> struct Tim3ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa8, Pb7>; static constexpr std::array<uint8_t, 2> alt_functions{12, 3}; }; };

        template<unsigned ChannelNumber> struct Tim14ChPins;
        template<> struct Tim14ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa4, Pa7, Pa8>; static constexpr std::array<uint8_t, 3> alt_functions{4, 4, 13}; }; };

        // TIM2 is present only on the larger C0 parts (C051/C071/C091). Its channel
        // pin/AF table is left empty until that datasheet mapping is on hand — declare
        // pins explicitly via the register API meanwhile (mirrors the g0 TIM4 stub).
        template<unsigned ChannelNumber> struct Tim2ChPins;

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
    #if defined (TIM2)
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
    #endif
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
    #if defined (TIM14)
        IO_STRUCT_WRAPPER(TIM14, Tim14Regs, TIM_TypeDef);
    #endif
    #if defined (TIM15)
        IO_STRUCT_WRAPPER(TIM15, Tim15Regs, TIM_TypeDef);
    #endif
    #if defined (TIM16)
        IO_STRUCT_WRAPPER(TIM16, Tim16Regs, TIM_TypeDef);
    #endif
    #if defined (TIM17)
        IO_STRUCT_WRAPPER(TIM17, Tim17Regs, TIM_TypeDef);
    #endif
    }

#if defined (TIM2)
    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
#endif
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
#if defined (TIM14)
    using Timer14 = Private::GPTimer<Private::Tim14Regs, Clock::Tim14Clock, TIM14_IRQn, Private::Tim14ChPins>;
#endif
#if defined (TIM15)
    using Timer15 = Private::BaseTimer<Private::Tim15Regs, Clock::Tim15Clock, TIM15_IRQn>;
#endif
#if defined (TIM16)
    using Timer16 = Private::BaseTimer<Private::Tim16Regs, Clock::Tim16Clock, TIM16_IRQn>;
#endif
#if defined (TIM17)
    using Timer17 = Private::BaseTimer<Private::Tim17Regs, Clock::Tim17Clock, TIM17_IRQn>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_C0_TIMER_H
