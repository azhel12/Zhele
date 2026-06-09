#include <zhele/usart.h>

#include <system_k1921vg015.h> // SystemInit()

using Uart = Zhele::Usart0<>;

static void writeString(const char* s)
{
    while (*s)
        Uart::Write(static_cast<uint8_t>(*s++));
}

int main()
{
    SystemInit();

    Uart::Init<115200>();
    Uart::SelectTxRxPins<Zhele::IO::Pa1, Zhele::IO::Pa0>();

    writeString("NIIET UART0 echo ready\r\n");

    for (;;)
    {
        if (Uart::ReadReady())
            Uart::Write(Uart::Read());
    }
}
