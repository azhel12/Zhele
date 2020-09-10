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
        template<typename>
        class AltFuncHelper
        {
        public:
            static uint8_t Get(uint8_t index);
        };

        template<unsigned... AltFuncNumbers>
        class AltFuncHelper<UnsignedArray<AltFuncNumbers ...>>
        {
            const static uint8_t _altFuncNumbers[sizeof...(AltFuncNumbers)];
        public:
            static uint8_t Get(uint8_t index) {return _altFuncNumbers[index];};
        };

        template<unsigned... AltFuncNumbers>
        const uint8_t AltFuncHelper<UnsignedArray<AltFuncNumbers ...>>::_altFuncNumbers[sizeof... (AltFuncNumbers)] = {AltFuncNumbers ...};

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
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins(uint8_t txPinNumber, uint8_t rxPinNumber)
        {
            using TxAltFuncNumbers = typename _TxPins::Value;
            using RxAltFuncNumbers = typename _RxPins::Value;

    
            using TxPins = typename _TxPins::Key;
            using RxPins = typename _RxPins::Key;

            using Type = typename TxPins::DataType;

            TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            TxPins::SetConfiguration(maskTx, TxPins::AltFunc);
            TxPins::AltFuncNumber(maskTx, AltFuncHelper<TxAltFuncNumbers>::Get(txPinNumber));

            RxPins::Enable();
            Type maskRx(1 << rxPinNumber);
            RxPins::SetConfiguration(maskRx, RxPins::AltFunc);
            RxPins::AltFuncNumber(maskRx, AltFuncHelper<RxAltFuncNumbers>::Get(rxPinNumber));
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
        template<uint8_t TxPinNumber, uint8_t RxPinNumber>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            using TxAltFuncNumbers = typename _TxPins::Value;
            using RxAltFuncNumbers = typename _RxPins::Value;

            using TxPin = typename _TxPins::Key::template Pin<TxPinNumber>;
            TxPin::Port::Enable();
            TxPin::SetConfiguration(TxPin::Port::AltFunc);
            TxPin::AltFuncNumber(GetNumber<TxPinNumber, TxAltFuncNumbers>::value);

            using RxPin = typename _RxPins::Key::template Pin<RxPinNumber>;
            if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>)
            {
                RxPin::Port::Enable();
            }
            RxPin::SetConfiguration(RxPin::Port::AltFunc);
            RxPin::AltFuncNumber(GetNumber<RxPinNumber, RxAltFuncNumbers>::value);
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
            const int txPinIndex = TypeIndex<TxPin, typename _TxPins::Key::PinsAsTypeList>::value;
            const int rxPinIndex = TypeIndex<RxPin, typename _RxPins::Key::PinsAsTypeList>::value;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= 0);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
        }

        using Usart1TxPins = Pair<IO::PinList<IO::Pa9, IO::Pb6>, UnsignedArray<1, 0>>;
        using Usart1RxPins = Pair<IO::PinList<IO::Pa10, IO::Pb7>, UnsignedArray<1, 0>>;

        using Usart2TxPins = Pair<IO::PinList<IO::Pa2, IO::Pd5>, UnsignedArray<1, 1>>;
        using Usart2RxPins = Pair<IO::PinList<IO::Pa3, IO::Pd6>, UnsignedArray<1, 1>>;

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