/**
 * @file
 * CH32V00x: instantiate the DMA1 controller and its channels.
 */
#ifndef ZHELE_PLATFORM_CH32_V0_DMA_H
#define ZHELE_PLATFORM_CH32_V0_DMA_H

#include <ch32v00x.h>

#include "clock.h"

#include "../common/dma.h"
#include "../common/ioreg.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA1, Dma1Regs, DMA_TypeDef);

        IO_STRUCT_WRAPPER(DMA1_Channel1, Dma1Channel1Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel2, Dma1Channel2Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel3, Dma1Channel3Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel4, Dma1Channel4Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel5, Dma1Channel5Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel6, Dma1Channel6Regs, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel7, Dma1Channel7Regs, DMA_Channel_TypeDef);
    }

    using Dma1 = DmaModule<Private::Dma1Regs, Clock::Dma1Clock, 7>;

    using Dma1Channel1 = DmaChannel<Dma1, Private::Dma1Channel1Regs, 1, DMA1_Channel1_IRQn>;
    using Dma1Channel2 = DmaChannel<Dma1, Private::Dma1Channel2Regs, 2, DMA1_Channel2_IRQn>;
    using Dma1Channel3 = DmaChannel<Dma1, Private::Dma1Channel3Regs, 3, DMA1_Channel3_IRQn>;
    using Dma1Channel4 = DmaChannel<Dma1, Private::Dma1Channel4Regs, 4, DMA1_Channel4_IRQn>;
    using Dma1Channel5 = DmaChannel<Dma1, Private::Dma1Channel5Regs, 5, DMA1_Channel5_IRQn>;
    using Dma1Channel6 = DmaChannel<Dma1, Private::Dma1Channel6Regs, 6, DMA1_Channel6_IRQn>;
    using Dma1Channel7 = DmaChannel<Dma1, Private::Dma1Channel7Regs, 7, DMA1_Channel7_IRQn>;
}

#endif // ZHELE_PLATFORM_CH32_V0_DMA_H
