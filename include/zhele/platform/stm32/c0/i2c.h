/**
 * @file
 * @brief Implements I2C protocol for stm32c0 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_C0_I2C_H
#define ZHELE_PLATFORM_STM32_C0_I2C_H

#include <stm32c0xx.h>

#include "../common/i2c.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include <array>
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

        // Pin/alt-function tables per STM32C011 datasheet (DS13866), I2C1 on AF6.
        // C011/C031 expose only I2C1; the larger C0 parts (C051/C071/C091) also
        // have I2C2 — add its tables here once their datasheet AF mapping is on hand.
        struct I2c1SclPins
        {
            using io_pins = IO::PinList<IO::Pa9, IO::Pb6>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };
        struct I2c1SdaPins
        {
            using io_pins = IO::PinList<IO::Pa10, IO::Pb7>;
            static constexpr std::array<uint8_t, 2> alt_functions{6, 6};
        };

        IO_STRUCT_WRAPPER(I2C1, I2c1Regs, I2C_TypeDef);
    }

// TODO:: Implement DMAMUX for use DMA
    // I2C1 IRQ is a single combined line on C0 (event and error share it).
    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c1 = Private::I2cBase<Private::I2c1Regs, I2C1_IRQn, I2C1_IRQn, Clock::I2c1Clock, Private::I2c1SclPins, Private::I2c1SdaPins, _DmaTx, _DmaRx>;
    using I2c1NoDma = I2c1<>;
}

#endif //! ZHELE_PLATFORM_STM32_C0_I2C_H
