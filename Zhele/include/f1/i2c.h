/**
 * @file
 * Implement i2c protocol for stm32f1 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_I2C_H
#define ZHELE_I2C_H

#include "../common/i2c.h"

#include "dma.h"
#include "iopins.h"
#include "remap.h"

namespace Zhele
{
    namespace Private
    {
        I2C_TEMPLATE_ARGS
        void I2C_TEMPLATE_QUALIFIER::SelectPins(uint8_t sclPinNumber, uint8_t sdaPinNumber)
        {
            if(sclPinNumber != sdaPinNumber)
                return;

            using Type = typename SclPins::DataType;

            _SclPins::Enable();
            Type maskScl(1 << sclPinNumber);
            SclPins::SetConfiguration(maskScl, SclPins::AltFunc);
            SclPins::SetDriverType(maskScl, SclPins::OpenDrain);

            _SdaPins::Enable();
            Type maskSda(1 << sdaPinNumber);
            SdaPins::SetConfiguration(maskSda, SdaPins::AltFunc);
            SdaPins::SetDriverType(maskSda, SdaPins::OpenDrain);

            Clock::AfioClock::Enable();
            if(sclPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_ClockCtrl>::Set(1);
            }
        }
        
        I2C_TEMPLATE_ARGS
        template<unsigned sclPinNumber, unsigned sdaPinNumber>
        void I2C_TEMPLATE_QUALIFIER::SelectPins()
        {
            using SclPin = typename _SclPins::template Pin<sclPinNumber>;
            SclPin::Port::Enable();
            SclPin::template SetConfiguration<SclPin::Port::AltFunc>();
            SclPin::template SetDriverType<SclPin::Port::OpenDrain>();

            using SdaPin = typename _SdaPins::template Pin<sdaPinNumber>;
            if constexpr (!std::is_same_v<typename SdaPin::Port, typename SclPin::Port>)
            {
                SdaPin::Port::Enable();
            }
            SdaPin::template SetConfiguration<SdaPin::Port::AltFunc>();
            SdaPin::template SetDriverType<SdaPin::Port::OpenDrain>();

            if constexpr(sclPinNumber == 1)
            {
                Zhele::IO::Private::PeriphRemap<_ClockCtrl>::Set(1);
            }
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
        IO_STRUCT_WRAPPER(I2C1, I2C1Regs, I2C_TypeDef);

    #if defined (I2C2)
        using I2C2SclPins = IO::PinList<IO::Pb10>;
        using I2C2SdaPins = IO::PinList<IO::Pb11>;
        IO_STRUCT_WRAPPER(I2C2, I2C2Regs, I2C_TypeDef);
        using I2c2 = I2cBase<I2C2Regs, I2C2_EV_IRQn, I2C2_ER_IRQn, Clock::I2c2Clock, I2C2SclPins, I2C2SdaPins, void, void>;
    #endif
    }

    using I2c1 = Private::I2cBase<Private::I2C1Regs, I2C1_EV_IRQn, I2C1_ER_IRQn, Clock::I2c1Clock, Private::I2C1SclPins, Private::I2C1SdaPins, Dma1Channel6, Dma1Channel7>;
#if defined (I2C2)
    using I2c2 = Private::I2cBase<Private::I2C2Regs, I2C2_EV_IRQn, I2C2_ER_IRQn, Clock::I2c2Clock, Private::I2C2SclPins, Private::I2C2SdaPins, Dma1Channel4, Dma1Channel5>;
#endif
}

#endif //! ZHELE_I2C_H