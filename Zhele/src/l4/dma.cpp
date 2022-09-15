#if defined (STM32L4)
    #include <common/macro_utils/declarations.h>
    #include <dma.h>

    using namespace Zhele;

    extern "C" ZHELE_INTERRUPT(DMA1_Channel1_IRQHandler)
    {
        Dma1Stream1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel2_IRQHandler)
    {
        Dma1Stream2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel3_IRQHandler)
    {
        Dma1Stream3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel4_IRQHandler)
    {
        Dma1Stream4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel5_IRQHandler)
    {
        Dma1Stream5::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel6_IRQHandler)
    {
        Dma1Stream6::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Channel7_IRQHandler)
    {
        Dma1Stream7::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel1_IRQHandler)
    {
        Dma2Stream1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel2_IRQHandler)
    {
        Dma2Stream2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel3_IRQHandler)
    {
        Dma2Stream3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel4_IRQHandler)
    {
        Dma2Stream4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel5_IRQHandler)
    {
        Dma2Stream5::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel6_IRQHandler)
    {
        Dma2Stream6::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Channel7_IRQHandler)
    {
        Dma2Stream7::IrqHandler();
    }

    void DmaDummy() {}
#endif
