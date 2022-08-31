 /**
 * @file
 * Implements SPI protocol for stm32f1 series
 * 
 * @author Aleksei Zhelonkin
 * @date 2019
 * @licence FreeBSD
 */

#ifndef ZHELE_SPI_H
#define ZHELE_SPI_H

#include <stm32f1xx.h>

#include "../common/spi.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "remap.h"

namespace Zhele
{
    namespace Private
    {
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins(int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber)
        {
            if(!(mosiPinNumber == misoPinNumber && mosiPinNumber == clockPinNumber && mosiPinNumber == ssPinNumber))
                return;

            using Type = typename _MosiPins::DataType;

            if(mosiPinNumber != -1)
            {
                _MosiPins::Enable();
                Type maskMosi (1 << mosiPinNumber);
                _MosiPins::SetConfiguration(maskMosi, _MosiPins::AltFunc);
                _MosiPins::SetDriverType(maskMosi, _MosiPins::DriverType::PushPull);
            }

            if(misoPinNumber != -1)
            {
                _MisoPins::Enable();
                Type maskMiso (1 << misoPinNumber);
                _MisoPins::SetConfiguration(maskMiso, _MisoPins::AltFunc);
            }
            
            _ClockPins::Enable();
            Type maskClock (1 << clockPinNumber);
            _ClockPins::SetConfiguration(maskClock, _ClockPins::AltFunc);
            _ClockPins::SetDriverType(maskClock, _ClockPins::DriverType::PushPull);
            
            if(ssPinNumber != -1)
            {
                _SsPins::Enable();
                Type maskSs (1 << ssPinNumber);
                _SsPins::SetConfiguration(maskSs, _SsPins::AltFunc);
                _SsPins::SetDriverType(maskSs, _SsPins::DriverType::PushPull);
            }

            Clock::AfioClock::Enable();
            if(clockPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_Clock>::Set(1);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            static_assert((clockPinNumber == mosiPinNumber || mosiPinNumber == -1)
                && (clockPinNumber == misoPinNumber || misoPinNumber == -1)
                && (clockPinNumber == ssPinNumber || ssPinNumber == -1));

            static_assert(clockPinNumber <= 1);

            using MosiPin = std::conditional_t<mosiPinNumber != -1, typename GetType<mosiPinNumber, typename _MosiPins::PinsAsTypeList>::type, typename IO::NullPin>;
            using MisoPin = std::conditional_t<misoPinNumber != -1, typename GetType<misoPinNumber, typename _MisoPins::PinsAsTypeList>::type, typename IO::NullPin>;
            using ClockPin = typename GetType<clockPinNumber, typename _ClockPins::PinsAsTypeList>::type;
            using SsPin = std::conditional_t<ssPinNumber != -1, typename GetType<ssPinNumber, typename _SsPins::PinsAsTypeList>::type, typename IO::NullPin>;
            
            using usedPorts = Zhele::IO::PortList<typename TemplateUtils::Unique<TypeList<typename MosiPin::Port, typename MisoPin::Port, typename ClockPin::Port, typename SsPin::Port>>::type>;
            usedPorts::Enable();

            if constexpr(mosiPinNumber != -1)
            {
                MosiPin::template SetConfiguration<MosiPin::Configuration::AltFunc>();
                MosiPin::template SetDriverType<MosiPin::DriverType::PushPull>();
            }

            if constexpr(misoPinNumber != -1)
            {
                MisoPin::template SetConfiguration<MisoPin::Configuration::AltFunc>();
            }

            ClockPin::template SetConfiguration<ClockPin::Configuration::AltFunc>();
            ClockPin::template SetDriverType<ClockPin::DriverType::PushPull>();

            if constexpr(ssPinNumber != -1)
            {
                SsPin::template SetConfiguration<SsPin::Configuration::AltFunc>();
                SsPin::template SetDriverType<SsPin::DriverType::PushPull>();
            }

            Clock::AfioClock::Enable();
            
            if constexpr (ssPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_Clock>::Set(1);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<typename MosiPin, typename MisoPin, typename ClockPin, typename SsPin>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            const int8_t mosiPinIndex = !std::is_same_v<MosiPin, IO::NullPin>
                                ? TypeIndex<MosiPin, typename _MosiPins::PinsAsTypeList>::value
                                : -1;
            const int8_t misoPinIndex = !std::is_same_v<MisoPin, IO::NullPin>
                                ? TypeIndex<MisoPin, typename _MisoPins::PinsAsTypeList>::value
                                : -1;
            const int8_t clockPinIndex = TypeIndex<ClockPin, typename _ClockPins::PinsAsTypeList>::value;
            const int8_t ssPinIndex = !std::is_same_v<SsPin, IO::NullPin>
                                ? TypeIndex<SsPin, typename _SsPins::PinsAsTypeList>::value
                                : -1;

            static_assert(mosiPinIndex >= -1);
            static_assert(misoPinIndex >= -1);
            static_assert(clockPinIndex >= 0);
            static_assert(ssPinIndex >= -1);

            SelectPins<mosiPinIndex, misoPinIndex, clockPinIndex, ssPinIndex>();
        }

        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
    #if defined (SPI2)
        IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
    #endif
    #if defined (SPI3)
        IO_STRUCT_WRAPPER(SPI3, Spi3Regs, SPI_TypeDef);
    #endif

        using Spi1SsPins = IO::PinList<IO::Pa4, IO::Pa15>;
		using Spi1ClockPins = IO::PinList<IO::Pa5, IO::Pb3>;
		using Spi1MisoPins = IO::PinList<IO::Pa6, IO::Pb4>;
		using Spi1MosiPins = IO::PinList<IO::Pa7, IO::Pb5>;
	#if defined (SPI2)
		using Spi2SsPins = IO::PinList<IO::Pb12>;
		using Spi2ClockPins = IO::PinList<IO::Pb13>;
		using Spi2MisoPins = IO::PinList<IO::Pb14>;
		using Spi2MosiPins = IO::PinList<IO::Pb15>;
    #endif
    #if defined (SPI3)
        using Spi3SsPins = IO::PinList<IO::Pa15, IO::Pa4>;
        using Spi3ClockPins = IO::PinList<IO::Pb3, IO::Pc10>;
        using Spi3MisoPins = IO::PinList<IO::Pb4, IO::Pc11>;
        using Spi3MosiPins = IO::PinList<IO::Pb5, IO::Pc12>;
    #endif
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
#if defined(SPI3)
    using Spi3 = Private::Spi<
        Private::Spi3Regs, 
        Clock::Spi3Clock, 
        Private::Spi3MosiPins, 
        Private::Spi3MisoPins,
        Private::Spi3ClockPins,
        Private::Spi3SsPins,
        Dma2Channel2,
        Dma2Channel1>;
#endif
    
}

#endif //! ZHELE_SPI_H