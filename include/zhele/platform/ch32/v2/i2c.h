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
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/i2c.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        // I2C1_REMAP: 0 -> SCL/PB6 SDA/PB7 (default), 1 -> SCL/PB8 SDA/PB9.
        struct I2C1SclPins { using io_pins = IO::PinList<IO::Pb6, IO::Pb8>; static constexpr uint8_t alt_functions[] = {0, 1}; };
        struct I2C1SdaPins { using io_pins = IO::PinList<IO::Pb7, IO::Pb9>; static constexpr uint8_t alt_functions[] = {0, 1}; };

        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
#if defined(I2C2)
        // I2C2 has no remap field on V20x (fixed SCL/PB10 SDA/PB11).
        struct I2C2SclPins { using io_pins = IO::PinList<IO::Pb10>; static constexpr uint8_t alt_functions[] = {0}; };
        struct I2C2SdaPins { using io_pins = IO::PinList<IO::Pb11>; static constexpr uint8_t alt_functions[] = {0}; };

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
