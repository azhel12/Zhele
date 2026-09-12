/**
 * @file
 * K1921VG7T: instantiate UART instances Usart0..Usart1 (PL011, same IP as K1921VG015).
 *
 * Pin options (РП таблица 3.2; the number in brackets is the ALTFUNCNUM value —
 * on this family the value *is* the AF number printed in the table):
 *   UART0 RX: Pa0 (AF2), Pa8  (AF1)   TX: Pa1 (AF2), Pa9  (AF1)
 *   UART1 RX: Pa2 (AF2), Pa10 (AF1)   TX: Pa3 (AF2), Pa11 (AF1)
 *
 * Note Pa10 doubles as TRST and Pa2 as SERVEN, both configured as alternate
 * functions out of reset — prefer the Pa0/Pa1 (UART0) and Pa3/Pa11 (UART1) sets
 * unless the board says otherwise.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG7T_USART_H
#define ZHELE_PLATFORM_NIIET_K1921VG7T_USART_H

#include <K1921VG7T.h>

#include "../common/usart.h"

#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <array>
#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        struct Usart0TxPins { using io_pins = IO::PinList<IO::Pa1, IO::Pa9>;  static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };
        struct Usart0RxPins { using io_pins = IO::PinList<IO::Pa0, IO::Pa8>;  static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };

        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pa3, IO::Pa11>; static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pa2, IO::Pa10>; static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };

        IO_STRUCT_WRAPPER(UART0, Uart0Regs, UART_TypeDef);
        IO_STRUCT_WRAPPER(UART1, Uart1Regs, UART_TypeDef);
    } // namespace Private

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart0 = Private::Usart<Private::Uart0Regs, IsrVect_IRQ_UART0, Clock::Uart0Clock,
                                  Private::Usart0TxPins, Private::Usart0RxPins, _DmaTx, _DmaRx>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart1 = Private::Usart<Private::Uart1Regs, IsrVect_IRQ_UART1, Clock::Uart1Clock,
                                  Private::Usart1TxPins, Private::Usart1RxPins, _DmaTx, _DmaRx>;
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG7T_USART_H
