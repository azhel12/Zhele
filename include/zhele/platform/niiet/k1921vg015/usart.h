/**
 * @file
 * Instantiate UART instances Usart0..Usart4.
 */
#ifndef ZHELE_PLATFORM_NIIET_K1921VG015_USART_H
#define ZHELE_PLATFORM_NIIET_K1921VG015_USART_H

#include <K1921VG015.h>

#include "../common/usart.h"

#include <zhele/dma.h>
#include <zhele/iopins.h>

#include <array>
#include <cstdint>

namespace Zhele
{
    namespace Private
    {
        struct Usart0TxPins { using io_pins = IO::PinList<IO::Pa1, IO::Pb7>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Usart0RxPins { using io_pins = IO::PinList<IO::Pa0, IO::Pb6>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Usart1TxPins { using io_pins = IO::PinList<IO::Pa3, IO::Pa11>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Usart1RxPins { using io_pins = IO::PinList<IO::Pa2, IO::Pa10>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Usart2TxPins { using io_pins = IO::PinList<IO::Pa5, IO::Pa13>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Usart2RxPins { using io_pins = IO::PinList<IO::Pa4, IO::Pa12>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Usart3TxPins { using io_pins = IO::PinList<IO::Pa7, IO::Pa15>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Usart3RxPins { using io_pins = IO::PinList<IO::Pa6, IO::Pa14>; static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        struct Usart4TxPins { using io_pins = IO::PinList<IO::Pa9, IO::Pb1>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };
        struct Usart4RxPins { using io_pins = IO::PinList<IO::Pa8, IO::Pb0>;  static constexpr std::array<uint8_t, 2> alt_functions{1, 2}; };

        IO_STRUCT_WRAPPER(UART0, Uart0Regs, UART_TypeDef);
    #if defined(UART1)
        IO_STRUCT_WRAPPER(UART1, Uart1Regs, UART_TypeDef);
    #endif
    #if defined(UART2)
        IO_STRUCT_WRAPPER(UART2, Uart2Regs, UART_TypeDef);
    #endif
    #if defined(UART3)
        IO_STRUCT_WRAPPER(UART3, Uart3Regs, UART_TypeDef);
    #endif
    #if defined(UART4)
        IO_STRUCT_WRAPPER(UART4, Uart4Regs, UART_TypeDef);
    #endif
    } // namespace Private

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart0 = Private::Usart<Private::Uart0Regs, IsrVect_IRQ_UART0, Clock::Uart0Clock,
                                  Private::Usart0TxPins, Private::Usart0RxPins, _DmaTx, _DmaRx>;

#if defined(UART1)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart1 = Private::Usart<Private::Uart1Regs, IsrVect_IRQ_UART1, Clock::Uart1Clock,
                                  Private::Usart1TxPins, Private::Usart1RxPins, _DmaTx, _DmaRx>;
#endif
#if defined(UART2)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart2 = Private::Usart<Private::Uart2Regs, IsrVect_IRQ_UART2, Clock::Uart2Clock,
                                  Private::Usart2TxPins, Private::Usart2RxPins, _DmaTx, _DmaRx>;
#endif
#if defined(UART3)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart3 = Private::Usart<Private::Uart3Regs, IsrVect_IRQ_UART3, Clock::Uart3Clock,
                                  Private::Usart3TxPins, Private::Usart3RxPins, _DmaTx, _DmaRx>;
#endif
#if defined(UART4)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart4 = Private::Usart<Private::Uart4Regs, IsrVect_IRQ_UART4, Clock::Uart4Clock,
                                  Private::Usart4TxPins, Private::Usart4RxPins, _DmaTx, _DmaRx>;
#endif
}

#endif // ZHELE_PLATFORM_NIIET_K1921VG015_USART_H
