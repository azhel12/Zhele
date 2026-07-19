/**
 * @file
 * CH32V00x: instantiate the USART peripherals.
 * Default pins: USART1 TX=PD5 RX=PD6, USART2 TX=PA7 RX=PB3.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_USART_H
#define ZHELE_PLATFORM_CH32_V0_USART_H

#include <ch32v00x.h>

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
    }

    using Usart1 = Private::Usart<
        Private::Usart1Regs,
        USART1_IRQn,
        Clock::Usart1Clock,
        AFIO_PCFR1_USART1_REMAP,
        6,
        Dma1Channel4,
        Dma1Channel5>;

#if defined(USART2)
    using Usart2 = Private::Usart<
        Private::Usart2Regs,
        USART2_IRQn,
        Clock::Usart2Clock,
        AFIO_PCFR1_USART2_REMAP,
        20,
        Dma1Channel7,
        Dma1Channel6>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V0_USART_H
