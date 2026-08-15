/**
 * @file
 * Implement i2c protocol for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_I2C_H
#define ZHELE_PLATFORM_STM32_G4_I2C_H

#include "../common/i2c.h"

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

        struct I2c1SclPins
        {
            using io_pins = IO::PinList<IO::Pa13, IO::Pa15>;
            static constexpr std::array<uint8_t, 2> alt_functions{4, 4};
        };
        struct I2c1SdaPins
        {
            using io_pins = IO::PinList<IO::Pa14, IO::Pb7, IO::Pb9>;
            static constexpr std::array<uint8_t, 3> alt_functions{4, 4, 4};
        };

        struct I2c2SclPins
        {
            using io_pins = IO::PinList<IO::Pa9, IO::Pc4>;
            static constexpr std::array<uint8_t, 2> alt_functions{4, 4};
        };
        struct I2c2SdaPins
        {
            using io_pins = IO::PinList<IO::Pa8>;
            static constexpr std::array<uint8_t, 1> alt_functions{4};
        };

        struct I2c3SclPins
        {
            using io_pins = IO::PinList<IO::Pa8, IO::Pc8>;
            static constexpr std::array<uint8_t, 2> alt_functions{2, 8};
        };
        struct I2c3SdaPins
        {
            using io_pins = IO::PinList<IO::Pb5, IO::Pc9, IO::Pc11>;
            static constexpr std::array<uint8_t, 3> alt_functions{8, 8, 8};
        };

#if defined (I2C4)
        struct I2c4SclPins
        {
            using io_pins = IO::PinList<IO::Pa13, IO::Pc6>;
            static constexpr std::array<uint8_t, 2> alt_functions{3, 8};
        };
        struct I2c4SdaPins
        {
            using io_pins = IO::PinList<IO::Pb7, IO::Pc7>;
            static constexpr std::array<uint8_t, 2> alt_functions{3, 8};
        };
#endif

        IO_STRUCT_WRAPPER(I2C1, I2c1Regs, I2C_TypeDef);
        IO_STRUCT_WRAPPER(I2C2, I2c2Regs, I2C_TypeDef);
        IO_STRUCT_WRAPPER(I2C3, I2c3Regs, I2C_TypeDef);
#if defined (I2C4)
        IO_STRUCT_WRAPPER(I2C4, I2c4Regs, I2C_TypeDef);
#endif
    }

    // DMAMUX request inputs: I2c1Rx/Tx = 16/17, I2c2Rx/Tx = 18/19, I2c3Rx/Tx = 20/21, I2c4Rx/Tx = 22/23 (see dmamux.h)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c1 = Private::I2cBase<Private::I2c1Regs, I2C1_EV_IRQn, I2C1_ER_IRQn, Clock::I2c1Clock, Private::I2c1SclPins, Private::I2c1SdaPins, _DmaTx, _DmaRx>;
    using I2c1NoDma = I2c1<>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c2 = Private::I2cBase<Private::I2c2Regs, I2C2_EV_IRQn, I2C2_ER_IRQn, Clock::I2c2Clock, Private::I2c2SclPins, Private::I2c2SdaPins, _DmaTx, _DmaRx>;
    using I2c2NoDma = I2c2<>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c3 = Private::I2cBase<Private::I2c3Regs, I2C3_EV_IRQn, I2C3_ER_IRQn, Clock::I2c3Clock, Private::I2c3SclPins, Private::I2c3SdaPins, _DmaTx, _DmaRx>;
    using I2c3NoDma = I2c3<>;

#if defined (I2C4)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using I2c4 = Private::I2cBase<Private::I2c4Regs, I2C4_EV_IRQn, I2C4_ER_IRQn, Clock::I2c4Clock, Private::I2c4SclPins, Private::I2c4SdaPins, _DmaTx, _DmaRx>;
    using I2c4NoDma = I2c4<>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_G4_I2C_H
