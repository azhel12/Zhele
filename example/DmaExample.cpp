#include <dma.h>

using namespace Zhele;

/// Dma stream looks like dma channels, but it is not same.
/// So Zhele contains channels and streams.
/// There is no difference between channel and stream for some tasks.
/// In this case you can use "typedef" or "using" as in this example.
#if defined (STM32F4)
    using MyDma = Dma2Stream1; // See RM for f4 series. "Only the DMA2 controller is able to perform memory-to-memory transfers."
#else
    using MyDma = Dma1Channel1;
#endif

void DmaTransferCallback(void* data, unsigned size, bool success);

char TxBuffer[] = "Hello,world"; // 12 bytes (with '\0')
char RxBuffer[16];

int main()
{
    // In Mem2Mem mode Source = periph, Destination = Mem
    MyDma::SetTransferCallback(DmaTransferCallback);

    MyDma::Transfer(
        MyDma::Mem2Mem // Memory To Memory
        | MyDma::MSize8Bits
        | MyDma::PSize8Bits
        | MyDma::MemIncrement
        | MyDma::PeriphIncrement,
        RxBuffer, TxBuffer, 12);

    // We have 4-bytes-aligned buffer and could transfer 3x4 bytes:
    /*
    Dma1Channel1::Transfer(
        Dma1Channel1::Mem2Mem
        | Dma1Channel1::MSize32Bits // Set memory (destination) size as 32 bits
        | Dma1Channel1::PSize326Bits // Set memory (destination) size as 32 bits
        | Dma1Channel1::MemIncrement
        | Dma1Channel1::PeriphIncrement,
        RxBuffer, TxBuffer, 3);
    */
   
    for (;;)
    {
    }
}


void DmaTransferCallback(void* data, unsigned size, bool success)
{
    // Set breakpoint on this dummy code and check RxBuffer in debugger. It contains "Hello,world"
    volatile int dummy = 42;
}