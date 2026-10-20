/**
 * @file
 * @brief Implements SPI protocol for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_SPI_H
#define ZHELE_PLATFORM_CH32_X0_SPI_H

#include <ch32x035.h>

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
        struct Spi1ClockPins { using io_pins = IO::PinList<IO::Pa5, IO::Pb15, IO::Pa11, IO::Pc5>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3}; };
        IO_STRUCT_WRAPPER(SPI1, Spi1Regs, SPI_TypeDef);
    }

    using Spi1 = Private::Spi<
        Private::Spi1Regs,
        Clock::Spi1Clock,
        Dma1Channel3,
        Dma1Channel2,
        Private::Spi1ClockPins>;
}

#endif // ZHELE_PLATFORM_CH32_X0_SPI_H
