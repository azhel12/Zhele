/**
 * @file
 * Implement DMA protocol for stm32f0 series
 * 
 * @author Alexey Zhelonkin
 * @date 2019
 * @license FreeBSD
 */

#ifndef ZHELE_DMA_H
#define ZHELE_DMA_H

#include <stm32f0xx.h>
#include "../common/dma.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA1, Dma1, DMA_TypeDef);
        
        IO_STRUCT_WRAPPER(DMA1_Channel1, Dma1Channel1, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel2, Dma1Channel2, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel3, Dma1Channel3, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel4, Dma1Channel4, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel5, Dma1Channel5, DMA_Channel_TypeDef);
    #if defined (DMA1_Channel6)
        IO_STRUCT_WRAPPER(DMA1_Channel6, Dma1Channel6, DMA_Channel_TypeDef);
    #endif
    #if defined (DMA1_Channel7)
        IO_STRUCT_WRAPPER(DMA1_Channel7, Dma1Channel7, DMA_Channel_TypeDef);	
    #endif
    }


    using Dma1 = DmaModule<Private::Dma1, Clock::Dma1Clock, 7>;
    using Dma1Channel1 = DmaChannel<Dma1, Private::Dma1Channel1, 1, DMA1_Channel1_IRQn>;
    using Dma1Channel2 = DmaChannel<Dma1, Private::Dma1Channel2, 2, DMA1_Ch2_3_DMA2_Ch1_2_IRQn>;
    using Dma1Channel3 = DmaChannel<Dma1, Private::Dma1Channel3, 3, DMA1_Ch2_3_DMA2_Ch1_2_IRQn>;
    using Dma1Channel4 = DmaChannel<Dma1, Private::Dma1Channel4, 4, DMA1_Channel4_5_6_7_IRQn>;
    using Dma1Channel5 = DmaChannel<Dma1, Private::Dma1Channel5, 5, DMA1_Channel4_5_6_7_IRQn>;
#if defined (DMA1_Channel6)
    using Dma1Channel6 = DmaChannel<Dma1, Private::Dma1Channel6, 6, DMA1_Channel4_5_6_7_IRQn>;
#endif
#if defined (DMA1_Channel7)
    using Dma1Channel7 = DmaChannel<Dma1, Private::Dma1Channel7, 7, DMA1_Channel4_5_6_7_IRQn>;
#endif

#if defined (RCC_AHBENR_DMA2EN)
    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA2, Dma2, DMA_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel1, Dma2Channel1, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel2, Dma2Channel2, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel3, Dma2Channel3, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel4, Dma2Channel4, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel5, Dma2Channel5, DMA_Channel_TypeDef);	
    }
    using Dma2 = DmaModule<Private::Dma2, Clock::Dma2Clock, 5>;
    using Dma2Channel1 = DmaChannel<Dma2, Private::Dma2Channel1, 1, DMA1_Ch2_3_DMA2_Ch1_2_IRQn>;
    using Dma2Channel2 = DmaChannel<Dma2, Private::Dma2Channel2, 2, DMA1_Ch2_3_DMA2_Ch1_2_IRQn>;
    using Dma2Channel3 = DmaChannel<Dma2, Private::Dma2Channel3, 3, DMA1_Ch4_7_DMA2_Ch3_5_IRQn>;
    using Dma2Channel4 = DmaChannel<Dma2, Private::Dma2Channel4, 4, DMA1_Ch4_7_DMA2_Ch3_5_IRQn>;
    using Dma2Channel5 = DmaChannel<Dma2, Private::Dma2Channel5, 5, DMA1_Ch4_7_DMA2_Ch3_5_IRQn>;
#endif
} // namespace Zhele

#endif //! ZHELE_DMA_H