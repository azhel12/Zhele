/**
 * @file
 * @brief Macros, templates for timers for stm32l4 series
 * @author Alexey Zhelonkin
 * @date 2022
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_L4_TIMER_H
#define ZHELE_PLATFORM_STM32_L4_TIMER_H

#include <stm32l4xx.h>

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

        using namespace Zhele::IO;
        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa0, Pa5, Pa15>; static constexpr std::array<uint8_t, 3> alt_functions{1, 1, 1}; }; };
        template<> struct Tim2ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa1, Pb3>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };
        template<> struct Tim2ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa2, Pb10>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };
        template<> struct Tim2ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa3, Pb11>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };
        
        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa6, Pb4, Pc6>; static constexpr std::array<uint8_t, 3> alt_functions{2, 2, 2}; }; };
        template<> struct Tim3ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa7, Pb5, Pc7>; static constexpr std::array<uint8_t, 3> alt_functions{2, 2, 2}; }; };
        template<> struct Tim3ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pb0, Pc8>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim3ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pb1, Pc9>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        
        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pb6, Pd12>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim4ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pb7, Pd13>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim4ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pb8, Pd14>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim4ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pb9, Pd15>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        
        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM4, Tim4Regs, TIM_TypeDef);
    }

    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
    using Timer4 = Private::GPTimer<Private::Tim4Regs, Clock::Tim4Clock, TIM4_IRQn, Private::Tim4ChPins>;
}

#endif //! ZHELE_PLATFORM_STM32_L4_TIMER_H