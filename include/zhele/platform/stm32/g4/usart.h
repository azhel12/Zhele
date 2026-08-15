/**
 * @file
 * @brief Implements USART protocol for stm32g4 series
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_USART_H
#define ZHELE_PLATFORM_STM32_G4_USART_H

#include <stm32g4xx.h>

#include "../common/usart.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Zhele
{
    namespace Private
    {
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins(int8_t txPinNumber, int8_t rxPinNumber)
        {
            using TxPins = typename _TxPins::io_pins;
            using RxPins = typename _RxPins::io_pins;

            using Type = typename TxPins::DataType;

            TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            TxPins::SetConfiguration(TxPins::Configuration::AltFunc, maskTx);
            TxPins::AltFuncNumber(_TxPins::alt_functions[static_cast<size_t>(txPinNumber)], maskTx);

            if(rxPinNumber != -1)
            {
                RxPins::Enable();
                Type maskRx(1 << rxPinNumber);
                RxPins::SetConfiguration(RxPins::Configuration::AltFunc, maskRx);
                RxPins::AltFuncNumber(_RxPins::alt_functions[static_cast<size_t>(rxPinNumber)], maskRx);
            }
        }

        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        template<int8_t TxPinNumber, int8_t RxPinNumber>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            using TxPin = typename _TxPins::io_pins::template Pin<TxPinNumber>;

            TxPin::Port::Enable();

            TxPin::template SetConfiguration<TxPin::Port::Configuration::AltFunc>();
            TxPin::template AltFuncNumber<_TxPins::alt_functions[TxPinNumber]>();

            if constexpr(RxPinNumber != -1)
            {
                using RxPin = typename _RxPins::io_pins::template Pin<RxPinNumber>;

                if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>) {
                    RxPin::Port::Enable();
                }

                RxPin::template SetConfiguration<RxPin::Port::Configuration::AltFunc>();
                RxPin::template AltFuncNumber<_RxPins::alt_functions[RxPinNumber]>();
            }
        }

        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        template<typename TxPin, typename RxPin>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            const int8_t txPinIndex = _TxPins::io_pins:: template IndexOf<TxPin>;
            const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                ? _RxPins::io_pins:: template IndexOf<RxPin>
                                : -1;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= -1);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
        }

        struct Usart1TxPins
        {
            using io_pins = IO::PinList<IO::Pa9, IO::Pb6, IO::Pc4, IO::Pe0>;
            static constexpr std::array<uint8_t, 4> alt_functions{7, 7, 7, 7};
        };
        struct Usart1RxPins
        {
            using io_pins = IO::PinList<IO::Pa10, IO::Pb7, IO::Pc5, IO::Pe1>;
            static constexpr std::array<uint8_t, 4> alt_functions{7, 7, 7, 7};
        };

        struct Usart2TxPins
        {
            using io_pins = IO::PinList<IO::Pa2, IO::Pa14, IO::Pb3, IO::Pd5>;
            static constexpr std::array<uint8_t, 4> alt_functions{7, 7, 7, 7};
        };
        struct Usart2RxPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa15, IO::Pb4, IO::Pd6>;
            static constexpr std::array<uint8_t, 4> alt_functions{7, 7, 7, 7};
        };

        struct Usart3TxPins
        {
            using io_pins = IO::PinList<IO::Pb9, IO::Pb10, IO::Pc10, IO::Pd8>;
            static constexpr std::array<uint8_t, 4> alt_functions{7, 7, 7, 7};
        };
        struct Usart3RxPins
        {
            using io_pins = IO::PinList<IO::Pb8, IO::Pb11, IO::Pc11, IO::Pd9, IO::Pe15>;
            static constexpr std::array<uint8_t, 5> alt_functions{7, 7, 7, 7, 7};
        };

#if defined (UART4)
        struct Uart4TxPins
        {
            using io_pins = IO::PinList<IO::Pc10>;
            static constexpr std::array<uint8_t, 1> alt_functions{5};
        };
        struct Uart4RxPins
        {
            using io_pins = IO::PinList<IO::Pc11>;
            static constexpr std::array<uint8_t, 1> alt_functions{5};
        };
#endif

#if defined (UART5)
        struct Uart5TxPins
        {
            using io_pins = IO::PinList<IO::Pc12>;
            static constexpr std::array<uint8_t, 1> alt_functions{5};
        };
        struct Uart5RxPins
        {
            using io_pins = IO::PinList<IO::Pd2>;
            static constexpr std::array<uint8_t, 1> alt_functions{5};
        };
#endif

        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
    #if defined(UART4)
        IO_STRUCT_WRAPPER(UART4, Uart4Regs, USART_TypeDef);
    #endif
    #if defined(UART5)
        IO_STRUCT_WRAPPER(UART5, Uart5Regs, USART_TypeDef);
    #endif
    }

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart1 = Private::Usart<Private::Usart1Regs, USART1_IRQn, Clock::Usart1Clock, Private::Usart1TxPins, Private::Usart1RxPins, _DmaTx, _DmaRx>;
    
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart2 = Private::Usart<Private::Usart2Regs, USART2_IRQn, Clock::Usart2Clock, Private::Usart2TxPins, Private::Usart2RxPins, _DmaTx, _DmaRx>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart3 = Private::Usart<Private::Usart3Regs, USART3_IRQn, Clock::Usart3Clock, Private::Usart3TxPins, Private::Usart3RxPins, _DmaTx, _DmaRx>;

#if defined (UART4)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Uart4 = Private::Usart<Private::Uart4Regs, UART4_IRQn, Clock::Uart4Clock, Private::Uart4TxPins, Private::Uart4RxPins, _DmaTx, _DmaRx>;
#endif

#if defined (UART5)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Uart5 = Private::Usart<Private::Uart5Regs, UART5_IRQn, Clock::Uart5Clock, Private::Uart5TxPins, Private::Uart5RxPins, _DmaTx, _DmaRx>;
#endif

    // LPUART1 is not supported yet: its baud rate register uses a different formula
    // (BRR = 256 * fck / baud, minimum 0x300) which UsartBase::SetBaud does not implement.
}

#endif //! ZHELE_PLATFORM_STM32_G4_USART_H
