#include <zhele/usart.h>

using namespace Zhele;
using namespace Zhele::IO;

#if defined (STM32C0)
    #include <zhele/clock.h>
    #include <zhele/dma.h>
    #include <zhele/dmamux.h>
    using UsartConnection = Usart2<Dma1Channel1, Dma1Channel2>;
    using Led = Pa4Inv;
#elif defined (STM32G0)
    #include <zhele/dma.h>
    #include <zhele/dmamux.h>
    using UsartConnection = Usart1<Dma1Channel1, Dma1Channel2>;
    using Led = Pa7;
#else
    using UsartConnection = Usart1;
    using Led = Pa7;
#endif

char TxBuffer[] = "SomeData";
char RxBuffer[9];

void TransferCompleteHandler(void* data, unsigned size, bool success);

// I have made connect USART1_Tx pin (PB6) with USART1_Rx pin (PB7).
// So, controller will send data to itself.
int main()
{
#if defined (STM32C0)
    // STM32C0 has no PLL: run SYSCLK straight off HSI48 (HSISYS = 48 MHz).
    Clock::SetHsiSysDivider<1>();
    Clock::SysClock::SelectClockSource<Clock::SysClock::Internal>();
#endif

    Led::Port::Enable();
    Led::SetConfiguration(Led::Configuration::Out);
    Led::SetDriverType(Led::DriverType::PushPull);
    Led::Set();

    // Init usart
    UsartConnection::Init(9600);
#if defined (STM32C0)
    UsartConnection::SelectTxRxPins<Pa2, Pa3>();
#else
    UsartConnection::SelectTxRxPins<Pb6, Pb7>();
#endif

// Configure DMA multiplexer
#if defined (STM32C0)
    Dma1::Enable();
    DmaMux1Channel1::SelectRequestInput(DmaMux1::RequestInput::Usart2Tx);
    DmaMux1Channel2::SelectRequestInput(DmaMux1::RequestInput::Usart2Rx);
#elif defined (STM32G0)
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
// C011 (configurable by DMAMUX): Dma1Channel1, Dma1Channel2 (ch1 own line, ch2/3 shared)

#if defined (STM32C0)
    void DMA1_Channel1_IRQHandler()
    {
        UsartConnection::DmaTx::IrqHandler();
    }
    void DMA1_Channel2_3_IRQHandler()
    {
        UsartConnection::DmaRx::IrqHandler();
    }
#elif defined (STM32F0)
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