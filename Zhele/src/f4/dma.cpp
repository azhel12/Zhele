#if defined (STM32F4)
    #include <common/macro_utils/declarations.h>
    #include <dma.h>

    using namespace Zhele;
    
    extern "C" ZHELE_INTERRUPT(DMA1_Stream0_IRQHandler)
    {
        Dma1Stream0::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream1_IRQHandler)
    {
        Dma1Stream1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream2_IRQHandler)
    {
        Dma1Stream2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream3_IRQHandler)
    {
        Dma1Stream3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream4_IRQHandler)
    {
        Dma1Stream4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream5_IRQHandler)
    {
        Dma1Stream5::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream6_IRQHandler)
    {
        Dma1Stream6::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Stream7_IRQHandler)
    {
        Dma1Stream7::IrqHandler();
    }

    extern "C" ZHELE_INTERRUPT(DMA2_Stream0_IRQHandler)
    {
        Dma2Stream0::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream1_IRQHandler)
    {
        Dma2Stream1::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream2_IRQHandler)
    {
        Dma2Stream2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream3_IRQHandler)
    {
        Dma2Stream3::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream4_IRQHandler)
    {
        Dma2Stream4::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream5_IRQHandler)
    {
        Dma2Stream5::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream6_IRQHandler)
    {
        Dma2Stream6::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA2_Stream7_IRQHandler)
    {
        Dma2Stream7::IrqHandler();
    }

    void DmaDummy() {}
#endif
