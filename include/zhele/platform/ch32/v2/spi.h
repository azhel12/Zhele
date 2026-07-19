/**
 * @file
 * CH32V20x: instantiate the SPI peripherals (remap auto-derived from the SCK pin).
 * SPI1 default SCK=PA5 MISO=PA6 MOSI=PA7 (remap 1: SCK=PB3 MISO=PB4 MOSI=PB5); SPI2 SCK=PB13 ...
 */
#ifndef ZHELE_PLATFORM_CH32_V2_SPI_H
#define ZHELE_PLATFORM_CH32_V2_SPI_H

#include <ch32v20x.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/spi.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
#if defined(SPI2)
        IO_STRUCT_WRAPPER(SPI2, Spi2Regs, SPI_TypeDef);
#endif

        struct Spi1ClockPins
        {
            using io_pins = IO::PinList<IO::Pa5, IO::Pb3>;
            static constexpr uint8_t alt_functions[] = {0, 1};
        };
        struct Spi2ClockPins
        {
            using io_pins = IO::PinList<IO::Pb13>;
            static constexpr uint8_t alt_functions[] = {0};
        };
    }

    using Spi1 = Private::Spi<
        Private::Spi1Regs, Clock::Spi1Clock,
        Dma1Channel3, Dma1Channel2,
        Private::Spi1ClockPins>;

#if defined(SPI2)
    using Spi2 = Private::Spi<
        Private::Spi2Regs, Clock::Spi2Clock,
        Dma1Channel5, Dma1Channel4,
        Private::Spi2ClockPins>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_SPI_H
