/**
 * UART 4-char block reverser over DMA, USART1, 115200 8N1 (TX=DMA1 ch4, RX=DMA1 ch5).
 * Receives 4 characters per DMA transfer and sends the block back reversed.
 *   V003/V006: TX=PD5 RX=PD6 | V203: TX=PA9 RX=PA10  (no remap)
 */
#include <zhele/usart.h>
#include <zhele/iopins.h>
#include <zhele/dma.h>

using namespace Zhele;
using namespace Zhele::IO;

using Uart = Usart1;

static uint8_t rxBlock[4];
static uint8_t txBlock[4];

void OnBlockReceived(void* data, unsigned size, bool success);

int main()
{
    Dma1::Enable();

    Uart::Init(115200);
#if defined(CH32V203)
    Uart::SelectTxRxPins<Pa9, Pa10>();
#else
    Uart::SelectTxRxPins<Pd5, Pd6>();
#endif

    const char hello[] = "Zhele UART reverser (DMA, 4-char blocks)\r\n";
    Uart::Write(hello, sizeof(hello) - 1);

    Uart::EnableAsyncRead(rxBlock, sizeof(rxBlock), OnBlockReceived);

    for (;;)
    {
    }
}

// Fires once the 4-char block has landed (RX DMA); reverse it and DMA it back.
void OnBlockReceived(void* data, unsigned size, bool success)
{
    if (success)
    {
        const uint8_t* in = static_cast<const uint8_t*>(data);
        for (unsigned i = 0; i < size; ++i)
            txBlock[i] = in[size - 1 - i];
        Uart::WriteAsync(txBlock, size);
    }

    Uart::EnableAsyncRead(rxBlock, sizeof(rxBlock), OnBlockReceived); // re-arm
}

#if defined(ZHELE_WCH_TOOLCHAIN)
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA1_Channel4_IRQHandler()
{
    Uart::DmaTx::IrqHandler();
}
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA1_Channel5_IRQHandler()
{
    Uart::DmaRx::IrqHandler();
}
#else
extern "C" __attribute__((naked)) void DMA1_Channel4_IRQHandler()
{
    Uart::DmaTx::IrqHandler();
    __asm volatile ("mret");
}
extern "C" __attribute__((naked)) void DMA1_Channel5_IRQHandler()
{
    Uart::DmaRx::IrqHandler();
    __asm volatile ("mret");
}
#endif
