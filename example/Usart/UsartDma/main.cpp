#include <zhele/usart.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined (STM32G0)
    #include <zhele/dma.h>
    #include <zhele/dmamux.h>
    using UsartConnection = Usart1<Dma1Channel1, Dma1Channel2>;
#else
    using UsartConnection = Usart1;
#endif
using Led = Pa7;

char TxBuffer[] = "SomeData";
char RxBuffer[9];

void TransferCompleteHandler(void* data, unsigned size, bool success);

// I have made connect USART1_Tx pin (PB6) with USART1_Rx pin (PB7).
// So, controller will send data to itself.
int main()
{	
    Led::Port::Enable();
    Led::SetConfiguration(Led::Configuration::Out);
    Led::SetDriverType(Led::DriverType::PushPull);
    Led::Set();

    // Init usart
    UsartConnection::Init(9600);
    UsartConnection::SelectTxRxPins<Pb6, Pb7>();

// Configure DMA multiplexer
#if defined (STM32G0)
    Dma1::Enable();
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::Usart1Tx);
    DmaMux1Channel2::SelectRequestInput(DmaMux1::RequestInput::Usart1Rx);
#endif

    // Init async read (via DMA)
    UsartConnection::EnableAsyncRead(RxBuffer, 9, TransferCompleteHandler);
    // Async write to USART
    UsartConnection::WriteAsync(TxBuffer, 9);
    // Also we can combine Sync/Async on transmit/receive
    // For example, next code works fine

    //UsartConnection::Write(TxBuffer, 9, false);

    for (;;)
    {
    }
}

void TransferCompleteHandler(void* data, unsigned size, bool success)
{
    if(success)
    {
        // Expected "SomeData" in RxBuffer.
        // Check it via breakpoint on "Led::Clear();"
        Led::Clear();
    }
}

extern "C" {

// F030: Dma1Channel2, Dma1Channel3
// F103: Dma1Channel4, Dma1Channel5
// F401: Dma2Stream7Channel4, Dma2Stream2Channel4
// G030 (configurable by DMAMUX): Dma1Channel1, Dma1Channel2

#if defined (STM32F0)
    void DMA1_Channel2_3_IRQHandler()
    {
        UsartConnection::DmaTx::IrqHandler();
        UsartConnection::DmaRx::IrqHandler();
    }
#elif defined (STM32F1)
    void DMA1_Channel4_IRQHandler()
    {
        UsartConnection::DmaTx::IrqHandler();
    }
    void DMA1_Channel5_IRQHandler()
    {
        UsartConnection::DmaRx::IrqHandler();
    }
#elif defined (STM32F4)
    void DMA2_Stream7_IRQHandler()
    {
        UsartConnection::DmaTx::IrqHandler();
    }
    void DMA2_Stream2_IRQHandler()
    {
        UsartConnection::DmaRx::IrqHandler();
    }
#elif defined (STM32G0)
    void DMA1_Channel1_IRQHandler()
    {
        UsartConnection::DmaTx::IrqHandler();
    }
    void DMA1_Channel2_3_IRQHandler()
    {
        UsartConnection::DmaRx::IrqHandler();
    }
#else
    #error "No example"
#endif
}