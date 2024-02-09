 /**
 * @file
 * Implements SPI protocol for stm32f0 series
 * 
 * @author Aleksei Zhelonkin
 * @date 2020
 * @licence FreeBSD
 */

#ifndef ZHELE_SPI_H
#define ZHELE_SPI_H

#include <stm32f0xx.h>

#include "../common/spi.h"
#include "../common/template_utils/pair.h"
#include "../common/template_utils/static_array.h"

#include "dma.h"

#define COMMA ,

namespace Zhele
{
    namespace Private
    {
        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins(int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber)
        {
            using MosiAltFuncNumbers = typename _MosiPins::Value;
            using MisoAltFuncNumbers = typename _MisoPins::Value;
            using ClockAltFuncNumbers = typename _ClockPins::Value;
            using SsAltFuncNumbers = typename _SsPins::Value;

            using MosiPins = typename _MosiPins::Key;
            using MisoPins = typename _MisoPins::Key;
            using ClockPins = typename _ClockPins::Key;
            using SsPins = typename _SsPins::Key;

            using Type = typename MosiPins::DataType;

            if(mosiPinNumber != -1)
            {
                MosiPins::Enable();
                Type maskMosi(1 << mosiPinNumber);
                MosiPins::SetConfiguration(MosiPins::AltFunc, maskMosi);
                MosiPins::SetDriverType(MosiPins::DriverType::PushPull, maskMosi);
                MosiPins::AltFuncNumber(GetNumberRuntime<MosiAltFuncNumbers>::Get(mosiPinNumber), maskMosi);
            }

            if(misoPinNumber != -1)
            {
                MisoPins::Enable();
                Type maskMiso(1 << misoPinNumber);
                MisoPins::SetConfiguration(MisoPins::In, maskMiso);
                MisoPins::AltFuncNumber(GetNumberRuntime<MisoAltFuncNumbers>::Get(maskMiso), maskMiso);
            }

            ClockPins::Enable();
            Type maskClock(1 << clockPinNumber);
            ClockPins::SetConfiguration(ClockPins::AltFunc, maskClock);
            ClockPins::SetDriverType(ClockPins::DriverType::PushPull, maskClock);
            ClockPins::AltFuncNumber(GetNumberRuntime<ClockAltFuncNumbers>::Get(maskClock), maskClock);
            
            if(ssPinNumber != -1)
            {
                SsPins::Enable();
                Type maskSs(1 << ssPinNumber);
                SsPins::SetConfiguration(SsPins::AltFunc, maskSs);
                SsPins::SetDriverType(SsPins::DriverType::PushPull, maskSs);
                SsPins::AltFuncNumber(GetNumberRuntime<SsAltFuncNumbers>::Get(maskSs), maskSs);
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<int8_t mosiPinNumber, int8_t misoPinNumber, int8_t clockPinNumber, int8_t ssPinNumber>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            using MosiAltFuncNumbers = typename _MosiPins::Value;
            using MisoAltFuncNumbers = typename _MisoPins::Value;
            using ClockAltFuncNumbers = typename _ClockPins::Value;
            using SsAltFuncNumbers = typename _SsPins::Value;

            using MosiPin = std::conditional_t<mosiPinNumber != -1, typename _MosiPins::Key::template Pin<mosiPinNumber>, typename IO::NullPin>;
            using MisoPin = std::conditional_t<misoPinNumber != -1, typename _MisoPins::Key::template Pin<misoPinNumber>, typename IO::NullPin>;
            using ClockPin = typename _ClockPins::Key::template Pin<clockPinNumber>;
            using SsPin = std::conditional_t<ssPinNumber != -1, typename _SsPins::Key::template Pin<ssPinNumber>, typename IO::NullPin>;

            constexpr auto usedPorts = TypeList<typename MosiPin::Port, typename MisoPin::Port, typename ClockPin::Port, typename SsPin::Port>::remove_duplicates();
            usedPorts.foreach([](auto port) { port.Enable(); });

            if constexpr(mosiPinNumber != -1)
            {
                MosiPin::template SetConfiguration<MosiPin::Port::AltFunc>();
                MosiPin::template SetDriverType<MosiPin::DriverType::PushPull>();
                MosiPin::template AltFuncNumber<GetNonTypeValueByIndex<mosiPinNumber, MosiAltFuncNumbers>::value>();
            }

            if constexpr(misoPinNumber != -1)
            {
                MisoPin::template SetConfiguration<MisoPin::Port::AltFunc>();
                MisoPin::template AltFuncNumber<GetNonTypeValueByIndex<misoPinNumber, MisoAltFuncNumbers>::value>();
            }

            ClockPin::template SetConfiguration<ClockPin::Port::AltFunc>();
            ClockPin::template SetDriverType<ClockPin::DriverType::PushPull>();
            ClockPin::template AltFuncNumber<GetNonTypeValueByIndex<clockPinNumber, ClockAltFuncNumbers>::value>();

            if constexpr(ssPinNumber != -1)
            {
                SsPin::template SetConfiguration<SsPin::Port::AltFunc>();
                SsPin::template SetDriverType<SsPin::DriverType::PushPull>;
                SsPin::template AltFuncNumber<GetNonTypeValueByIndex<ssPinNumber, SsAltFuncNumbers>::value>();
            }
        }

        template<typename _Regs, typename _Clock, typename _MosiPins, typename _MisoPins, typename _ClockPins, typename _SsPins, typename _DmaTx, typename _DmaRx>
        template<typename MosiPin, typename MisoPin, typename ClockPin, typename SsPin>
        void Spi<_Regs, _Clock, _MosiPins, _MisoPins, _ClockPins, _SsPins, _DmaTx, _DmaRx>::SelectPins()
        {
            constexpr auto mosiPinIndex = !std::is_same_v<MosiPin, IO::NullPin>
                                ? _MosiPins::Key::template IndexOf<MosiPin>
                                : -1;
            constexpr auto misoPinIndex = !std::is_same_v<MisoPin, IO::NullPin>
                                ? _MisoPins::Key:: template IndexOf<MisoPin>
                                : -1;
            constexpr auto clockPinIndex = _ClockPins::Key:: template IndexOf<ClockPin>;

            constexpr auto ssPinIndex = !std::is_same_v<MisoPin, IO::NullPin>
                                ? _SsPins::Key:: template IndexOf<SsPin>
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

        using Spi1SsPins = Pair<IO::PinList<IO::Pa4, IO::Pa15, IO::Pe12>, NonTypeTemplateArray<0, 0, 1>>;
		using Spi1ClockPins = Pair<IO::PinList<IO::Pa5, IO::Pb3, IO::Pe13>, NonTypeTemplateArray<0, 0, 1>>;
		using Spi1MisoPins = Pair<IO::PinList<IO::Pa6, IO::Pb4, IO::Pe14>, NonTypeTemplateArray<0, 0, 1>>;
		using Spi1MosiPins = Pair<IO::PinList<IO::Pa7, IO::Pb5, IO::Pe15>, NonTypeTemplateArray<0, 0, 1>>;
		
		using Spi2SsPins = Pair<IO::PinList<IO::Pb12, IO::Pb9, IO::Pd0>, NonTypeTemplateArray<0, 5, 1>>;
		using Spi2ClockPins = Pair<IO::PinList<IO::Pb13, IO::Pb10, IO::Pd1>, NonTypeTemplateArray<0, 5, 1>>;
		using Spi2MisoPins = Pair<IO::PinList<IO::Pb14, IO::Pc2, IO::Pd3>, NonTypeTemplateArray<0, 1, 1>>;
		using Spi2MosiPins = Pair<IO::PinList<IO::Pb15, IO::Pc3, IO::Pd4>, NonTypeTemplateArray<0, 1, 1>>;
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

#endif //! ZHELE_SPI_H