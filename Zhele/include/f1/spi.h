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

#include "remap.h"
#include <clock.h>
#include <iopins.h>

namespace Zhele
{
    namespace Private
    {
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins>::SelectPins(unsigned mosiPinNumber, unsigned misoPinNumber, unsigned clockPinNumber, unsigned ssPinNumber)
        {
            if(!(mosiPinNumber == misoPinNumber && mosiPinNumber == clockPinNumber && mosiPinNumber == ssPinNumber))
                return;

            using Type = typename _MosiPins::DataType;

            _MosiPins::Enable();
            Type maskMosi (1 << mosiPinNumber);
            _MosiPins::SetConfiguration(maskMosi, _MosiPins::AltFunc);

            _MisoPins::Enable();
            Type maskMiso (1 << misoPinNumber);
            _MisoPins::SetConfiguration(maskMiso, _MisoPins::AltFunc);
            
            _ClockPins::Enable();
            Type maskSck (1 << clockPinNumber);
            _ClockPins::SetConfiguration(maskSck, _ClockPins::AltFunc);
            
            _SsPins::Enable();
            Type maskNss (1 << ssPinNumber);
            _SsPins::SetConfiguration(maskNss, _SsPins::AltFunc);

            Clock::AfioClock::Enable();
            if(mosiPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_Clock>::Set(1);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins>
        template<unsigned mosiPinNumber, unsigned misoPinNumber, unsigned clockPinNumber, unsigned ssPinNumber>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins>::SelectPins()
        {
            static_assert(mosiPinNumber == misoPinNumber && mosiPinNumber == clockPinNumber && mosiPinNumber == ssPinNumber);
            static_assert(mosiPinNumber <= 1);

            using MosiPin = typename GetType<mosiPinNumber, typename _MosiPins::PinsAsTypeList>::type;
            MosiPin::Port::Enable();
            MosiPin::SetConfiguration(MosiPin::Configuration::AltFunc);
        
            using MisoPin = typename GetType<misoPinNumber, typename _MisoPins::PinsAsTypeList>::type;
            MisoPin::Port::Enable();
            MisoPin::SetConfiguration(MisoPin::Configuration::AltFunc);

            using ClockPin = typename GetType<clockPinNumber, typename _ClockPins::PinsAsTypeList>::type;
            ClockPin::Port::Enable();
            ClockPin::SetConfiguration(ClockPin::Configuration::AltFunc);

            using SsPin = typename GetType<ssPinNumber, typename _SsPins::PinsAsTypeList>::type;
            SsPin::Port::Enable();
            SsPin::SetConfiguration(SsPin::Configuration::AltFunc);

            Clock::AfioClock::Enable();
            if constexpr (mosiPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_Clock>::Set(1);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins>
        template<typename MosiPin, typename MisoPin, typename ClockPin, typename SsPin>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins>::SelectPins()
        {
            const int mosiPinIndex = TypeIndex<MosiPin, typename _MosiPins::PinsAsTypeList>::value;
            const int misoPinIndex = TypeIndex<MisoPin, typename _MisoPins::PinsAsTypeList>::value;
            const int clockPinIndex = TypeIndex<ClockPin, typename _ClockPins::PinsAsTypeList>::value;
            const int ssPinIndex = TypeIndex<SsPin, typename _SsPins::PinsAsTypeList>::value;

            static_assert(mosiPinIndex >= 0);
            static_assert(misoPinIndex >= 0);
            static_assert(clockPinIndex >= 0);
            static_assert(ssPinIndex >= 0);

            SelectPins<mosiPinIndex, misoPinIndex, clockPinIndex, ssPinIndex>();
        }

        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
		IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
        #if defined (SPI3)
            IO_STRUCT_WRAPPER(SPI3, Spi3Regs, SPI_TypeDef);
        #endif

        using Spi1SsPins = IO::PinList<IO::Pa4, IO::Pa15>;
		using Spi1ClockPins = IO::PinList<IO::Pa5, IO::Pb3>;
		using Spi1MisoPins = IO::PinList<IO::Pa6, IO::Pb4>;
		using Spi1MosiPins = IO::PinList<IO::Pa7, IO::Pb5>;
		
		using Spi2SsPins = IO::PinList<IO::Pb12>;
		using Spi2ClockPins = IO::PinList<IO::Pb13>;
		using Spi2MisoPins = IO::PinList<IO::Pb14>;
		using Spi2MosiPins = IO::PinList<IO::Pb15>;

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
        Private::Spi1SsPins>;

    using Spi2 = Private::Spi<
        Private::Spi2Regs, 
        Clock::Spi2Clock, 
        Private::Spi2MosiPins, 
        Private::Spi2MisoPins,
        Private::Spi2ClockPins,
        Private::Spi2SsPins>;

    #if defined(SPI3)
        using Spi3 = Private::Spi<
            Private::Spi3Regs, 
            Clock::Spi3Clock, 
            Private::Spi3MosiPins, 
            Private::Spi3MisoPins,
            Private::Spi3ClockPins,
            Private::Spi3SsPins>;
    #endif
    
}

#endif //! ZHELE_SPI_H