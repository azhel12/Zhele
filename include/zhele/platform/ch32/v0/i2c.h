/**
 * @file
 * CH32V00x: instantiate the I2C1 peripheral.
 * Default pins: SCL=PC2 SDA=PC1.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_I2C_H
#define ZHELE_PLATFORM_CH32_V0_I2C_H

#include <ch32v00x.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include "../common/i2c.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        using I2C1SclPins = IO::PinList<IO::Pc2, IO::Pd1, IO::Pc5>;
        using I2C1SdaPins = IO::PinList<IO::Pc1, IO::Pd0, IO::Pc6>;

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

#endif // ZHELE_PLATFORM_CH32_V0_I2C_H
