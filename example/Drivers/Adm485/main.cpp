// HSI_VALUE for my stm32f401cc is equal to 16Mhz.
// I need to define this macro globally (as compiler option) for correct clock settings.

#include <zhele/drivers/adm485.h>

using namespace Zhele;
using namespace Zhele::IO;

using Connection = Zhele::Drivers::Adm485<Zhele::Usart1, Pb5>;
using Led = Pc13Inv;

char TxBuffer[] = "SomeData";
char RxBuffer[9];

void TransferCompleteHandler(void* data, unsigned size, bool success);

int main()
{	
    Led::Port::Enable();
    Led::SetConfiguration(Led::Configuration::Out);
    Led::SetDriverType(Led::DriverType::PushPull);
    Led::Clear();

    // Init
    Connection::Init(9600);
    Connection::SelectTxRxPins<Pb6, Pb7>();

    // Init async read (via DMA)
    Connection::EnableAsyncRead(RxBuffer, 9, TransferCompleteHandler);

    // Async write
    // Connection::Write(TxBuffer, 8);
    // Async write
    Connection::WriteAsync(TxBuffer, 8);

    for (;;)
    {
    }
}

void TransferCompleteHandler(void* data, unsigned size, bool success)
{
    if(success) {
        Led::Set();
    }
}
