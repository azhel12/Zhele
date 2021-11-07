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

#include "../common/template_utils/pair.h"
#include "../common/template_utils/static_array.h"

#include "dma.h"
#include "iopins.h"

namespace Zhele
{
    namespace Private
    {
        I2C_TEMPLATE_ARGS
        template<unsigned sclPinNumber, unsigned sdaPinNumber>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            using SclAltFuncNumbers = typename _SclPins::Value;
            using SdaAltFuncNumbers = typename _SdaPins::Value;

            using SclPin = typename _SclPins::Key::template Pin<sclPinNumber>;
            SclPin::Port::Enable();
            SclPin::template SetConfiguration<SclPin::Port::AltFunc>();
            SclPin::template AltFuncNumber<GetNonTypeValueByIndex<sclPinNumber, SclAltFuncNumbers>::value>();
            SclPin::SetDriverType(SclPin::Port::OpenDrain);
            SclPin::SetPullMode(SclPin::PullMode::PullUp);

            using SdaPin = typename _SdaPins::Key::template Pin<sdaPinNumber>;
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
            using SclAltFuncNumbers = typename _SclPins::Value;
            using SdaAltFuncNumbers = typename _SdaPins::Value;

            using SclPins = typename _SclPins::Key;
            using SdaPins = typename _SdaPins::Key;

            using Type = typename SclPins::DataType;

            SclPins::Enable();
            Type maskScl(1 << sclPinNumber);
            SclPins::SetConfiguration(maskScl, SclPins::AltFunc);
            SclPins::AltFuncNumber(maskScl, GetNumberRuntime<SclAltFuncNumbers>::Get(sclPinNumber));
            SclPins::SetDriverType(maskScl, SclPins::OpenDrain);
            SclPins::SetPullMode(maskScl, SclPins::PullMode::PullUp);

            SdaPins::Enable();
            Type maskSda(1 << sdaPinNumber);
            SdaPins::SetConfiguration(maskSda, SdaPins::AltFunc);
            SdaPins::AltFuncNumber(maskSda, GetNumberRuntime<SdaAltFuncNumbers>::Get(sdaPinNumber));
            SdaPins::SetDriverType(maskSda, SdaPins::OpenDrain);
            SdaPins::SetPullMode(maskSda, SdaPins::PullMode::PullUp);
        }
        
        I2C_TEMPLATE_ARGS
        template<typename SclPin, typename SdaPin>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            const int sclPinIndex = TypeIndex<SclPin, typename _SclPins::Key::PinsAsTypeList>::value;
            const int sdaPinIndex = TypeIndex<SdaPin, typename _SdaPins::Key::PinsAsTypeList>::value;
            
            static_assert(sclPinIndex >= 0);
            static_assert(sdaPinIndex >= 0);

            SelectPins<sclPinIndex, sdaPinIndex>();
        }

        using I2C1SclPins = Pair<IO::PinList<IO::Pa9, IO::Pa11, IO::Pb6, IO::Pb8, IO::Pb10>, NonTypeTemplateArray<4, 5, 1, 1, 1>>;
        using I2C1SdaPins = Pair<IO::PinList<IO::Pa10, IO::Pa12, IO::Pb7, IO::Pb9, IO::Pb11>, NonTypeTemplateArray<4, 5, 1, 1, 1>>;

        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
    }
    using I2c1 = Private::I2cBase<Private::I2C1Regs, I2C1_IRQn, I2C1_IRQn, Clock::I2c1Clock, Private::I2C1SclPins, Private::I2C1SdaPins, Dma1Channel2, Dma1Channel3>;
}

#endif //! ZHELE_I2C_H