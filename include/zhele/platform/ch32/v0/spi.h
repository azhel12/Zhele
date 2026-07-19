/**
 * @file
 * CH32V00x: instantiate SPI1.
 * Default pins: SCK=PC5 MISO=PC7 MOSI=PC6 NSS=PC1.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_SPI_H
#define ZHELE_PLATFORM_CH32_V0_SPI_H

#include <ch32v00x.h>

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
    }

    using Spi1 = Private::Spi<
        Private::Spi1Regs,
        Clock::Spi1Clock,
        Dma1Channel3,
        Dma1Channel2,
        AFIO_PCFR1_SPI1_REMAP>;
}

#endif // ZHELE_PLATFORM_CH32_V0_SPI_H
