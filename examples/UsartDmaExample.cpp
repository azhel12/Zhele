#define F_CPU 8000000

#include <usart.h>

#include <cstring>

using namespace Zhele;
using namespace Zhele::IO;

using UsartConnection = Usart1;

char TxBuffer[] = "SomeData";
char RxBuffer[9];

void TransferCompleteHandler(void* data, unsigned size, bool success);

// I have made connect USART1_Tx pin (PB6) with USART1_Rx pin (PB7).
// So, controller will send data to itself.
int main()
{	
    // Configure led (I have stm32f103c8t6-based board. It has led on C13)
    Pc13::Port::Enable();
    Pc13::SetConfiguration(Pc13::Configuration::Out);
    Pc13::SetDriverType(Pc13::DriverType::PushPull);
    Pc13::Set();

    // Init usart
    UsartConnection::Init(9600);
    UsartConnection::SelectTxRxPins<Pb6, Pb7>();
    // Init async read (via DMA)
    UsartConnection::EnableAsyncRead(RxBuffer, 9, TransferCompleteHandler);
    // Async write to USART
    UsartConnection::Write(TxBuffer, 9, true);
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
        // Check it via breakpoint on "Pc13::Clear();"
        Pc13::Clear();
    }
}
