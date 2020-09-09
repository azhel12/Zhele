#if defined (STM32F1)
    #include <common/macro_utils/declarations.h>
    #include <dma.h>

    using namespace Zhele;
    
    extern "C" ZHELE_INTERRUPT(DMA1_Channel1_IRQHandler)
    {
        Dma1Channel1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel2_IRQHandler)
    {
        Dma1Channel2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel3_IRQHandler)
    {
        Dma1Channel3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel4_IRQHandler)
    {
        Dma1Channel4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel5_IRQHandler)
    {
        Dma1Channel5::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel6_IRQHandler)
    {
        Dma1Channel6::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel7_IRQHandler)
    {
        Dma1Channel7::IrqHandler();
    }
    #if defined (RCC_AHBENR_DMA2EN)
    extern "C" ZHELE_INTERRUPT(DMA2_Channel1_IRQHandler)
    {
        Dma2Channel1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel2_IRQHandler)
    {
        Dma2Channel2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel3_IRQHandler)
    {
        Dma2Channel3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel4_IRQHandler)
    {
        Dma2Channel4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel5_IRQHandler)
    {
        Dma2Channel5::IrqHandler();
    }
    #endif

    void DmaDummy() {}
#endif
