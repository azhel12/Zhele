/**
 * @file
 * @brief Implements USART protocol for stm32f4 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_USART_H
#define ZHELE_USART_H

#include <stm32f4xx.h>

#include "../common/usart.h"

#include "afio_bind.h"
#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "../common/template_utils/static_array.h"

namespace Zhele
{
    namespace Private
    {
        /**
         * @brief Select RX and TX pins (set settings)
         * 
         * @param [in] txPinNumber pin number in Txs PinList
         * @param [in] rxPinNumber pin number in Rxs PinList
         * 
         * @par Returns
         *	Nothing
        */
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins,  _DmaTx, _DmaRx>::SelectTxRxPins(int8_t txPinNumber, int8_t rxPinNumber)
        {
            using Type = typename _TxPins::DataType;
            _TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            _TxPins::SetConfiguration(maskTx, _TxPins::AltFunc);
            _TxPins::AltFuncNumber(maskTx, GetAltFunctionNumber<_Regs>);

            if(rxPinNumber != -1)
            {
                _RxPins::Enable();
                Type maskRx(1 << rxPinNumber);
                _RxPins::SetConfiguration(maskRx, _RxPins::AltFunc);
                _RxPins::AltFuncNumber(maskRx, GetAltFunctionNumber<_Regs>);
            }
        }

        /**
         * @brief Template clone of SelectTxRxPins method
         * 
         * @tparam TxPinNumber pin number in Txs PinList
         * @tparam RxPinNumber pin number in Rxs PinList
         * 
         * @par Returns
         *	Nothing
        */
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        template<int8_t TxPinNumber, int8_t RxPinNumber>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            using TxPin = typename _TxPins::template Pin<TxPinNumber>;
            using RxPin = std::conditional_t<RxPinNumber != -1, typename _RxPins::template Pin<RxPinNumber>, typename IO::NullPin>;

            using usedPorts = IO::PortList<typename TemplateUtils::Unique<TypeList<typename TxPin::Port, typename RxPin::Port>>::type>;

            usedPorts::Enable();
            TxPin::SetConfiguration(TxPin::Port::AltFunc);
            TxPin::AltFuncNumber(GetAltFunctionNumber<_Regs>);

            if constexpr(!std::is_same_v<RxPin, IO::NullPin>)
            {
                RxPin::SetConfiguration(RxPin::Port::AltFunc);
                RxPin::AltFuncNumber(GetAltFunctionNumber<_Regs>);
            }
        }

        /**
         * @brief Template clone of SelectTxRxPins method (params are TPin instances)
         * 
         * @tparam TxPin TxPin
         * @tparam RxPin RxPin
         * 
         * @par Returns
         *	Nothing
        */
        template<typename _Regs, IRQn_Type _IRQNumber, typename _ClockCtrl, typename _TxPins, typename _RxPins, typename _DmaTx, typename _DmaRx>
        template<typename TxPin, typename RxPin>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            const int8_t txPinIndex = TypeIndex<TxPin, typename _TxPins::PinsAsTypeList>::value;
            const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                ? TypeIndex<RxPin, typename _RxPins::PinsAsTypeList>::value
                                : -1;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= -1);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
        }

        using Usart1TxPins = IO::PinList<IO::Pa9, IO::Pb6>;
        using Usart1RxPins = IO::PinList<IO::Pa10, IO::Pb7>;

        using Usart2TxPins = IO::PinList<IO::Pa2, IO::Pd5>;
        using Usart2RxPins = IO::PinList<IO::Pa3, IO::Pd6>;

        using Usart3TxPins = IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8>;
        using Usart3RxPins = IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9>;

        using Uart4TxPins = IO::PinList<IO::Pa0, IO::Pc10>;
        using Uart4RxPins = IO::PinList<IO::Pa1, IO::Pc11>;

        using Uart5TxPins = IO::PinList<IO::Pc12>;
        using Uart5RxPins = IO::PinList<IO::Pd2>;

        using Usart6TxPins = IO::PinList<IO::Pc6>;
        using Usart6RxPins = IO::PinList<IO::Pc7>;

        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
    #if defined(USART3)
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
    #endif
    #if defined(UART4)
        IO_STRUCT_WRAPPER(UART4, Uart4Regs, USART_TypeDef);
    #endif
    #if defined(UART5)
        IO_STRUCT_WRAPPER(UART5, Uart5Regs, USART_TypeDef);
    #endif
    #if defined(USART6)
        IO_STRUCT_WRAPPER(USART6, Usart6Regs, USART_TypeDef);
    #endif
    }
    using Usart1 = Private::Usart<Private::Usart1Regs, USART1_IRQn, Clock::Usart1Clock, Private::Usart1TxPins, Private::Usart1RxPins, Dma2Stream7Channel4, Dma2Stream2Channel4>;
    using Usart2 = Private::Usart<Private::Usart2Regs, USART2_IRQn, Clock::Usart2Clock, Private::Usart2TxPins, Private::Usart2RxPins, Dma1Stream6Channel4, Dma1Stream5Channel4>;
#if defined(USART3)
    using Usart3 = Private::Usart<Private::Usart3Regs, USART3_IRQn, Clock::Usart3Clock, Private::Usart3TxPins, Private::Usart3RxPins, Dma1Stream3Channel4, Dma1Stream1Channel4>;
#endif
#if defined(UART4)
    using Uart4 = Private::Usart<Private::Uart4Regs, UART4_IRQn, Clock::Uart4Clock, Private::Uart4TxPins, Private::Uart4RxPins, Dma1Stream4Channel4, Dma1Stream2Channel4>;
#endif
#if defined(UART5)
    using Uart5 = Private::Usart<Private::Uart5Regs, UART5_IRQn, Clock::Uart5Clock, Private::Uart5TxPins, Private::Uart5RxPins, Dma2Stream7Channel4, Dma2Stream0Channel4>;
#endif
#if defined(USART6)
    using Usart6 = Private::Usart<Private::Usart6Regs, USART6_IRQn, Clock::Usart6Clock, Private::Usart6TxPins, Private::Usart6RxPins, Dma2Stream7Channel5, Dma2Stream2Channel5>;
#endif
}

#endif //! ZHELE_USART_H