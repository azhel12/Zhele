/**
 * @file
 * Implement i2c protocol for stm32g0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
 * @license MIT
 */

#pragma once

#include "../common/i2c.h"

#include "dma.h"
#include "iopins.h"

#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        I2C_TEMPLATE_ARGS
        template<unsigned sclPinNumber, unsigned sdaPinNumber>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            using SclPin = typename _SclPins::io_pins::template Pin<sclPinNumber>;
            SclPin::Port::Enable();
            SclPin::template SetConfiguration<SclPin::Port::Configuration::AltFunc>();
            SclPin::template AltFuncNumber<_SclPins::alt_functions[sclPinNumber]>();
            SclPin::SetDriverType(SclPin::Port::DriverType::OpenDrain);
            SclPin::SetPullMode(SclPin::PullMode::PullUp);

            using SdaPin = typename _SdaPins::io_pins::template Pin<sdaPinNumber>;
            if constexpr (!std::is_same_v<typename SdaPin::Port, typename SclPin::Port>)
            {
                SdaPin::Port::Enable();
            }
            SdaPin::template SetConfiguration<SdaPin::Port::Configuration::AltFunc>();
            SdaPin::template AltFuncNumber<_SdaPins::alt_functions[sdaPinNumber]>();
            SdaPin::SetDriverType(SdaPin::Port::DriverType::OpenDrain);
            SdaPin::SetPullMode(SdaPin::PullMode::PullUp);
        }

        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::SelectPins(uint8_t sclPinNumber, uint8_t sdaPinNumber)
        {
            using SclPins = typename _SclPins::io_pins;
            using SdaPins = typename _SdaPins::io_pins;

            using Type = typename SclPins::DataType;

            SclPins::Enable();
            Type maskScl(1 << sclPinNumber);
            SclPins::SetConfiguration(SclPins::Configuration::AltFunc, maskScl);
            SclPins::AltFuncNumber(_SclPins::alt_functions[sclPinNumber], maskScl);
            SclPins::SetDriverType(SclPins::DriverType::OpenDrain, maskScl);
            SclPins::SetPullMode(SclPins::PullMode::PullUp, maskScl);

            SdaPins::Enable();
            Type maskSda(1 << sdaPinNumber);
            SdaPins::SetConfiguration(SdaPins::Configuration::AltFunc, maskSda);
            SdaPins::AltFuncNumber(_SdaPins::alt_functions[sdaPinNumber], maskSda);
            SdaPins::SetDriverType(SdaPins::DriverType::OpenDrain, maskSda);
            SdaPins::SetPullMode(SdaPins::PullMode::PullUp, maskSda);
        }
        
        I2C_TEMPLATE_ARGS
        template<typename SclPin, typename SdaPin>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            const int sclPinIndex = _SclPins::io_pins:: template IndexOf<SclPin>;
            const int sdaPinIndex = _SdaPins::io_pins:: template IndexOf<SdaPin>;
            
            static_assert(sclPinIndex >= 0);
            static_assert(sdaPinIndex >= 0);

            SelectPins<sclPinIndex, sdaPinIndex>();
        }

        using I2c1SclPins = IO::AltPinList<{6, 6, 6},
            IO::Pa9, IO::Pb6, IO::Pb8>;
        using I2c1SdaPins = IO::AltPinList<{6, 6, 6},
            IO::Pa10, IO::Pb7, IO::Pb9>;

        using I2c2SclPins = IO::AltPinList<{6, 6, 6},
            IO::Pa1, IO::Pb10, IO::Pb13>;
        using I2c2SdaPins = IO::AltPinList<{6, 6, 6},
            IO::Pa12, IO::Pb11, IO::Pb14>;

        IO_STRUCT_WRAPPER(I2C1, I2c1Regs, I2C_TypeDef);
        IO_STRUCT_WRAPPER(I2C2, I2c2Regs, I2C_TypeDef);
    }

// TODO:: Implement DMAMUX for use DMA
    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c1 = Private::I2cBase<Private::I2c1Regs, I2C1_IRQn, I2C1_IRQn, Clock::I2c1Clock, Private::I2c1SclPins, Private::I2c1SdaPins, _DmaTx, _DmaRx>;
    using I2c1NoDma = I2c1<>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c2 = Private::I2cBase<Private::I2c2Regs, I2C2_IRQn, I2C2_IRQn, Clock::I2c2Clock, Private::I2c2SclPins, Private::I2c2SdaPins, _DmaTx, _DmaRx>;
    using I2c2NoDma = I2c2<>;
}


