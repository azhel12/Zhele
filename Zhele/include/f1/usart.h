/**
 * @file
 * @brief Implements USART protocol for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_USART_H
#define ZHELE_USART_H

#include <stm32f1xx.h>

#include "../common/usart.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "remap.h"
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
            using AltFuncNumbers = typename _TxPins::Value;

            using TxPins = typename _TxPins::Key;
            using RxPins = typename _RxPins::Key;

            using Type = typename _TxPins::Key::DataType;

            TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            TxPins::SetConfiguration(maskTx, TxPins::AltFunc);

            if(rxPinNumber != -1)
            {
                RxPins::Enable();
                Type maskRx(1 << rxPinNumber);
                RxPins::SetConfiguration(maskRx, RxPins::In);
            }

            Clock::AfioClock::Enable();
            Zhele::IO::Private::PeriphRemap<_ClockCtrl>::Set(GetNumberRuntime<AltFuncNumbers>::Get(txPinNumber));
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
            static_assert(TxPinNumber == RxPinNumber || RxPinNumber == -1);

            using TxAltFuncNumbers = typename _TxPins::Value;
            using RxAltFuncNumbers = typename _RxPins::Value;

            using TxPin = typename _TxPins::Key:: template Pin<TxPinNumber>;
            TxPin::Port::Enable();
            TxPin::SetConfiguration(TxPin::Port::AltFunc);

            using RxPin = std::conditional_t<RxPinNumber != -1, typename _RxPins::Key::template Pin<RxPinNumber>, typename IO::NullPin>;
            
            if constexpr(!std::is_same_v<RxPin, IO::NullPin>)
            {
                if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>)
                {
                    RxPin::Port::Enable();
                }
                RxPin::SetConfiguration(RxPin::Port::In);
            }

            Clock::AfioClock::Enable();
            Zhele::IO::Private::PeriphRemap<_ClockCtrl>::Set(GetNonTypeValueByIndex<TxPinNumber, TxAltFuncNumbers>::value);
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
            const int8_t txPinIndex = TypeIndex<TxPin, typename _TxPins::Key::PinsAsTypeList>::value;
            const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                ? TypeIndex<RxPin, typename _RxPins::Key::PinsAsTypeList>::value
                                : -1;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= -1);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
         }


        using Usart1TxPins = Pair<IO::PinList<IO::Pa9, IO::Pb6>, NonTypeTemplateArray<0, 1>>;
        using Usart1RxPins = Pair<IO::PinList<IO::Pa10, IO::Pb7>, NonTypeTemplateArray<0, 1>>;

        using Usart2TxPins = Pair<IO::PinList<IO::Pa2, IO::Pd5>, NonTypeTemplateArray<0, 1>>;
        using Usart2RxPins = Pair<IO::PinList<IO::Pa3, IO::Pd6>, NonTypeTemplateArray<0, 1>>;

        using Usart3TxPins = Pair<IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8>, NonTypeTemplateArray<0, 1, 3>>;
        using Usart3RxPins = Pair<IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9>, NonTypeTemplateArray<0, 1, 3>>;

        IO_STRUCT_WRAPPER(USART1, Usart1Regs, USART_TypeDef);
        IO_STRUCT_WRAPPER(USART2, Usart2Regs, USART_TypeDef);
    #if defined(USART3)
        IO_STRUCT_WRAPPER(USART3, Usart3Regs, USART_TypeDef);
    #endif
    }
    using Usart1 = Private::Usart<Private::Usart1Regs, USART1_IRQn, Clock::Usart1Clock, Private::Usart1TxPins, Private::Usart1RxPins, Dma1Channel4, Dma1Channel5>;
    using Usart2 = Private::Usart<Private::Usart2Regs, USART2_IRQn, Clock::Usart2Clock, Private::Usart2TxPins, Private::Usart2RxPins, Dma1Channel7, Dma1Channel6>;
    #if defined(USART3)
        using Usart3 = Private::Usart<Private::Usart3Regs, USART3_IRQn, Clock::Usart3Clock, Private::Usart3TxPins, Private::Usart3RxPins, Dma1Channel2, Dma1Channel3>;
    #endif
}

#endif //! ZHELE_USART_H