/**
 * @file
 * UART0 echo over DMA, interrupt-driven, for NIIET K1921VG015 (IRIS/UNO-VG015).
 */
#include <zhele/dma.h>
#include <zhele/usart.h>

#include <system_k1921vg015.h>
extern "C" {
    #include <plic.h> // SetIrqHandler()
}

using namespace Zhele;

using DmaTx = DmaUart0Tx;
using DmaRx = DmaUart0Rx;
using Uart  = Usart0<DmaTx, DmaRx>;

static constexpr size_t ChunkSize = 8;
static uint8_t buffer[ChunkSize];

static void onReceived(void* data, unsigned size, bool success);
static void onTransmitted(void* data, unsigned size, bool success);

static void onReceived(void* data, unsigned size, bool) {
    Uart::WriteAsync(data, size, onTransmitted);
}

static void onTransmitted(void*, unsigned, bool) {
    Uart::EnableAsyncRead(buffer, ChunkSize, onReceived); // re-arm the next read
}

// Shared DMA-vector ISRs (defined below); each services its own channel.
extern "C" {
    void DmaTxIsr();
    void DmaRxIsr();
}

int main()
{
    SystemInit();

    Uart::Init<115200>();
    Uart::SelectTxRxPins<IO::Pa1, IO::Pa0>();

    SetIrqHandler(DmaTx::IRQNumber, DmaTxIsr, 1);
    SetIrqHandler(DmaRx::IRQNumber, DmaRxIsr, 1);
    InterruptEnable();

    Uart::EnableAsyncRead(buffer, ChunkSize, onReceived);

    for (;;) {

    }
}

extern "C" {
    void DmaTxIsr() {
        DmaTx::IrqHandler();
    }
    void DmaRxIsr() {
        DmaRx::IrqHandler();
    }
}
