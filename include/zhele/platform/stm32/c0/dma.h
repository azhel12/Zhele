/**
 * @file
 * Implement DMA protocol for stm32c0 series
 *
 * STM32C0 is a cut-down STM32G0: same DMA IP with DMAMUX, but only one
 * DMA controller with 3 channels and two grouped IRQ lines.
 *
 * @author Alexey Zhelonkin
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_C0_DMA_H
#define ZHELE_PLATFORM_STM32_C0_DMA_H

#include <stm32c0xx.h>
#include "../common/dma.h"

namespace Zhele
{
    namespace Private
    {
        IO_STRUCT_WRAPPER(DMA1, Dma1, DMA_TypeDef);

        IO_STRUCT_WRAPPER(DMA1_Channel1, Dma1Channel1, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel2, Dma1Channel2, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel3, Dma1Channel3, DMA_Channel_TypeDef);
    }

    using Dma1 = DmaModule<Private::Dma1, Clock::Dma1Clock, 3>;
    using Dma1Channel1 = DmaChannel<Dma1, Private::Dma1Channel1, 1, DMA1_Channel1_IRQn>;
    using Dma1Channel2 = DmaChannel<Dma1, Private::Dma1Channel2, 2, DMA1_Channel2_3_IRQn>;
    using Dma1Channel3 = DmaChannel<Dma1, Private::Dma1Channel3, 3, DMA1_Channel2_3_IRQn>;
} // namespace Zhele

#endif //! ZHELE_PLATFORM_STM32_C0_DMA_H
