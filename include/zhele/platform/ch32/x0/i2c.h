/**
 * @file
 * @brief Implements I2C protocol for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_I2C_H
#define ZHELE_PLATFORM_CH32_X0_I2C_H

#include <ch32x035.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/i2c.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        struct I2C1SclPins { using io_pins = IO::PinList<IO::Pa10, IO::Pa13, IO::Pc16, IO::Pc19, IO::Pc17, IO::Pc18>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3, 4, 5}; };
        struct I2C1SdaPins { using io_pins = IO::PinList<IO::Pa11, IO::Pa14, IO::Pc17, IO::Pc18, IO::Pc16, IO::Pc19>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3, 4, 5}; };
        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
    }

    using I2c1 = Private::I2cBase<
        Private::I2C1Regs,
        I2C1_EV_IRQn,
        I2C1_ER_IRQn,
        Clock::I2c1Clock,
        Private::I2C1SclPins,
        Private::I2C1SdaPins,
        Dma1Channel6,
        Dma1Channel7>;
}

#endif // ZHELE_PLATFORM_CH32_X0_I2C_H
