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
namespace Zhele
{

    namespace Private
    {
        class Usart1Regs;
        class Usart2Regs;
        class Usart3Regs;

        using UsartRegs = TypeList<Usart1Regs, Usart2Regs, Usart3Regs>;
        using UsartRemaps = TypeList<Zhele::IO::Usart1Remap, Zhele::IO::Usart2Remap, Zhele::IO::Usart3Remap>;

        template <typename _Regs>
        struct UsartRemapHelper
        {
            using type = typename GetType<TypeIndex<_Regs, UsartRegs>::value, UsartRemaps>::type;
        };

        template<typename Regs>
        using GetUsartRemap = typename UsartRemapHelper<Regs>::type;
        
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
            using Type = typename _TxPins::ValueType;
            Type maskTx(1 << txPinNumber);
            _TxPins::Enable();
            _TxPins::SetConfiguration(maskTx, _TxPins::AltFunc);

            Type maskRx(1 << rxPinNumber);

            _RxPins::Enable();
            _RxPins::SetConfiguration(maskRx, _RxPins::In);

            Clock::AfioClock::Enable();
            if (_TxPins::Length == 3 && txPinNumber == 2) // Usart3
            {
                GetUsartRemap<_Regs>::Set(3);
            }
            else
            {
                GetUsartRemap<_Regs>::Set(txPinNumber);
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
        template<uint8_t TxPinNumber, uint8_t RxPinNumber>
        void Usart<_Regs, _IRQNumber, _ClockCtrl, _TxPins, _RxPins, _DmaTx, _DmaRx>::SelectTxRxPins()
        {
            static_assert(TxPinNumber == RxPinNumber);
            using TxPin = typename GetType<TxPinNumber, typename _TxPins::PinsAsTypeList>::type;
            using RxPin = typename GetType<RxPinNumber, typename _RxPins::PinsAsTypeList>::type;

            TxPin::Port::Enable();
            
            if constexpr (!std::is_same_v<typename RxPin::Port, typename TxPin::Port>)
            {
                RxPin::Port::Enable();
            }

            TxPin::SetConfiguration(TxPin::Port::AltFunc);
            RxPin::SetConfiguration(RxPin::Port::In);

            if constexpr(TxPinNumber > 0)
            {
                Clock::AfioClock::Enable();
                if constexpr(_TxPins::Length == 3 && TxPinNumber == 2) // Usart3 full remap
                {
                    GetUsartRemap<_Regs>::Set(3);
                }
                else
                {
                    GetUsartRemap<_Regs>::Set(TxPinNumber);
                }
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
            const int txPinIndex = TypeIndex<TxPin, typename _TxPins::PinsAsTypeList>::value;
            const int rxPinIndex = TypeIndex<RxPin, typename _RxPins::PinsAsTypeList>::value;
            static_assert(txPinIndex >= 0);
            static_assert(rxPinIndex >= 0);
            SelectTxRxPins<txPinIndex, rxPinIndex>();
         }


        using Usart1TxPins = IO::PinList<IO::Pa9,  IO::Pb6>;
        using Usart1RxPins = IO::PinList<IO::Pa10, IO::Pb7>;

        using Usart2TxPins = IO::PinList<IO::Pa2, IO::Pd5>;
        using Usart2RxPins = IO::PinList<IO::Pa3, IO::Pd6>;

        using Usart3TxPins = IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8>;
        using Usart3RxPins = IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9>;

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