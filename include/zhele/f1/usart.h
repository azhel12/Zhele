/**
 * @file
 * @brief Implements USART protocol for stm32f1 series
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_USART_H
#define ZHELE_USART_H

#include "../common/template_utils/static_array.h"
#include "../common/usart.h"

#include "clock.h"
#include "dma.h"
#include "iopins.h"
#include "remap.h"

#include <type_traits>
#include <utility>

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
            using AltFuncNumbers = typename _TxPins::second_type;

            using TxPins = typename _TxPins::first_type;
            using RxPins = typename _RxPins::first_type;

            using Type = typename _TxPins::first_type::DataType;

            TxPins::Enable();
            Type maskTx(1 << txPinNumber);
            TxPins::SetConfiguration(TxPins::AltFunc, maskTx);

            if(rxPinNumber != -1)
            {
                RxPins::Enable();
                Type maskRx(1 << rxPinNumber);
                RxPins::SetConfiguration(RxPins::In, maskRx);
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

            using TxAltFuncNumbers = typename _TxPins::second_type;
            using RxAltFuncNumbers = typename _RxPins::second_type;

            using TxPin = typename _TxPins::first_type:: template Pin<TxPinNumber>;
            TxPin::Port::Enable();
            TxPin::SetConfiguration(TxPin::Port::AltFunc);
            
            if constexpr(RxPinNumber != -1)
            {
                using RxPin = typename _RxPins::first_type::template Pin<RxPinNumber>;

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
            const int8_t txPinIndex = _TxPins::first_type:: template IndexOf<TxPin>;
            const int8_t rxPinIndex = !std::is_same_v<RxPin, IO::NullPin>
                                ? _RxPins::first_type:: template IndexOf<RxPin>
                                : -1;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= -1);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
         }


        using Usart1TxPins = std::pair<IO::PinList<IO::Pa9, IO::Pb6>, NonTypeTemplateArray<0, 1>>;
        using Usart1RxPins = std::pair<IO::PinList<IO::Pa10, IO::Pb7>, NonTypeTemplateArray<0, 1>>;

        using Usart2TxPins = std::pair<IO::PinList<IO::Pa2, IO::Pd5>, NonTypeTemplateArray<0, 1>>;
        using Usart2RxPins = std::pair<IO::PinList<IO::Pa3, IO::Pd6>, NonTypeTemplateArray<0, 1>>;

        using Usart3TxPins = std::pair<IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8>, NonTypeTemplateArray<0, 1, 3>>;
        using Usart3RxPins = std::pair<IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9>, NonTypeTemplateArray<0, 1, 3>>;

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