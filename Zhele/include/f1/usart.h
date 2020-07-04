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

namespace Zhele
{
    namespace Private
    {
        typedef IO::PinList<IO::Pa9,  IO::Pb6> Usart1TxPins;
		typedef IO::PinList<IO::Pa10, IO::Pb7> Usart1RxPins;

		typedef IO::PinList<IO::Pa2, IO::Pd5> Usart2TxPins;
		typedef IO::PinList<IO::Pa3, IO::Pd6> Usart2RxPins;

		typedef IO::PinList<IO::Pb10, IO::Pc10, IO::Pd8> Usart3TxPins;
		typedef IO::PinList<IO::Pb11, IO::Pc11, IO::Pd9> Usart3RxPins;

		IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart1Remap, uint32_t, AFIO_MAPR_USART1_REMAP_Pos, 1);
		IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart2Remap, uint32_t, AFIO_MAPR_USART2_REMAP_Pos, 1);
		IO_BITFIELD_WRAPPER(AFIO->MAPR, Usart3Remap, uint32_t, AFIO_MAPR_USART3_REMAP_Pos, 2);
    }

    DECLARE_USART(USART1, USART1_IRQn, Clock::Usart1Clock, Usart1, Dma1Channel4, Dma1Channel5);
    DECLARE_USART(USART2, USART2_IRQn, Clock::Usart2Clock, Usart2, Dma1Channel7, Dma1Channel6);
    DECLARE_USART(USART3, USART3_IRQn, Clock::Usart3Clock, Usart3, Dma1Channel2, Dma1Channel3);
}

#endif //! ZHELE_USART_H