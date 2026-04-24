 /**
 * @file
 * Implements SPI protocol for stm32f0 series
 * 
 * @author Aleksei Zhelonkin
 * @date 2020
 * @licence MIT
 */

#pragma once

#include "../common/spi.h"

#include "dma.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

#define COMMA ,

namespace Zhele
{
    namespace Private
    {
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins(int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber)
        {
            using MosiPins = typename _MosiPins::io_pins;
            using MisoPins = typename _MisoPins::io_pins;
            using ClockPins = typename _ClockPins::io_pins;
            using SsPins = typename _SsPins::io_pins;

            using Type = typename MosiPins::DataType;

            if(mosiPinNumber != -1)
            {
                MosiPins::Enable();
                Type maskMosi(1 << mosiPinNumber);
                MosiPins::SetConfiguration(MosiPins::Configuration::AltFunc, maskMosi);
                MosiPins::SetDriverType(MosiPins::DriverType::PushPull, maskMosi);
                MosiPins::AltFuncNumber(_MosiPins::alt_functions[static_cast<size_t>(mosiPinNumber)], maskMosi);
            }

            if(misoPinNumber != -1)
            {
                MisoPins::Enable();
                Type maskMiso(1 << misoPinNumber);
                MisoPins::SetConfiguration(MisoPins::Configuration::In, maskMiso);
                MisoPins::AltFuncNumber(_MisoPins::alt_functions[static_cast<size_t>(misoPinNumber)], maskMiso);
            }

            ClockPins::Enable();
            Type maskClock(1 << clockPinNumber);
            ClockPins::SetConfiguration(ClockPins::Configuration::AltFunc, maskClock);
            ClockPins::SetDriverType(ClockPins::DriverType::PushPull, maskClock);
            ClockPins::AltFuncNumber(_ClockPins::alt_functions[static_cast<size_t>(clockPinNumber)], maskClock);
            
            if(ssPinNumber != -1)
            {
                SsPins::Enable();
                Type maskSs(1 << ssPinNumber);
                SsPins::SetConfiguration(SsPins::Configuration::AltFunc, maskSs);
                SsPins::SetDriverType(SsPins::DriverType::PushPull, maskSs);
                SsPins::AltFuncNumber(_SsPins::alt_functions[static_cast<size_t>(ssPinNumber)], maskSs);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            using MosiPin = std::conditional_t<mosiPinNumber != -1, typename _MosiPins::io_pins::template Pin<mosiPinNumber>, typename IO::NullPin>;
            using MisoPin = std::conditional_t<misoPinNumber != -1, typename _MisoPins::io_pins::template Pin<misoPinNumber>, typename IO::NullPin>;
            using ClockPin = typename _ClockPins::io_pins::template Pin<clockPinNumber>;
            using SsPin = std::conditional_t<ssPinNumber != -1, typename _SsPins::io_pins::template Pin<ssPinNumber>, typename IO::NullPin>;

            constexpr auto usedPorts = template_utils::type_list<typename MosiPin::Port, typename MisoPin::Port, typename ClockPin::Port, typename SsPin::Port>::remove_duplicates();
            usedPorts.foreach([](auto port) { port.Enable(); });

            if constexpr(mosiPinNumber != -1)
            {
                MosiPin::template SetConfiguration<MosiPin::Port::Configuration::AltFunc>();
                MosiPin::template SetDriverType<MosiPin::DriverType::PushPull>();
                MosiPin::template AltFuncNumber<_MosiPins::alt_functions[mosiPinNumber]>();
            }

            if constexpr(misoPinNumber != -1)
            {
                MisoPin::template SetConfiguration<MisoPin::Port::Configuration::AltFunc>();
                MisoPin::template AltFuncNumber<_MisoPins::alt_functions[misoPinNumber]>();
            }

            ClockPin::template SetConfiguration<ClockPin::Port::Configuration::AltFunc>();
            ClockPin::template SetDriverType<ClockPin::DriverType::PushPull>();
            ClockPin::template AltFuncNumber<_ClockPins::alt_functions[clockPinNumber]>();

            if constexpr(ssPinNumber != -1)
            {
                SsPin::template SetConfiguration<SsPin::Port::Configuration::AltFunc>();
                SsPin::template SetDriverType<SsPin::DriverType::PushPull>;
                SsPin::template AltFuncNumber<_SsPins::alt_functions[ssPinNumber]>();
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<typename MosiPin, typename MisoPin, typename ClockPin, typename SsPin>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            constexpr auto mosiPinIndex = !std::is_same_v<MosiPin, IO::NullPin>
                                ? _MosiPins::io_pins::template IndexOf<MosiPin>
                                : -1;
            constexpr auto misoPinIndex = !std::is_same_v<MisoPin, IO::NullPin>
                                ? _MisoPins::io_pins:: template IndexOf<MisoPin>
                                : -1;
            constexpr auto clockPinIndex = _ClockPins::io_pins:: template IndexOf<ClockPin>;

            constexpr auto ssPinIndex = !std::is_same_v<SsPin, IO::NullPin>
                                ? _SsPins::io_pins:: template IndexOf<SsPin>
                                : -1;

            static_assert(mosiPinIndex >= -1);
            static_assert(misoPinIndex >= -1);
            static_assert(clockPinIndex >= 0);
            static_assert(ssPinIndex >= -1);

            SelectPins<mosiPinIndex, misoPinIndex, clockPinIndex, ssPinIndex>();
        }

        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
    #if defined(SPI2)
		IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
    #endif

        using Spi1SsPins = IO::AltPinList<{0, 0, 1},
            IO::Pa4, IO::Pa15, IO::Pe12>;
        using Spi1ClockPins = IO::AltPinList<{0, 0, 1},
            IO::Pa5, IO::Pb3, IO::Pe13>;
        using Spi1MisoPins = IO::AltPinList<{0, 0, 1},
            IO::Pa6, IO::Pb4, IO::Pe14>;
        using Spi1MosiPins = IO::AltPinList<{0, 0, 1},
            IO::Pa7, IO::Pb5, IO::Pe15>;
		
        using Spi2SsPins = IO::AltPinList<{0, 5, 1},
            IO::Pb12, IO::Pb9, IO::Pd0>;
        using Spi2ClockPins = IO::AltPinList<{0, 5, 1},
            IO::Pb13, IO::Pb10, IO::Pd1>;
        using Spi2MisoPins = IO::AltPinList<{0, 1, 1},
            IO::Pb14, IO::Pc2, IO::Pd3>;
        using Spi2MosiPins = IO::AltPinList<{0, 1, 1},
            IO::Pb15, IO::Pc3, IO::Pd4>;
    }
    using Spi1 = Private::Spi<
        Private::Spi1Regs, 
        Clock::Spi1Clock, 
        Private::Spi1MosiPins, 
        Private::Spi1MisoPins,
        Private::Spi1ClockPins,
        Private::Spi1SsPins,
        Dma1Channel3,
        Dma1Channel2>;

#if defined(SPI2)
    using Spi2 = Private::Spi<
        Private::Spi2Regs, 
        Clock::Spi2Clock, 
        Private::Spi2MosiPins, 
        Private::Spi2MisoPins,
        Private::Spi2ClockPins,
        Private::Spi2SsPins,
        Dma1Channel5,
        Dma1Channel4>;
#endif
}


