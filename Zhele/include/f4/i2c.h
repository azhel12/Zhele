/**
 * @file
 * Implement i2c protocol for stm32f4 series
 * 
 * @author Alexey Zhelonkin
 * @date 2021
 * @license FreeBSD
 */

#ifndef ZHELE_I2C_H
#define ZHELE_I2C_H

#include "../common/i2c.h"

#include "../common/template_utils/pair.h"
#include "../common/template_utils/static_array.h"

#include "afio_bind.h"
#include "dma.h"
#include "iopins.h"

namespace Zhele
{
    namespace Private
    {
        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::SelectPins(uint8_t sclPinNumber, uint8_t sdaPinNumber)
        {
            using Type = typename _SclPins::DataType;

            _SclPins::Enable();
            Type maskScl(1 << sclPinNumber);
            _SclPins::SetConfiguration(maskScl, _SclPins::AltFunc);
            _SclPins::AltFuncNumber(maskScl, GetAltFunctionNumber<_Regs>);
            _SclPins::SetDriverType(maskScl, _SclPins::OpenDrain);
            _SclPins::SetPullMode(maskScl, _SclPins::PullMode::PullUp);

            _SdaPins::Enable();
            Type maskSda(1 << sdaPinNumber);
            _SdaPins::SetConfiguration(maskSda, _SdaPins::AltFunc);
            _SdaPins::AltFuncNumber(maskSda, GetAltFunctionNumber<_Regs>);
            _SdaPins::SetDriverType(maskSda, _SdaPins::OpenDrain);
            _SdaPins::SetPullMode(maskSda, _SdaPins::PullMode::PullUp);
        }
        
        I2C_TEMPLATE_ARGS
        template<unsigned sclPinNumber, unsigned sdaPinNumber>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {            
            using SclPin = typename _SclPins::template Pin<sclPinNumber>;
            SclPin::Port::Enable();
            SclPin::template SetConfiguration<SclPin::Port::AltFunc>();
            SclPin::template AltFuncNumber<GetAltFunctionNumber<_Regs>>();
            SclPin::template SetDriverType<SclPin::Port::OpenDrain>();
            SclPin::template SetPullMode<SclPin::PullMode::PullUp>();

            using SdaPin = typename _SdaPins::template Pin<sdaPinNumber>;
            if constexpr (!std::is_same_v<typename SdaPin::Port, typename SclPin::Port>)
            {
                SdaPin::Port::Enable();
            }
            SdaPin::template SetConfiguration<SdaPin::Port::AltFunc>();
            SdaPin::template AltFuncNumber<GetAltFunctionNumber<_Regs>>();
            SdaPin::template SetDriverType<SdaPin::Port::OpenDrain>();
            SdaPin::template SetPullMode<SdaPin::PullMode::PullUp>();
        }

        I2C_TEMPLATE_ARGS
        template<typename SclPin, typename SdaPin>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            const int sclPinIndex = TypeIndex<SclPin, typename _SclPins::PinsAsTypeList>::value;
            const int sdaPinIndex = TypeIndex<SdaPin, typename _SdaPins::PinsAsTypeList>::value;
            
            static_assert(sclPinIndex >= 0);
            static_assert(sdaPinIndex >= 0);

            SelectPins<sclPinIndex, sdaPinIndex>();
        }

        using I2C1SclPins = IO::PinList<IO::Pb6, IO::Pb8>;
        using I2C1SdaPins = IO::PinList<IO::Pb7, IO::Pb9>;

        using I2C2SdaPins = IO::PinList<IO::Pb10>;
        using I2C2SclPins = IO::PinList<IO::Pb11>;

        using I2C3SdaPins = IO::PinList<IO::Pa8>;
        using I2C3SclPins = IO::PinList<IO::Pa9>;

        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);
        IO_STRUCT_WRAPPER(I2C2, I2C2Regs, I2C_TypeDef);
    #if defined (I2C3)
        IO_STRUCT_WRAPPER(I2C3, I2C3Regs, I2C_TypeDef);
    #endif
    }
        using I2c1 = Private::I2cBase<Private::I2C1Regs, I2C1_EV_IRQn, I2C1_ER_IRQn, Clock::I2c1Clock, Private::I2C1SclPins, Private::I2C1SdaPins, Dma1Stream6Channel1, Dma1Stream0Channel1>;
        using I2c2 = Private::I2cBase<Private::I2C2Regs, I2C2_EV_IRQn, I2C2_ER_IRQn, Clock::I2c2Clock, Private::I2C2SclPins, Private::I2C2SdaPins, Dma1Stream7Channel7, Dma1Stream2Channel7>;
    #if defined (I2C3)
        using I2c3 = Private::I2cBase<Private::I2C3Regs, I2C3_EV_IRQn, I2C3_ER_IRQn, Clock::I2c3Clock, Private::I2C3SclPins, Private::I2C3SdaPins, Dma1Stream4Channel3, Dma1Stream2Channel3>;
    #endif
}

#endif //! ZHELE_I2C_H