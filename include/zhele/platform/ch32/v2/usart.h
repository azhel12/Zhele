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
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/usart.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        // USART1_REMAP: 0 -> TX/PA9 RX/PA10 (default), 1 -> TX/PB6 RX/PB7.
        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pa9, IO::Pb6>; static constexpr uint8_t alt_functions[] = {0, 1}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pa10, IO::Pb7>; static constexpr uint8_t alt_functions[] = {0, 1}; };
        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
#if defined(USART2)
        // USART2_REMAP: 0 -> TX/PA2 RX/PA3 (default), 1 -> TX/PD5 RX/PD6.
        struct Usart2TxPins { using io_pins = IO::PinList<IO::Pa2, IO::Pd5>; static constexpr uint8_t alt_functions[] = {0, 1}; };
        struct Usart2RxPins { using io_pins = IO::PinList<IO::Pa3, IO::Pd6>; static constexpr uint8_t alt_functions[] = {0, 1}; };
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
#endif
#if defined(USART3)
        // USART3_REMAP[1:0]: 0 -> TX/PB10 RX/PB11 (default), 1 -> TX/PC10 RX/PC11, 3 -> TX/PD8 RX/PD9.
        struct Usart3TxPins { using io_pins = IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8>; static constexpr uint8_t alt_functions[] = {0, 1, 3}; };
        struct Usart3RxPins { using io_pins = IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9>; static constexpr uint8_t alt_functions[] = {0, 1, 3}; };
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
#endif
    }

    using Usart1 = Private::Usart<
        Private::Usart1Regs,
        USART1_IRQn,
        Clock::Usart1Clock,
        Private::Usart1TxPins,
        Private::Usart1RxPins,
        Dma1Channel4,
        Dma1Channel5>;

#if defined(USART2)
    using Usart2 = Private::Usart<
        Private::Usart2Regs,
        USART2_IRQn,
        Clock::Usart2Clock,
        Private::Usart2TxPins,
        Private::Usart2RxPins,
        Dma1Channel7,
        Dma1Channel6>;
#endif

#if defined(USART3)
    using Usart3 = Private::Usart<
        Private::Usart3Regs,
        USART3_IRQn,
        Clock::Usart3Clock,
        Private::Usart3TxPins,
        Private::Usart3RxPins,
        Dma1Channel2,
        Dma1Channel3>;
#endif
}

#endif // ZHELE_PLATFORM_CH32_V2_USART_H
