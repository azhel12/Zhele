/**
 * UART 4-char block reverser over DMA, 115200 8N1, no remap.
 * Receives 4 characters per DMA transfer and sends the block back reversed.
 *   V003/V006: USART1 TX=PD5 RX=PD6 | V203: USART1 TX=PA9 RX=PA10  (TX=DMA1 ch4, RX=DMA1 ch5)
 *   X035:      USART2 TX=PA2 RX=PA3                               (TX=DMA1 ch7, RX=DMA1 ch6)
 */
#include <zhele/usart.h>
#include <zhele/iopins.h>
#include <zhele/dma.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined(CH32X035)
using Uart = Usart2;
#  define DMA_TX_IRQHandler DMA1_Channel7_IRQHandler
#  define DMA_RX_IRQHandler DMA1_Channel6_IRQHandler
#else
using Uart = Usart1;
#  define DMA_TX_IRQHandler DMA1_Channel4_IRQHandler
#  define DMA_RX_IRQHandler DMA1_Channel5_IRQHandler
#endif

static uint8_t rxBlock[4];
static uint8_t txBlock[4];

void OnBlockReceived(void* data, unsigned size, bool success);

int main()
{
    Dma1::Enable();

    Uart::Init(115200);
#if defined(CH32V203)
    Uart::SelectTxRxPins<Pa9, Pa10>();
#elif defined(CH32X035)
    Uart::SelectTxRxPins<Pa2, Pa3>();
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
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA_TX_IRQHandler()
{
    Uart::DmaTx::IrqHandler();
}
extern "C" __attribute__((interrupt("WCH-Interrupt-fast"))) void DMA_RX_IRQHandler()
{
    Uart::DmaRx::IrqHandler();
}
#else
extern "C" __attribute__((naked)) void DMA_TX_IRQHandler()
{
    Uart::DmaTx::IrqHandler();
    __asm volatile ("mret");
}
extern "C" __attribute__((naked)) void DMA_RX_IRQHandler()
{
    Uart::DmaRx::IrqHandler();
    __asm volatile ("mret");
}
#endif
