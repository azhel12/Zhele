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
        // Uncomment necessary lines
        
        //Dma1Channel2::IrqHandler();
        //Dma1Channel3::IrqHandler();
        //Dma2Channel1::IrqHandler();
        //Dma2Channel2::IrqHandler();
    }
    extern "C" ZHELE_INTERRUPT(DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler)
    {
        // Uncomment necessary lines

        //Dma1Channel4::IrqHandler();
        //Dma1Channel5::IrqHandler();
        //Dma1Channel6::IrqHandler();
        //Dma1Channel7::IrqHandler();
        //Dma2Channel3::IrqHandler();
        //Dma2Channel4::IrqHandler();
        //Dma2Channel5::IrqHandler();
    }

    void DmaDummy() {}
#endif
