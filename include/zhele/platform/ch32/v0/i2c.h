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
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/i2c.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
#if defined(CH32V00X)
        // V00x I2C1_RM[2:0]: only the PC2/PC1 default pair is wired here (remap 0).
        struct I2C1SclPins { using io_pins = IO::PinList<IO::Pc2>; static constexpr uint8_t alt_functions[] = {0}; };
        struct I2C1SdaPins { using io_pins = IO::PinList<IO::Pc1>; static constexpr uint8_t alt_functions[] = {0}; };
#else
        // V003 I2C1 remap (SplitRemapField): 0 -> PC2/PC1, 1 -> PD1/PD0, 2 -> PC5/PC6.
        struct I2C1SclPins { using io_pins = IO::PinList<IO::Pc2, IO::Pd1, IO::Pc5>; static constexpr uint8_t alt_functions[] = {0, 1, 2}; };
        struct I2C1SdaPins { using io_pins = IO::PinList<IO::Pc1, IO::Pd0, IO::Pc6>; static constexpr uint8_t alt_functions[] = {0, 1, 2}; };
#endif

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
