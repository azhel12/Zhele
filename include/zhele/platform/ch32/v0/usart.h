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
#include "afio.h"

#include <zhele/pinlist.h>

#include "../common/usart.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        // Only the reset-default pin pair (remap 0) is wired for V0; the remap
        // tables for the alternate pin sets need per-chip datasheet verification.
        // USART1 default: TX/PD5 RX/PD6 (V003 and V00x).
        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pd5>; static constexpr uint8_t alt_functions[] = {0}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pd6>; static constexpr uint8_t alt_functions[] = {0}; };
        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
#if defined(USART2)
        // USART2 default: TX/PA2 RX/PA3 (V00x only; V003 has no USART2).
        struct Usart2TxPins { using io_pins = IO::PinList<IO::Pa2>; static constexpr uint8_t alt_functions[] = {0}; };
        struct Usart2RxPins { using io_pins = IO::PinList<IO::Pa3>; static constexpr uint8_t alt_functions[] = {0}; };
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
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
}

#endif // ZHELE_PLATFORM_CH32_V0_USART_H
