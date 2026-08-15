/**
 * @file
 * Implement DMA protocol for stm32g4 series
 * 
 * @author Aleksei Zhelonkin & Damir Bakiev
 * @date 2026
 * @license MIT
 */

#ifndef ZHELE_PLATFORM_STM32_G4_DMA_H
#define ZHELE_PLATFORM_STM32_G4_DMA_H

#include <stm32g4xx.h>
#include "../common/dma.h"

namespace Zhele
{
    namespace Private
    {
        /**
         * @brief DMA clock control which also enables DMAMUX clock (required for any DMA request on G4)
         */
        template<typename _DmaClock>
        class DmaWithMuxClock : public _DmaClock
        {
        public:
            static void Enable()
            {
                _DmaClock::Enable();
                Clock::DmaMux1Clock::Enable();
            }
        };

        IO_STRUCT_WRAPPER(DMA1, Dma1, DMA_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel1, Dma1Channel1, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel2, Dma1Channel2, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel3, Dma1Channel3, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel4, Dma1Channel4, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel5, Dma1Channel5, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA1_Channel6, Dma1Channel6, DMA_Channel_TypeDef);
    #if defined (DMA1_Channel7)
        IO_STRUCT_WRAPPER(DMA1_Channel7, Dma1Channel7, DMA_Channel_TypeDef);
    #endif
    #if defined (DMA1_Channel8)
        IO_STRUCT_WRAPPER(DMA1_Channel8, Dma1Channel8, DMA_Channel_TypeDef);
    #endif

        IO_STRUCT_WRAPPER(DMA2, Dma2, DMA_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel1, Dma2Channel1, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel2, Dma2Channel2, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel3, Dma2Channel3, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel4, Dma2Channel4, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel5, Dma2Channel5, DMA_Channel_TypeDef);
        IO_STRUCT_WRAPPER(DMA2_Channel6, Dma2Channel6, DMA_Channel_TypeDef);
    #if defined (DMA2_Channel7)
        IO_STRUCT_WRAPPER(DMA2_Channel7, Dma2Channel7, DMA_Channel_TypeDef);
    #endif
    #if defined (DMA2_Channel8)
        IO_STRUCT_WRAPPER(DMA2_Channel8, Dma2Channel8, DMA_Channel_TypeDef);
    #endif
    }

#if defined (DMA1_Channel8)
    using Dma1 = DmaModule<Private::Dma1, Private::DmaWithMuxClock<Clock::Dma1Clock>, 8>;
    using Dma2 = DmaModule<Private::Dma2, Private::DmaWithMuxClock<Clock::Dma2Clock>, 8>;
#else
    using Dma1 = DmaModule<Private::Dma1, Private::DmaWithMuxClock<Clock::Dma1Clock>, 6>;
    using Dma2 = DmaModule<Private::Dma2, Private::DmaWithMuxClock<Clock::Dma2Clock>, 6>;
#endif

    using Dma1Channel1 = DmaChannel<Dma1, Private::Dma1Channel1, 1, DMA1_Channel1_IRQn>;
    using Dma1Channel2 = DmaChannel<Dma1, Private::Dma1Channel2, 2, DMA1_Channel2_IRQn>;
    using Dma1Channel3 = DmaChannel<Dma1, Private::Dma1Channel3, 3, DMA1_Channel3_IRQn>;
    using Dma1Channel4 = DmaChannel<Dma1, Private::Dma1Channel4, 4, DMA1_Channel4_IRQn>;
    using Dma1Channel5 = DmaChannel<Dma1, Private::Dma1Channel5, 5, DMA1_Channel5_IRQn>;
    using Dma1Channel6 = DmaChannel<Dma1, Private::Dma1Channel6, 6, DMA1_Channel6_IRQn>;
#if defined (DMA1_Channel7)
    using Dma1Channel7 = DmaChannel<Dma1, Private::Dma1Channel7, 7, DMA1_Channel7_IRQn>;
#endif
#if defined (DMA1_Channel8)
    using Dma1Channel8 = DmaChannel<Dma1, Private::Dma1Channel8, 8, DMA1_Channel8_IRQn>;
#endif

    using Dma2Channel1 = DmaChannel<Dma2, Private::Dma2Channel1, 1, DMA2_Channel1_IRQn>;
    using Dma2Channel2 = DmaChannel<Dma2, Private::Dma2Channel2, 2, DMA2_Channel2_IRQn>;
    using Dma2Channel3 = DmaChannel<Dma2, Private::Dma2Channel3, 3, DMA2_Channel3_IRQn>;
    using Dma2Channel4 = DmaChannel<Dma2, Private::Dma2Channel4, 4, DMA2_Channel4_IRQn>;
    using Dma2Channel5 = DmaChannel<Dma2, Private::Dma2Channel5, 5, DMA2_Channel5_IRQn>;
    using Dma2Channel6 = DmaChannel<Dma2, Private::Dma2Channel6, 6, DMA2_Channel6_IRQn>;
#if defined (DMA2_Channel7)
    using Dma2Channel7 = DmaChannel<Dma2, Private::Dma2Channel7, 7, DMA2_Channel7_IRQn>;
#endif
#if defined (DMA2_Channel8)
    using Dma2Channel8 = DmaChannel<Dma2, Private::Dma2Channel8, 8, DMA2_Channel8_IRQn>;
#endif
} // namespace Zhele

#endif //! ZHELE_PLATFORM_STM32_G4_DMA_H
