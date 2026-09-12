/**
 * @file
 * K1921VG5T: instantiate UART instances Usart0..Usart1 (PL011, same IP as K1921VG015).
 *
 * Pin options (РП таблица 3.2; the number in brackets is the ALTFUNCNUM value —
 * on this family the value *is* the AF number printed in the table):
 *   UART0 RX: Pb0 (AF2), Pb8  (AF1)   TX: Pb1 (AF2), Pb9  (AF1)
 *   UART1 RX: Pa8 (AF2), Pb10 (AF2)   TX: Pa9 (AF2), Pb11 (AF2)
 *
 * Pb8/Pb9 with AF1 is what the NIIET SDK retarget uses for its console.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG5T_USART_H
#define ZHELE_PLATFORM_NIIET_K1921VG5T_USART_H

#include <K1921VG5T.h>

#include "../common/usart.h"

#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <array>
#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        struct Usart0TxPins { using io_pins = IO::PinList<IO::Pb1, IO::Pb9>;  static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };
        struct Usart0RxPins { using io_pins = IO::PinList<IO::Pb0, IO::Pb8>;  static constexpr std::array<uint8_t, 2> alt_functions{2, 1}; };

        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pa9, IO::Pb11>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pa8, IO::Pb10>; static constexpr std::array<uint8_t, 2> alt_functions{2, 2}; };

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

#endif // ZHELE_PLATFORM_NIIET_K1921VG5T_USART_H
