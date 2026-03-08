/**
 * @file
 * Implement i2c protocol for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2020
 * @license FreeBSD
 */

#ifndef ZHELE_I2C_H
#define ZHELE_I2C_H

#include "../common/i2c.h"
#include "../common/template_utils/static_array.h"

#include "dma.h"
#include "iopins.h"

#include <utility>

namespace Zhele
{
    namespace Private
    {
        I2C_TEMPLATE_ARGS
        template<unsigned sclPinNumber, unsigned sdaPinNumber>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            using SclAltFuncNumbers = typename _SclPins::second_type;
            using SdaAltFuncNumbers = typename _SdaPins::second_type;

            using SclPin = typename _SclPins::first_type::template Pin<sclPinNumber>;
            SclPin::Port::Enable();
            SclPin::template SetConfiguration<SclPin::Port::AltFunc>();
            SclPin::template AltFuncNumber<GetNonTypeValueByIndex<sclPinNumber, SclAltFuncNumbers>::value>();
            SclPin::SetDriverType(SclPin::Port::OpenDrain);
            SclPin::SetPullMode(SclPin::PullMode::PullUp);

            using SdaPin = typename _SdaPins::first_type::template Pin<sdaPinNumber>;
            if constexpr (!std::is_same_v<typename SdaPin::Port, typename SclPin::Port>)
            {
                SdaPin::Port::Enable();
            }
            SdaPin::template SetConfiguration<SdaPin::Port::AltFunc>();
            SdaPin::template AltFuncNumber<GetNonTypeValueByIndex<sdaPinNumber, SdaAltFuncNumbers>::value>();
            SdaPin::SetDriverType(SdaPin::Port::OpenDrain);
            SdaPin::SetPullMode(SdaPin::PullMode::PullUp);
        }

        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::SelectPins(uint8_t sclPinNumber, uint8_t sdaPinNumber)
        {
            using SclAltFuncNumbers = typename _SclPins::second_type;
            using SdaAltFuncNumbers = typename _SdaPins::second_type;

            using SclPins = typename _SclPins::first_type;
            using SdaPins = typename _SdaPins::first_type;

            using Type = typename SclPins::DataType;

            SclPins::Enable();
            Type maskScl(1 << sclPinNumber);
            SclPins::SetConfiguration(SclPins::AltFunc, maskScl);
            SclPins::AltFuncNumber(GetNumberRuntime<SclAltFuncNumbers>::Get(sclPinNumber), maskScl);
            SclPins::SetDriverType(SclPins::OpenDrain, maskScl);
            SclPins::SetPullMode(SclPins::PullMode::PullUp, maskScl);

            SdaPins::Enable();
            Type maskSda(1 << sdaPinNumber);
            SdaPins::SetConfiguration(SdaPins::AltFunc, maskSda);
            SdaPins::AltFuncNumber(GetNumberRuntime<SdaAltFuncNumbers>::Get(sdaPinNumber), maskSda);
            SdaPins::SetDriverType(SdaPins::OpenDrain, maskSda);
            SdaPins::SetPullMode(SdaPins::PullMode::PullUp, maskSda);
        }
        
        I2C_TEMPLATE_ARGS
        template<typename SclPin, typename SdaPin>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            const int sclPinIndex = _SclPins::first_type:: template IndexOf<SclPin>;
            const int sdaPinIndex = _SdaPins::first_type:: template IndexOf<SdaPin>;
            
            static_assert(sclPinIndex >= 0);
            static_assert(sdaPinIndex >= 0);

            SelectPins<sclPinIndex, sdaPinIndex>();
        }

        using I2C1SclPins = std::pair<IO::PinList<IO::Pa9, IO::Pa11, IO::Pb6, IO::Pb8, IO::Pb10>, NonTypeTemplateArray<4, 5, 1, 1, 1>>;
        using I2C1SdaPins = std::pair<IO::PinList<IO::Pa10, IO::Pa12, IO::Pb7, IO::Pb9, IO::Pb11>, NonTypeTemplateArray<4, 5, 1, 1, 1>>;

        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
    }
    using I2c1 = Private::I2cBase<Private::I2C1Regs, I2C1_IRQn, I2C1_IRQn, Clock::I2c1Clock, Private::I2C1SclPins, Private::I2C1SdaPins, Dma1Channel2, Dma1Channel3>;
}

#endif //! ZHELE_I2C_H