/**
 * @file
 * CH32V20x: instantiate the I2C peripherals.
 * Default pins: I2C1 SCL=PB6 SDA=PB7, I2C2 SCL=PB10 SDA=PB11.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_I2C_H
#define ZHELE_PLATFORM_CH32_V2_I2C_H

#include <ch32v20x.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include "../common/i2c.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        using I2C1SclPins = IO::PinList<IO::Pb6>;
        using I2C1SdaPins = IO::PinList<IO::Pb7>;

        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
#if defined(I2C2)
        using I2C2SclPins = IO::PinList<IO::Pb10>;
        using I2C2SdaPins = IO::PinList<IO::Pb11>;

        IO_STRUCT_WRAPPER(I2C2, I2C2Regs, I2C_TypeDef);
#endif
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

#if defined(I2C2)
    using I2c2 = Private::I2cBase<
        Private::I2C2Regs,
        I2C2_EV_IRQn,
        I2C2_ER_IRQn,
        Clock::I2c2Clock,
        Private::I2C2SclPins,
        Private::I2C2SdaPins,
        Dma1Channel4,
        Dma1Channel5>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_I2C_H
