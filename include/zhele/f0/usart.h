/**
 * @file
 * @brief Implements USART protocol for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_USART_H
#define ZHELE_USART_H

#include <stm32f0xx.h>

#include "../common/usart.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"

#include "../common/template_utils/pair.h"
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
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins(int8_t txPinNumber, int8_t rxPinNumber)
        {
            using TxAltFuncNumbers = typename _TxPins::Value;
            using RxAltFuncNumbers = typename _RxPins::Value;

            using TxPins = typename _TxPins::Key;
            using RxPins = typename _RxPins::Key;

            using Type = typename TxPins::DataType;

            TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            TxPins::SetConfiguration(TxPins::AltFunc, maskTx);
            TxPins::AltFuncNumber(GetNumberRuntime<TxAltFuncNumbers>::Get(txPinNumber), maskTx);

            if(rxPinNumber != -1)
            {
                RxPins::Enable();
                Type maskRx(1 << rxPinNumber);
                RxPins::SetConfiguration(RxPins::AltFunc, maskRx);
                RxPins::AltFuncNumber(GetNumberRuntime<RxAltFuncNumbers>::Get(rxPinNumber), maskRx);
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
            using TxAltFuncNumbers = typename _TxPins::Value;
            using RxAltFuncNumbers = typename _RxPins::Value;

            using TxPin = typename _TxPins::Key::template Pin<TxPinNumber>;

            TxPin::Port::Enable();

            TxPin::template SetConfiguration<TxPin::Port::AltFunc>();
            TxPin::template AltFuncNumber<GetNonTypeValueByIndex<TxPinNumber, TxAltFuncNumbers>::value>();

            if constexpr(RxPinNumber != -1)
            {
                using RxPin = typename _RxPins::Key::template Pin<RxPinNumber>;

                if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>) {
                    RxPin::Port::Enable();
                }

                RxPin::template SetConfiguration<RxPin::Port::AltFunc>();
                RxPin::template AltFuncNumber<GetNonTypeValueByIndex<RxPinNumber, RxAltFuncNumbers>::value>();
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
            const int8_t txPinIndex = _TxPins::Key:: template IndexOf<TxPin>;
            const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                ? _RxPins::Key:: template IndexOf<RxPin>
                                : -1;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= -1);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
        }

        using Usart1TxPins = Pair<IO::PinList<IO::Pa2, IO::Pa9, IO::Pb6>, NonTypeTemplateArray<1, 1, 0>>;
        using Usart1RxPins = Pair<IO::PinList<IO::Pa10, IO::Pa15, IO::Pb7>, NonTypeTemplateArray<1, 1, 0>>;

        using Usart2TxPins = Pair<IO::PinList<IO::Pa2, IO::Pd5>, NonTypeTemplateArray<1, 1>>;
        using Usart2RxPins = Pair<IO::PinList<IO::Pa3, IO::Pd6>, NonTypeTemplateArray<1, 1>>;

        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
    #if defined(USART2)
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
    #endif
    }
    using Usart1 = Private::Usart<Private::Usart1Regs, USART1_IRQn, Clock::Usart1Clock, Private::Usart1TxPins, Private::Usart1RxPins, Dma1Channel2, Dma1Channel3>;
#if defined (USART2)
    using Usart2 = Private::Usart<Private::Usart2Regs, USART2_IRQn, Clock::Usart2Clock, Private::Usart2TxPins, Private::Usart2RxPins, Dma1Channel4, Dma1Channel5>;
#endif
}

#endif //! ZHELE_USART_H
