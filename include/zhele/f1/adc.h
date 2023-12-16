/**
 * @file ADC implementation for stm32f1 series.
 *
 * @author Alexey Zhelonkin
 * @date 2021
 * @licence FreeBSD
 */

#ifndef ZHELE_ADC_H
#define ZHELE_ADC_H

#include <clock.h>
#include <dma.h>
#include <iopins.h>
#include <pinlist.h>

#include "../common/adc.h"

namespace Zhele
{
    namespace Private
    {
        template <typename _Regs, typename _ClockCtrl, typename _InputPins, typename _DmaChannel>
        class Adc : public AdcBase<_Regs, _ClockCtrl, _InputPins, _DmaChannel>
        {
        public:
            // External trigger for regular channels
            enum class RegularTrigger : uint8_t
            {
                Timer1TRGO = 0, //< Timer 1 TRGO
                Timer1CC4, //< Timer 1 CC4
                Timer2TRGO, //< Timer 2 TRGO
                Timer3TRGO, //< Timer 3 TRGO
                Timer15TRGO, //< Timer 15 TRGO
            };

            // Trigger mode
            enum class TriggerMode
            {
                Disabled, //< Trigger detection disabled,
                RisingFalling, // Detection on both rising and falling edge
            };
        };

        using Adc1Pins = Zhele::IO::PinList<
            Zhele::IO::Pa0,
            Zhele::IO::Pa1,
            Zhele::IO::Pa2,
            Zhele::IO::Pa3,
            Zhele::IO::Pa4,
            Zhele::IO::Pa5,
            Zhele::IO::Pa6,
            Zhele::IO::Pa7,
            Zhele::IO::Pb0,
            Zhele::IO::Pb1,
            Zhele::IO::Pc0,
            Zhele::IO::Pc1,
            Zhele::IO::Pc2,
            Zhele::IO::Pc3,
            Zhele::IO::Pc4,
            Zhele::IO::Pc5>;

        IO_STRUCT_WRAPPER(ADC1, Adc1Regs, ADC_TypeDef);
    }

    using Adc1 = Private::Adc<Private::Adc1Regs, Clock::Adc1Clock, Private::Adc1Pins, Dma1Channel1>;
}

#endif //! ZHELE_ADC_H