/**
 * @file
 * CH32V20x: instantiate the USART peripherals.
 * Default pins: USART1 TX=PA9 RX=PA10, USART2 TX=PA2 RX=PA3, USART3 TX=PB10 RX=PB11.
 */
#ifndef ZHELE_PLATFORM_CH32_V2_USART_H
#define ZHELE_PLATFORM_CH32_V2_USART_H

#include <ch32v20x.h>

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include "../common/usart.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
#if defined(USART2)
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
#endif
#if defined(USART3)
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
#endif
    }

    using Usart1 = Private::Usart<
        Private::Usart1Regs,
        USART1_IRQn,
        Clock::Usart1Clock,
        AFIO_PCFR1_USART1_REMAP,
        2,
        Dma1Channel4,
        Dma1Channel5>;

#if defined(USART2)
    using Usart2 = Private::Usart<
        Private::Usart2Regs,
        USART2_IRQn,
        Clock::Usart2Clock,
        AFIO_PCFR1_USART2_REMAP,
        3,
        Dma1Channel7,
        Dma1Channel6>;
#endif

#if defined(USART3)
    using Usart3 = Private::Usart<
        Private::Usart3Regs,
        USART3_IRQn,
        Clock::Usart3Clock,
        AFIO_PCFR1_USART3_REMAP,
        4,
        Dma1Channel2,
        Dma1Channel3>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_USART_H
