/**
 * @file
 * Implements SPI protocol for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @licence MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G0_SPI_H
#define ZHELE_PLATFORM_STM32_G0_SPI_H

#include "../common/spi.h"

#include "dma.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

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
                SsPin::template SetDriverType<SsPin::DriverType::PushPull>();
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
        IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
        IO_STRUCT_WRAPPER(SPI3, Spi3Regs, SPI_TypeDef);
    #if defined(SPI4)
        IO_STRUCT_WRAPPER(SPI4, Spi4Regs, SPI_TypeDef);
    #endif

        struct Spi1SsPins
        {
            using io_pins = IO::PinList<IO::Pa4, IO::Pa15>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi1ClockPins
        {
            using io_pins = IO::PinList<IO::Pa5, IO::Pb3>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi1MisoPins
        {
            using io_pins = IO::PinList<IO::Pa6, IO::Pb4>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi1MosiPins
        {
            using io_pins = IO::PinList<IO::Pa7, IO::Pb5>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };

        struct Spi2SsPins
        {
            using io_pins = IO::PinList<IO::Pb12>;
            static constexpr std::array<uint8_t, 1> alt_functions{5};
        };
        struct Spi2ClockPins
        {
            using io_pins = IO::PinList<IO::Pb13, IO::Pf9, IO::Pf10>;
            static constexpr std::array<uint8_t, 3> alt_functions{5, 5, 5};
        };
        struct Spi2MisoPins
        {
            using io_pins = IO::PinList<IO::Pa10, IO::Pb14>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi2MosiPins
        {
            using io_pins = IO::PinList<IO::Pa11, IO::Pb15>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };

        struct Spi3SsPins
        {
            using io_pins = IO::PinList<IO::Pa4, IO::Pa15>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };
        struct Spi3ClockPins
        {
            using io_pins = IO::PinList<IO::Pb3, IO::Pc10>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };
        struct Spi3MisoPins
        {
            using io_pins = IO::PinList<IO::Pb4, IO::Pc11>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };
        struct Spi3MosiPins
        {
            using io_pins = IO::PinList<IO::Pb5, IO::Pc12>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };

#if defined(SPI4)
        struct Spi4SsPins
        {
            using io_pins = IO::PinList<IO::Pe4, IO::Pe11>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi4ClockPins
        {
            using io_pins = IO::PinList<IO::Pe2, IO::Pe12>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi4MisoPins
        {
            using io_pins = IO::PinList<IO::Pe5, IO::Pe13>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
        struct Spi4MosiPins
        {
            using io_pins = IO::PinList<IO::Pe6, IO::Pe14>;
            static constexpr std::array<uint8_t, 2> alt_functions{5, 5};
        };
#endif
    }

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi1 = Private::Spi<
        Private::Spi1Regs, 
        Clock::Spi1Clock, 
        Private::Spi1MosiPins, 
        Private::Spi1MisoPins,
        Private::Spi1ClockPins,
        Private::Spi1SsPins,
        _DmaTx,
        _DmaRx>;
    using Spi1NoDma = Spi1<>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi2 = Private::Spi<
        Private::Spi2Regs, 
        Clock::Spi2Clock, 
        Private::Spi2MosiPins, 
        Private::Spi2MisoPins,
        Private::Spi2ClockPins,
        Private::Spi2SsPins,
        _DmaTx,
        _DmaRx>;
    using Spi2NoDma = Spi2<>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi3 = Private::Spi<
        Private::Spi3Regs, 
        Clock::Spi3Clock, 
        Private::Spi3MosiPins, 
        Private::Spi3MisoPins,
        Private::Spi3ClockPins,
        Private::Spi3SsPins,
        _DmaTx,
        _DmaRx>;
    using Spi3NoDma = Spi3<>;

#if defined(SPI4)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Spi4 = Private::Spi<
        Private::Spi4Regs, 
        Clock::Spi4Clock, 
        Private::Spi4MosiPins, 
        Private::Spi4MisoPins,
        Private::Spi4ClockPins,
        Private::Spi4SsPins,
        _DmaTx,
        _DmaRx>;
    using Spi4NoDma = Spi4<>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_G4_SPI_H
