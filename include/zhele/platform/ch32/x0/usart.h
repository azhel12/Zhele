/**
 * @file
 * @brief Implements USART protocol for ch32x035 series
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */
#ifndef ZHELE_PLATFORM_CH32_X0_USART_H
#define ZHELE_PLATFORM_CH32_X0_USART_H

#include <ch32x035.h>

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
        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pb10, IO::Pa10, IO::Pa7>; static constexpr uint8_t alt_functions[] = {0, 1, 3}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pb11, IO::Pa11, IO::Pb2>; static constexpr uint8_t alt_functions[] = {0, 1, 3}; };
        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);

        struct Usart2TxPins { using io_pins = IO::PinList<IO::Pa2, IO::Pa20, IO::Pa15, IO::Pc0>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3}; };
        struct Usart2RxPins { using io_pins = IO::PinList<IO::Pa3, IO::Pa19, IO::Pa16, IO::Pc1>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3}; };
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);

        struct Usart3TxPins { using io_pins = IO::PinList<IO::Pb3, IO::Pc18, IO::Pa18, IO::Pb16>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3}; };
        struct Usart3RxPins { using io_pins = IO::PinList<IO::Pb4, IO::Pc19, IO::Pb14, IO::Pb17>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3}; };
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);

        struct Usart4TxPins { using io_pins = IO::PinList<IO::Pb0, IO::Pa5, IO::Pc16, IO::Pb9, IO::Pb13, IO::Pc17>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3, 4, 7}; };
        struct Usart4RxPins { using io_pins = IO::PinList<IO::Pb1, IO::Pa9, IO::Pc17, IO::Pa10, IO::Pc19, IO::Pc16>; static constexpr uint8_t alt_functions[] = {0, 1, 2, 3, 4, 7}; };
        IO_STRUCT_WRAPPER(USART4, Usart4Regs, USART_TypeDef);
    }

    using Usart1 = Private::Usart<
        Private::Usart1Regs,
        USART1_IRQn,
        Clock::Usart1Clock,
        Private::Usart1TxPins,
        Private::Usart1RxPins,
        Dma1Channel4,
        Dma1Channel5>;

    using Usart2 = Private::Usart<
        Private::Usart2Regs,
        USART2_IRQn,
        Clock::Usart2Clock,
        Private::Usart2TxPins,
        Private::Usart2RxPins,
        Dma1Channel7,
        Dma1Channel6>;

    using Usart3 = Private::Usart<
        Private::Usart3Regs,
        USART3_IRQn,
        Clock::Usart3Clock,
        Private::Usart3TxPins,
        Private::Usart3RxPins,
        Dma1Channel2,
        Dma1Channel3>;

    using Usart4 = Private::Usart<
        Private::Usart4Regs,
        USART4_IRQn,
        Clock::Usart4Clock,
        Private::Usart4TxPins,
        Private::Usart4RxPins,
        Dma1Channel1,
        Dma1Channel8>;
}

#endif // ZHELE_PLATFORM_CH32_X0_USART_H
