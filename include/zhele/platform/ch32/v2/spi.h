/**
 * @file
 * CH32V20x: instantiate the SPI peripherals.
 * Default pins: SPI1 SCK=PA5 MISO=PA6 MOSI=PA7, SPI2 SCK=PB13 MISO=PB14 MOSI=PB15.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_SPI_H
#define ZHELE_PLATFORM_CH32_V2_SPI_H

#include <ch32v20x.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"

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
    }

    using Spi1 = Private::Spi<
        Private::Spi1Regs,
        Clock::Spi1Clock,
        Dma1Channel3,
        Dma1Channel2,
        AFIO_PCFR1_SPI1_REMAP>;

#if defined(SPI2)
    using Spi2 = Private::Spi<
        Private::Spi2Regs,
        Clock::Spi2Clock,
        Dma1Channel5,
        Dma1Channel4>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_SPI_H
