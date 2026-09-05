/**
 * @file
 * Implements USART protocol for stm32h5 series
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_H5_USART_H
#define ZHELE_PLATFORM_STM32_H5_USART_H

#include <stm32h5xx.h>

// Unlike G0/C0, the H5 CMSIS headers already provide both the classic
// (USART_ISR_TXE, USART_CR1_TXEIE, ...) and the FIFO spellings, so no
// compatibility defines are needed here.

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
            using io_pins = IO::PinList<IO::Pa2, IO::Pa9, IO::Pa12, IO::Pa14, IO::Pb6, IO::Pb14>;
            static constexpr std::array<uint8_t, 6> alt_functions{8, 7, 8, 7, 7, 4};
        };
        struct Usart1RxPins
        {
            using io_pins = IO::PinList<IO::Pa1, IO::Pa10, IO::Pa11, IO::Pa13, IO::Pb7, IO::Pb15>;
            static constexpr std::array<uint8_t, 6> alt_functions{8, 7, 8, 7, 7, 4};
        };

        struct Usart2TxPins
        {
            using io_pins = IO::PinList<IO::Pa2, IO::Pa5, IO::Pa8, IO::Pa12, IO::Pa14, IO::Pb0, IO::Pb4, IO::Pc6>;
            static constexpr std::array<uint8_t, 8> alt_functions{7, 9, 4, 4, 9, 9, 13, 13};
        };
        struct Usart2RxPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa11, IO::Pa13, IO::Pa15, IO::Pb1, IO::Pb5, IO::Pb15, IO::Pc7>;
            static constexpr std::array<uint8_t, 8> alt_functions{7, 4, 9, 9, 9, 13, 13, 13};
        };

        struct Usart3TxPins
        {
            using io_pins = IO::PinList<IO::Pa4, IO::Pa8, IO::Pb3, IO::Pb7, IO::Pb10, IO::Pc10>;
            static constexpr std::array<uint8_t, 6> alt_functions{13, 13, 13, 13, 7, 7};
        };
        struct Usart3RxPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa5, IO::Pa12, IO::Pa15, IO::Pb8, IO::Pc4, IO::Pc11>;
            static constexpr std::array<uint8_t, 7> alt_functions{13, 13, 13, 13, 13, 7, 7};
        };

        struct LpUart1TxPins
        {
            using io_pins = IO::PinList<IO::Pa3, IO::Pa9, IO::Pb6, IO::Pc3>;
            static constexpr std::array<uint8_t, 4> alt_functions{3, 3, 8, 3};
        };
        struct LpUart1RxPins
        {
            using io_pins = IO::PinList<IO::Pa2, IO::Pa10, IO::Pb7, IO::Pb15>;
            static constexpr std::array<uint8_t, 4> alt_functions{3, 3, 8, 8};
        };

        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
    #if defined(USART3)
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
    #endif
    #if defined(LPUART1)
        IO_STRUCT_WRAPPER(LPUART1, LpUart1Regs, USART_TypeDef);
    #endif
    }

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart1 = Private::Usart<Private::Usart1Regs, USART1_IRQn, Clock::Usart1Clock, Private::Usart1TxPins, Private::Usart1RxPins, _DmaTx, _DmaRx>;

    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart2 = Private::Usart<Private::Usart2Regs, USART2_IRQn, Clock::Usart2Clock, Private::Usart2TxPins, Private::Usart2RxPins, _DmaTx, _DmaRx>;

#if defined (USART3)
    template<typename _DmaTx = void, typename _DmaRx = void>
    using Usart3 = Private::Usart<Private::Usart3Regs, USART3_IRQn, Clock::Usart3Clock, Private::Usart3TxPins, Private::Usart3RxPins, _DmaTx, _DmaRx>;
#endif

#if defined (LPUART1)
    /**
     * @brief LPUART1
     *
     * @details
     * LPUART uses a different (fractional, x256) baud rate divider, so
     * Init()/SetBaud() from the generic USART code do not apply to it. Use it
     * for its register/pin definitions and program BRR yourself.
     */
    template<typename _DmaTx = void, typename _DmaRx = void>
    using LpUart1 = Private::Usart<Private::LpUart1Regs, LPUART1_IRQn, Clock::LpUart1Clock, Private::LpUart1TxPins, Private::LpUart1RxPins, _DmaTx, _DmaRx>;
#endif
}

#endif //! ZHELE_PLATFORM_STM32_H5_USART_H
