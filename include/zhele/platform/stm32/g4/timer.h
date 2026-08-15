/**
 * @file
 * @brief Macros, templates for timers for stm32g4 series
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_TIMER_H
#define ZHELE_PLATFORM_STM32_G4_TIMER_H

#include <stm32g4xx.h>

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
        template<> struct Tim1ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa8, Pc0>; static constexpr std::array<uint8_t, 2> alt_functions{6, 2}; }; };
        template<> struct Tim1ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa9, Pc1>; static constexpr std::array<uint8_t, 2> alt_functions{6, 2}; }; };
        template<> struct Tim1ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa10, Pc2>; static constexpr std::array<uint8_t, 2> alt_functions{6, 2}; }; };
        template<> struct Tim1ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa11, Pc3>; static constexpr std::array<uint8_t, 2> alt_functions{11, 2}; }; };

        template<unsigned ChannelNumber> struct Tim2ChPins;
        template<> struct Tim2ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa0, Pa5, Pa15>; static constexpr std::array<uint8_t, 3> alt_functions{1, 1, 1}; }; };
        template<> struct Tim2ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa1, Pb3>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };
        template<> struct Tim2ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa2, Pb10>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };
        template<> struct Tim2ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa3, Pb11>; static constexpr std::array<uint8_t, 2> alt_functions{1, 1}; }; };

        template<unsigned ChannelNumber> struct Tim3ChPins;
        template<> struct Tim3ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa6, Pb4, Pc6>; static constexpr std::array<uint8_t, 3> alt_functions{2, 2, 2}; }; };
        template<> struct Tim3ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa4, Pa7, Pb5, Pc7>; static constexpr std::array<uint8_t, 4> alt_functions{2, 2, 2, 2}; }; };
        template<> struct Tim3ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pb0, Pc8>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim3ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pb1, Pc9>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };

        template<unsigned ChannelNumber> struct Tim4ChPins;
        template<> struct Tim4ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pb6, Pd12>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim4ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pb7, Pd13>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim4ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pd14>; static constexpr std::array<uint8_t, 1> alt_functions{2}; }; };
        template<> struct Tim4ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pb9, Pd15>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };

#if defined (TIM5)
        template<unsigned ChannelNumber> struct Tim5ChPins;
        template<> struct Tim5ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa0, Pb2>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; }; };
        template<> struct Tim5ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa1>; static constexpr std::array<uint8_t, 1> alt_functions{2}; }; };
        template<> struct Tim5ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pa2>; static constexpr std::array<uint8_t, 1> alt_functions{2}; }; };
        template<> struct Tim5ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pa3>; static constexpr std::array<uint8_t, 1> alt_functions{2}; }; };
#endif

        template<unsigned ChannelNumber> struct Tim8ChPins;
        template<> struct Tim8ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa15, Pb6, Pc6>; static constexpr std::array<uint8_t, 3> alt_functions{2, 5, 4}; }; };
        template<> struct Tim8ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa14, Pc7>; static constexpr std::array<uint8_t, 2> alt_functions{5, 4}; }; };
        template<> struct Tim8ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pb9, Pc8>; static constexpr std::array<uint8_t, 2> alt_functions{10, 4}; }; };
        template<> struct Tim8ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pc9>; static constexpr std::array<uint8_t, 1> alt_functions{4}; }; };

        template<unsigned ChannelNumber> struct Tim15ChPins;
        template<> struct Tim15ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa2, Pb14>; static constexpr std::array<uint8_t, 2> alt_functions{9, 1}; }; };
        template<> struct Tim15ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pa3, Pb15>; static constexpr std::array<uint8_t, 2> alt_functions{9, 1}; }; };

        template<unsigned ChannelNumber> struct Tim16ChPins;
        template<> struct Tim16ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa6, Pa12, Pb4, Pb8>; static constexpr std::array<uint8_t, 4> alt_functions{1, 1, 1, 1}; }; };

        template<unsigned ChannelNumber> struct Tim17ChPins;
        template<> struct Tim17ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pa7, Pb5, Pb9>; static constexpr std::array<uint8_t, 3> alt_functions{1, 10, 1}; }; };

#if defined (TIM20)
        template<unsigned ChannelNumber> struct Tim20ChPins;
        template<> struct Tim20ChPins<0>{ struct Pins { using io_pins = IO::PinList<Pb2, Pf12>; static constexpr std::array<uint8_t, 2> alt_functions{3, 2}; }; };
        template<> struct Tim20ChPins<1>{ struct Pins { using io_pins = IO::PinList<Pf13>; static constexpr std::array<uint8_t, 1> alt_functions{2}; }; };
        template<> struct Tim20ChPins<2>{ struct Pins { using io_pins = IO::PinList<Pc8, Pf14>; static constexpr std::array<uint8_t, 2> alt_functions{6, 2}; }; };
        template<> struct Tim20ChPins<3>{ struct Pins { using io_pins = IO::PinList<Pe1, Pf15>; static constexpr std::array<uint8_t, 2> alt_functions{6, 2}; }; };
#endif

        IO_STRUCT_WRAPPER(TIM1, Tim1Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM2, Tim2Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM3, Tim3Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM4, Tim4Regs, TIM_TypeDef);
    #if defined (TIM5)
        IO_STRUCT_WRAPPER(TIM5, Tim5Regs, TIM_TypeDef);
    #endif
        IO_STRUCT_WRAPPER(TIM6, Tim6Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM7, Tim7Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM8, Tim8Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM15, Tim15Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM16, Tim16Regs, TIM_TypeDef);
        IO_STRUCT_WRAPPER(TIM17, Tim17Regs, TIM_TypeDef);
    #if defined (TIM20)
        IO_STRUCT_WRAPPER(TIM20, Tim20Regs, TIM_TypeDef);
    #endif
    }

    using Timer1 = Private::AdvancedTimer<Private::Tim1Regs, Clock::Tim1Clock, TIM1_UP_TIM16_IRQn, Private::Tim1ChPins>;
    using Timer2 = Private::GPTimer<Private::Tim2Regs, Clock::Tim2Clock, TIM2_IRQn, Private::Tim2ChPins>;
    using Timer3 = Private::GPTimer<Private::Tim3Regs, Clock::Tim3Clock, TIM3_IRQn, Private::Tim3ChPins>;
    using Timer4 = Private::GPTimer<Private::Tim4Regs, Clock::Tim4Clock, TIM4_IRQn, Private::Tim4ChPins>;
#if defined (TIM5)
    using Timer5 = Private::GPTimer<Private::Tim5Regs, Clock::Tim5Clock, TIM5_IRQn, Private::Tim5ChPins>;
#endif
    using Timer6 = Private::BaseTimer<Private::Tim6Regs, Clock::Tim6Clock, TIM6_DAC_IRQn>;
    using Timer7 = Private::BaseTimer<Private::Tim7Regs, Clock::Tim7Clock, TIM7_DAC_IRQn>;
    using Timer8 = Private::AdvancedTimer<Private::Tim8Regs, Clock::Tim8Clock, TIM8_UP_IRQn, Private::Tim8ChPins>;
    using Timer15 = Private::GPTimer<Private::Tim15Regs, Clock::Tim15Clock, TIM1_BRK_TIM15_IRQn, Private::Tim15ChPins>;
    using Timer16 = Private::GPTimer<Private::Tim16Regs, Clock::Tim16Clock, TIM1_UP_TIM16_IRQn, Private::Tim16ChPins>;
    using Timer17 = Private::GPTimer<Private::Tim17Regs, Clock::Tim17Clock, TIM1_TRG_COM_TIM17_IRQn, Private::Tim17ChPins>;
#if defined (TIM20)
    using Timer20 = Private::AdvancedTimer<Private::Tim20Regs, Clock::Tim20Clock, TIM20_UP_IRQn, Private::Tim20ChPins>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_G4_TIMER_H