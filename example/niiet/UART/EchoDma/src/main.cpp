/**
 * @file
 * UART0 4-char block reverser over DMA, 115200 8N1.
 *
 * DMA receives 4 characters, the completion callback reverses the block and
 * DMA sends it back, then the next read is re-armed.
 *   VG015: TX=Pa1 RX=Pa0, PL230 channels hard-wired to UART0
 *   VG5T:  TX=Pb1 RX=Pb0 | VG7T: TX=Pa1 RX=Pa0, AXI DMA channels 0/1 on the UART0 request
 */
#include <zhele/dma.h>
#include <zhele/usart.h>

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
    #include <system_k1921vg015.h> // SystemInit()
#elif defined(ZHELE_NIIET_DEVICE_K1921VG5T)
    #include <system_k1921vg5t.h>
#else
    #include <system_k1921vg7t.h>
#endif

extern "C" {
    #include <plic.h>
}

using namespace Zhele;
using namespace Zhele::IO;

#if defined(ZHELE_NIIET_DEVICE_K1921VG015)
    using DmaTx = DmaUart0Tx;
    using DmaRx = DmaUart0Rx;
#else
    using DmaTx = Dma1Channel<0, DmaRequest::Uart0>;
    using DmaRx = Dma1Channel<1, DmaRequest::Uart0>;
#endif
using Uart = Usart0<DmaTx, DmaRx>;

static uint8_t rxBlock[4];
static uint8_t txBlock[4];

static void OnBlockReceived(void* data, unsigned size, bool success);

static void DmaTxIsr() { DmaTx::IrqHandler(); }
static void DmaRxIsr() { DmaRx::IrqHandler(); }

int main()
{
    SystemInit();

    Uart::Init<115200>();
#if defined(ZHELE_NIIET_DEVICE_K1921VG5T)
    Uart::SelectTxRxPins<Pb1, Pb0>();
#else
    Uart::SelectTxRxPins<Pa1, Pa0>();
#endif

    const char hello[] = "NIIET UART0 reverser (DMA, 4-char blocks)\r\n";
    Uart::Write(hello, sizeof(hello) - 1);

    PLIC_SetIrqHandler(Plic_Mach_Target, DmaTx::IRQNumber, DmaTxIsr);
    PLIC_SetIrqHandler(Plic_Mach_Target, DmaRx::IRQNumber, DmaRxIsr);
    InterruptEnable();

    Uart::EnableAsyncRead(rxBlock, sizeof(rxBlock), OnBlockReceived);

    for (;;)
    {
    }
}

// Fires once the 4-char block has landed (RX DMA); reverse it and DMA it back.
static void OnBlockReceived(void* data, unsigned size, bool success)
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
