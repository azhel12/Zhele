/**
 * @file
 * CH32V00x: instantiate SPI1 (remap auto-derived from the SCK pin).
 * Default: SCK=PC5 MISO=PC7 MOSI=PC6 NSS=PC1.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_SPI_H
#define ZHELE_PLATFORM_CH32_V0_SPI_H

#include <ch32v00x.h>

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

        struct Spi1ClockPins
        {
#if defined(CH32V00X)
            // V00x SPI1_RM[2:0]: SCK is PC5(0/1), PD2(2), PB1(3), PD4(4), PA1(5).
            using io_pins = IO::PinList<IO::Pc5, IO::Pd2, IO::Pb1, IO::Pd4, IO::Pa1>;
            static constexpr uint8_t alt_functions[] = {0, 2, 3, 4, 5};
#else
            // V003: SCK is PC5 for both remap values (0/1 differ only in NSS).
            using io_pins = IO::PinList<IO::Pc5>;
            static constexpr uint8_t alt_functions[] = {0};
#endif
        };
    }

    using Spi1 = Private::Spi<
        Private::Spi1Regs, Clock::Spi1Clock,
        Dma1Channel3, Dma1Channel2,
        Private::Spi1ClockPins>;
}

#endif // ZHELE_PLATFORM_CH32_V0_SPI_H
