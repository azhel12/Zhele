/**
 * @file
 * Implements DMA methods.
 * 
 * @author Konstantin Chizhov
 * @date ??
 * @license FreeBSD
 */

#if defined (STM32F0)
    #include <common/macro_utils/declarations.h>
    #include <dma.h>
    
    using namespace Zhele;

    extern "C" ZHELE_INTERRUPT(DMA1_Ch1_IRQHandler)
    {
        Dma1Channel1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler)
    {
        // Comment unnecessary lines (if you use not all channels of this interrupt,
        // you can comment unnecessary lines and spare some MCU ticks)
        
        Dma1Channel2::IrqHandler();
        Dma1Channel3::IrqHandler();
    #if defined(DMA2)
        Dma2Channel1::IrqHandler();
        Dma2Channel2::IrqHandler();
    #endif
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler)
    {
        // Comment unnecessary lines (if you use not all channels of this interrupt,
        // you can comment unnecessary lines and spare some MCU ticks)

        Dma1Channel4::IrqHandler();
        Dma1Channel5::IrqHandler();
     #if defined (DMA1_Channel6)
        Dma1Channel6::IrqHandler();
    #endif
    #if defined (DMA1_Channel7)
        Dma1Channel7::IrqHandler();
    #endif
    #if defined(DMA2)
        Dma2Channel3::IrqHandler();
        Dma2Channel4::IrqHandler();
        Dma2Channel5::IrqHandler();
    #endif
    }

    void DmaDummy() {}
#endif
