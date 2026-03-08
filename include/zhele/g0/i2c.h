/**
 * @file
 * Implement i2c protocol for stm32g0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2024
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

        using I2c1SclPins = std::pair<IO::PinList<IO::Pa9, IO::Pb6, IO::Pb8>, NonTypeTemplateArray<6, 6, 6>>;
        using I2c1SdaPins = std::pair<IO::PinList<IO::Pa10, IO::Pb7, IO::Pb9>, NonTypeTemplateArray<6, 6, 6>>;

        using I2c2SclPins = std::pair<IO::PinList<IO::Pa1, IO::Pb10, IO::Pb13>, NonTypeTemplateArray<6, 6, 6>>;
        using I2c2SdaPins = std::pair<IO::PinList<IO::Pa12, IO::Pb11, IO::Pb14>, NonTypeTemplateArray<6, 6, 6>>;

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

#endif //! ZHELE_I2C_H